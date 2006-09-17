/*
 * $Id: PwsFileV2.java 944 2006-09-08 03:25:19 +0000 (Fri, 08 Sep 2006) glen_a_smith $
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import java.io.IOException;

import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;

/**
 * Encapsulates version 3 PasswordSafe files.
 * 
 * @author Glen Smith (based on Kevin Preece's v2 implementation).
 */
public class PwsFileV3 extends PwsFile
{
	/**
	 * The PasswordSafe database version number that this class supports.
	 */
	public static final int		VERSION		= 3;

	/**
	 * The string that identifies a database as V2 rather than V1
	 */
	public static final String	ID_STRING	= " !!!Version 2 File Format!!! Please upgrade to PasswordSafe 2.0 or later";

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
		PwsRecordV1	hdr;

		hdr = new PwsRecordV1();
		hdr.loadRecord( file );

		if ( !hdr.getField(PwsRecordV1.TITLE).equals(ID_STRING) )
		{
			throw new UnsupportedFileVersionException();
		}
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
		PwsRecordV1	hdr;

		hdr = new PwsRecordV1();

		hdr.setField( new PwsStringField( PwsRecordV1.TITLE, PwsFileV3.ID_STRING ) );
		hdr.setField( new PwsStringField( PwsRecordV1.PASSWORD, "3.0" ) );

		hdr.saveRecord( file );
	}
}
