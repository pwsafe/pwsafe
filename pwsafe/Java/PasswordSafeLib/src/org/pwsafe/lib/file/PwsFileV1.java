package org.pwsafe.lib.file;

import java.io.IOException;

import org.pwsafe.lib.exception.EndOfFileException;

/**
 * 
 * @author Kevin Preece
 */
public class PwsFileV1 extends PwsFile
{
	public static final int	VERSION		= 1;

	/**
	 * 
	 */
	public PwsFileV1( String filename, String password )
	throws EndOfFileException, IOException
	{
		super( filename, password );
	}

	public int getFileVersionMajor()
	{
		return VERSION;
	}
}
