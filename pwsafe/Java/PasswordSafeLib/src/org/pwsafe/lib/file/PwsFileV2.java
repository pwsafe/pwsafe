package org.pwsafe.lib.file;

import java.io.IOException;

import org.pwsafe.lib.exception.EndOfFileException;

/**
 * @author Kevin Preece
 */
public class PwsFileV2 extends PwsFile
{
	public static final int		VERSION		= 2;
	public static final String	ID_STRING	= " !!!Version 2 File Format!!! Please upgrade to PasswordSafe 2.0 or later";

	/**
	 * 
	 */
	public PwsFileV2( String filename, String password ) 
	throws EndOfFileException, IOException
	{
		super(filename, password);
	}

	public int getFileVersionMajor()
	{
		return VERSION;
	}

	protected void readExtraHeader( PwsFile file )
	throws EndOfFileException, IOException
	{
		PwsRecordV1	hdr;

		hdr = new PwsRecordV1();
		hdr.loadRecord( file );
	}

	/* (non-Javadoc)
	 * @see org.pwsafe.lib.file.PwsFile#writeExtraHeader(org.pwsafe.lib.file.PwsFile)
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
