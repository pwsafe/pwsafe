/*
 * $Id: PwsFileV2.java 944 2006-09-08 03:25:19 +0000 (Fri, 08 Sep 2006) glen_a_smith $
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import javax.crypto.Cipher;

import org.pwsafe.lib.I18nHelper;
import org.pwsafe.lib.Log;
import org.pwsafe.lib.Util;
import org.pwsafe.lib.crypto.SHA256Pws;
import org.pwsafe.lib.crypto.TwofishPws;
import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;

/**
 * Encapsulates version 3 PasswordSafe files.
 * 
 * @author Glen Smith (based on Kevin Preece's v2 implementation).
 */
public class PwsFileV3 extends PwsFile
{
	private static final Log LOG = Log.getInstance(PwsFileV3.class.getPackage().getName());

	/**
	 * The PasswordSafe database version number that this class supports.
	 */
	public static final int		VERSION		= 3;

	/**
	 * The string that identifies a database as V2 rather than V1
	 */
	public static final String	ID_STRING	= " !!!Version 2 File Format!!! Please upgrade to PasswordSafe 2.0 or later";

	/**
	 * The file's standard header.
	 */
	protected PwsFileHeaderV3	Header			= null;
	
	protected byte[] stretchedPassword;
	protected byte[] decryptedRecordKey;
	protected byte[] decryptedHmacKey;
	
	protected Cipher fieldCrypto;
	protected Cipher fieldDecrypto;
	
	/**
	 * Constructs and initialises a new, empty version 3 PasswordSafe database in memory.
	 */
	public PwsFileV3()
	{
		super();
	}

	/**
	 * Use of this constructor to load a PasswordSafe database is STRONGLY discouraged
	 * since it's use ties the caller to a particular file version.  Use {@link
	 * PwsFileFactory#loadFile(String, String)} instead.
	 * </p><p>
	 * <b>N.B. </b>this constructor's visibility may be reduced in future releases.
	 * </p>
	 * @param filename   the name of the database to open.
	 * @param passphrase the passphrase for the database.
	 * 
	 * @throws EndOfFileException
	 * @throws IOException
	 * @throws UnsupportedFileVersionException
	 */
	public PwsFileV3( String filename, String passphrase ) 
	throws EndOfFileException, IOException, UnsupportedFileVersionException
	{
		super( filename, passphrase );
	}
	
	public void dumpBytes(String title, byte[] bytes) {
		System.out.print(title + " [");
		for (int i = 0; i < bytes.length; i++) {
			System.out.print(bytes[i] + " ");
		}
		System.out.println("]");
	}

	protected void open( File file, String passphrase )
	throws EndOfFileException, IOException, UnsupportedFileVersionException
	{
		LOG.enterMethod( "PwsFileV3.init" );

		setFilename( file );

		Passphrase		= passphrase;
		

		InStream		= new FileInputStream( file );
		Header			= new PwsFileHeaderV3( this );
		
		int iter = Util.getIntFromByteArray(Header.getIter(), 0);
		LOG.debug1("Using iterations: [" + iter + "]");
		stretchedPassword = stretchPassphrase(passphrase.getBytes(), Header.getSalt(), iter);
		//Algorithm		= makeBlowfish( passphrase.getBytes() );
		dumpBytes("From file", Header.getPassword());
		dumpBytes("Calc", SHA256Pws.digest(stretchedPassword));
		
		if (!Util.bytesAreEqual(Header.getPassword(), SHA256Pws.digest(stretchedPassword))) {
			throw new IOException("Invalid password");
		}
		
		try {
			
			dumpBytes("stretchedPassword", stretchedPassword);
			
			Cipher cipher = TwofishPws.getCipher(stretchedPassword, null, false, true);
			byte[] rka = cipher.doFinal(Header.getB1());
			byte[] rkb = cipher.doFinal(Header.getB2());
			decryptedRecordKey = Util.mergeBytes(rka, rkb);
			
			byte[] hka = cipher.doFinal(Header.getB3());
			byte[] hkb = cipher.doFinal(Header.getB4());
			decryptedHmacKey = Util.mergeBytes(hka, hkb);
			
		} catch (Exception e) {
			e.printStackTrace();
			throw new IOException("Error reading encrypted fields");
		}
		
		fieldCrypto = TwofishPws.getCipher(decryptedRecordKey, Header.getIV(), true, false);
		fieldDecrypto = TwofishPws.getCipher(decryptedRecordKey, Header.getIV(), false, false);

		readExtraHeader( this );

		LOG.leaveMethod( "PwsFileV3.init" );
	}
	
	

