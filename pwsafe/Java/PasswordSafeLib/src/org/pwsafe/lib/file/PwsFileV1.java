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
 * Encapsulates version 1 PasswordSafe files.
 * 
 * @author Kevin Preece
 */
public class PwsFileV1 extends PwsFile
{
//	private static final Log LOG = Log.getInstance(PwsFileV1.class.getPackage().getName());

	/**
	 * The PasswordSafe database version number that this class supports.
	 */
	public static final int	VERSION		= 1;

	/**
	 * Use of this constructor to load a PasswordSafe database is STRONGLY discouraged
	 * since it's use ties the caller to a particular file version.  Use {@link
	 * PwsFileFactory#loadFile} instead.
	 * </p><p>
	 * <b>N.B. </b>this constructor's visibility may be reduced in future releases.
	 * </p>
	 * @param filename the name of the database to read.
	 * @param password the password needed to open the database.
	 * 
	 * @throws EndOfFileException
	 * @throws IOException
	 * @throws UnsupportedFileVersionException
	 */
	public PwsFileV1( String filename, String password )
	throws EndOfFileException, IOException, UnsupportedFileVersionException
	{
		super( filename, password );
	}

	/**
	 * Returns the major version number for the file.
	 * 
	 * @return The file's major version number.
	 */
	public int getFileVersionMajor()
	{
		return VERSION;
	}
}
