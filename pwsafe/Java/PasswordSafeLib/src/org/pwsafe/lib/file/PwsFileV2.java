/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import java.io.IOException;

import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;

/**
 * Encapsulates version 2 PasswordSafe files.
 * 
 * @author Kevin Preece
 */
public class PwsFileV2 extends PwsFile
{
	public static final int		VERSION		= 2;
	public static final String	ID_STRING	= " !!!Version 2 File Format!!! Please upgrade to PasswordSafe 2.0 or later";

	/**
	 * Use of this constructor to load a PasswordSafe database is STRONGLY discouraged
	 * since it's use ties the caller to a particular file version.  Use {@link
	 * PwsFileFactory#loadFile} instead.
	 * </p><p>
	 * <b>N.B. </b>this constructor's visibility may be reduced in future releases.
	 * </p>
	 */
	public PwsFileV2( String filename, String password ) 
	throws EndOfFileException, IOException, UnsupportedFileVersionException
	{
		super(filename, password);
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
		// TODO Create dummy V1 record with title set to ID_STRING
		PwsRecordV1	hdr;

		hdr = new PwsRecordV1();

		hdr.setField( new PwsStringField( PwsRecordV1.TITLE, PwsFileV2.ID_STRING ) );
		hdr.setField( new PwsStringField( PwsRecordV1.PASSWORD, "pre-2.0" ) );

		hdr.saveRecord( file );
	}
}
