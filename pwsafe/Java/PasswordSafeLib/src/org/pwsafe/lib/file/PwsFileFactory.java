/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import org.pwsafe.lib.I18nHelper;
import org.pwsafe.lib.Log;
import org.pwsafe.lib.Util;
import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.InvalidPasswordException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;

import BlowfishJ.BlowfishECB;
import BlowfishJ.SHA1;

/**
 * This is a singleton factory class used to load a PasswordSafe file.  It is able to
 * determine which version of the file format the file has and returns the correct
 * subclass of {@link PwsFile}
 * 
 * @author Kevin Preece
 */
public class PwsFileFactory
{
	private static final Log LOG = Log.getInstance(PwsFileFactory.class.getPackage().getName());

	/**
	 * Private for the singleton pattern.
	 */
	private PwsFileFactory()
	{
	}

	/**
	 * Verifies that <code>password</code> is actually the password for the file.  It returns
	 * normally if everything is OK or {@link InvalidPasswordException} if the password is
	 * incorrect.
	 * 
	 * @param filename the name of the file to be opened.
	 * @param password the file's password.
	 * 
	 * @throws InvalidPasswordException If the password is not the correct one for the file.
	 * @throws FileNotFoundException    If the given file does not exist.
	 * @throws IOException              If an error occurs whilst reading from the file.
	 */
	private static final void checkPassword( String filename, String password )
	throws InvalidPasswordException, FileNotFoundException, IOException 
	{
		LOG.enterMethod( "PwsFileFactory.checkPassword" );

		FileInputStream	fis				= null;
		byte []			stuff;
		byte []			fudged;
		byte []			fhash;
		byte []			phash;
		boolean			handlingError	= false;

		try
		{
			fis		= new FileInputStream( filename );
			stuff	= new byte[ PwsFile.STUFF_LENGTH ];
			fhash	= new byte[ PwsFile.HASH_LENGTH ];
			
			fis.read( stuff );
			fis.read( fhash );

			fudged	= new byte[ PwsFile.STUFF_LENGTH + 2 ];
			
			for ( int ii = 0; ii < PwsFile.STUFF_LENGTH; ++ii )
			{
				fudged[ii] = stuff[ii];
			}
			stuff	= null;
			phash	= genRandHash( password, fudged );
			
			if ( !Util.bytesAreEqual( fhash, phash ) )
			{
				LOG.debug1( "Password is incorrect - throwing InvalidPasswordException" );
				LOG.leaveMethod( "PwsFileFactory.checkPassword" );
				throw new InvalidPasswordException();
			}
		}
		catch ( IOException e )
		{
			handlingError = true;
			LOG.error( I18nHelper.formatMessage("E00007", new Object [] { e.getClass().getName() } ), e );
			LOG.info( "I00001" );
			LOG.leaveMethod( "PwsFileFactory.checkPassword" );
			throw e;
		}
		finally
		{
			if ( fis != null )
			{	
				try
				{
					LOG.debug1( "Attempting to close the file" );
					fis.close();
	
					fis = null;
				}
				catch ( IOException e )
				{
					// log the exception then decide what we're going to do with it
					LOG.error( I18nHelper.formatMessage("E00007", new Object [] { e.getClass().getName() } ), e );
					if ( handlingError )
					{
						// ignore the error
						LOG.info( I18nHelper.formatMessage( "I00002" ) );
					}
					else
					{	
						LOG.info( I18nHelper.formatMessage( "I00001" ) );
						throw e;
					}
				}
			}
		}

		LOG.debug1( "Password is OK" );
		LOG.leaveMethod( "PwsFileFactory.checkPassword" );
	}

	/**
	 * Generates a checksum from the password and some random bytes.
	 * 
	 * @param  password  the password.
	 * @param  stuff     the random bytes.
	 * 
	 * @return the generated checksum.
	 */
	static final byte [] genRandHash( String password, byte [] stuff )
	{
		LOG.enterMethod( "PwsFileFactory.genRandHash" );

		SHA1			md;
		BlowfishECB		bf;
		byte []			pw;
		byte []			digest;
		byte []			tmp;
		
		pw	= password.getBytes();
		md	= new SHA1();

		md.update( stuff, 0, stuff.length );
		md.update( pw, 0, pw.length );
		md.finalize();
		digest = md.getDigest();
		
		bf	= new BlowfishECB( digest, 0, digest.length );
		tmp	= Util.cloneByteArray( stuff, 8 );

		Util.bytesToLittleEndian( tmp );

		for ( int ii = 0; ii < 1000; ++ii )
		{
			bf.encrypt( tmp, 0, tmp, 0, tmp.length );
		}

		Util.bytesToLittleEndian( tmp );
		tmp = Util.cloneByteArray( tmp, 10 );

		md.clearContext();
		md.update( tmp, 0, tmp.length );
		md.finalize();
		
		LOG.leaveMethod( "PwsFileFactory.genRandHash" );
		return md.getDigest();
	}

	/**
	 * Loads a Password Safe file.  It returns the appropriate subclass of {@link PwsFile}.
	 * 
	 * @param filename the name of the file to open
	 * @param password the password for the file
	 * 
	 * @return The correct subclass of {@link PwsFile} for the file.
	 * 
	 * @throws EndOfFileException
	 * @throws FileNotFoundException
	 * @throws InvalidPasswordException
	 * @throws IOException
	 * @throws UnsupportedFileVersionException
	 */
	public static final PwsFile loadFile( String filename, String password )
	throws EndOfFileException, FileNotFoundException, InvalidPasswordException, IOException, UnsupportedFileVersionException
	{
		LOG.enterMethod( "PwsFileFactory.loadFile" );
		
		PwsFile		file;
		PwsRecordV1	rec;

		checkPassword( filename, password );

		file	= new PwsFileV1( filename, password );
		rec		= (PwsRecordV1) file.readRecord();

		file.close();

		// TODO what can we do about this?
		// it will probably be fooled if someone is daft enough to create a V1 file with the
		// title of the first record set to the value of PwsFileV2.ID_STRING!

		if ( rec.getField(PwsRecordV1.TITLE).equals(PwsFileV2.ID_STRING) )
		{
			LOG.debug1( "This is a V2 format file." );
			file = new PwsFileV2( filename, password );
		}
		else
		{
			LOG.debug1( "This is a V1 format file." );
			file = new PwsFileV1( filename, password );
		}
		file.readAll();
		file.close();

		LOG.debug1( "File contains " + file.getRecordCount() + " records." );
		LOG.leaveMethod( "PwsFileFactory.loadFile" );
		return file;
	}
}