	/**
	 * Calculate stretched key.
	 * 
	 * http://www.schneier.com/paper-low-entropy.pdf (Section 4.1), 
	 * with SHA-256 as the hash function, and ITER iterations 
	 * (at least 2048, i.e., t = 11).
	 * @param passphrase the user entered passphrase
	 * @param salt the salt from the file
	 * @param iter the number of iters from the file
	 * @return the stretched user key for comparison
	 */
	private byte[] stretchPassphrase(byte[] passphrase, byte[] salt, int iter) {
		byte[] p = Util.mergeBytes(passphrase, salt);
		byte[] hash = SHA256Pws.digest(p);
		for (int i = 0; i < iter; i++) {
			hash = SHA256Pws.digest(hash);
		}
		return hash;
		
	}

	/**
	 * Returns the major version number for the file.
	 * 
	 * @return The major version number for the file.
	 */
	public int getFileVersionMajor()
	{
		return VERSION;
	}

	/**
	 * Allocates a new, empty record unowned by any file.  The record type is
	 * {@link PwsRecordV2}.
	 * 
	 * @return A new empty record
	 * 
	 * @see org.pwsafe.lib.file.PwsFile#newRecord()
	 */
	public PwsRecord newRecord()
	{
		return new PwsRecordV3();
	}
	
	/**
	 * Reads the extra header present in version 2 files.
	 * 
	 * @param file the file to read the header from.
	 * 
	 * @throws EndOfFileException If end of file is reached.
	 * @throws IOException If an error occurs whilst reading. 
	 * @throws UnsupportedFileVersionException If the header is not a valid V2 header.
	 */
	protected void readExtraHeader( PwsFile file )
	throws EndOfFileException, IOException, UnsupportedFileVersionException
	{
		
	}

	/**
	 * Writes the extra version 2 header.
	 * 
	 * @param file the file to write the header to.
	 * 
	 * @throws IOException if an error occurs whilst writing the header. 
	 */
	protected void writeExtraHeader( PwsFile file )
	throws IOException
	{
		
	}
	
	/**
	 * Reads bytes from the file and decryps them.  <code>buff</code> may be any length provided
	 * that is a multiple of <code>BLOCK_LENGTH</code> bytes in length.
	 * 
	 * @param buff the buffer to read the bytes into.
	 * 
	 * @throws EndOfFileException If end of file has been reached.
	 * @throws IOException If a read error occurs.
	 * @throws IllegalArgumentException If <code>buff.length</code> is not an integral multiple of <code>BLOCK_LENGTH</code>.
	 */
	protected void readDecryptedBytes( byte [] buff )
	throws EndOfFileException, IOException
	{
		if ( (buff.length == 0) || ((buff.length % BLOCK_LENGTH) != 0) )
		{
			throw new IllegalArgumentException( I18nHelper.getInstance().formatMessage("E00001") );
		}
		readBytes( buff );
		byte[] decrypted;
		try {
			decrypted = fieldDecrypto.doFinal(buff);
		} catch (Exception e) {
			throw new IOException("Error decrypting field");
		}
		Util.copyBytes(decrypted, buff);
		//Algorithm.decrypt( buff );
	}
	
	/**
	 * Encrypts then writes the contents of <code>buff</code> to the file.
	 * 
	 * @param buff the data to be written.
	 * 
	 * @throws IOException
	 */
	protected void writeEncryptedBytes( byte [] buff )
	throws IOException
	{
		if ( (buff.length == 0) || ((buff.length % BLOCK_LENGTH) != 0) )
		{
			throw new IllegalArgumentException( I18nHelper.getInstance().formatMessage("E00001") );
		}
		
		byte [] temp = Util.cloneByteArray( buff );
		try {
			temp = fieldCrypto.doFinal(temp);
		} catch(Exception e) {
			throw new IOException("Error writing encrypted field");
		}
		writeBytes( temp );
	}
	
	
}
