/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.util;

import java.io.File;
import java.io.IOException;
import java.util.Iterator;

import org.pwsafe.lib.file.PwsFile;
import org.pwsafe.lib.file.PwsFileV1;
import org.pwsafe.lib.file.PwsFileV2;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;

/**
 * This singleton class contains methods for converting PasswordSafe databases
 * between formats.
 * 
 * @author Kevin Preece
 */
public class FileConverter
{
	/**
	 * Private for the singleton pattern.
	 */
	private FileConverter()
	{
		super();
	}

	/**
	 * Converts <code>oldFile</code> to the latest supported version, currently version 2.  If
	 * the file is already the latest format no new file is created and the reference is simply
	 * returned as-is.
	 * 
	 * @param oldFile the file to be converted.
	 * 
	 * @return A file in the latest format containing the data from <code>oldFile</code>.
	 * 
	 * @throws IOException
	 */
	public static PwsFile convertToLatest( PwsFile oldFile )
	throws IOException
	{
		if ( oldFile instanceof PwsFileV2 )
		{
			return oldFile;
		}
		return convertV1ToV2( (PwsFileV1) oldFile );
	}

	/**
	 * Converts a version 1 PasswordSafe database to version 2.
	 * 
	 * @param oldFile the database to convert.
	 * 
	 * @return The version 2 database.
	 * 
	 * @throws IOException
	 */ 
	public static PwsFile convertV1ToV2( PwsFileV1 oldFile )
	throws IOException
	{
		PwsFileV2	newFile;
		PwsRecordV1	oldRec;
		PwsRecord	newRec;

		newFile = new PwsFileV2();

		newFile.setPassphrase( oldFile.getPassphrase() );
		newFile.setFilename( makeNewFilename(oldFile.getFilename(), "v2-") );

		for ( Iterator iter = oldFile.getRecords(); iter.hasNext(); )
		{
			oldRec	= (PwsRecordV1) iter.next();
			newRec	= newFile.newRecord();

			newRec.setField( oldRec.getField(PwsRecordV1.TITLE) );
			newRec.setField( oldRec.getField(PwsRecordV1.USERNAME) );
			newRec.setField( oldRec.getField(PwsRecordV1.PASSWORD) );
			newRec.setField( oldRec.getField(PwsRecordV1.NOTES) );
		}

		return newFile;
	}

	/**
	 * Converts a version 2 PasswordSafe database to version 1.  Note this will
	 * result in data loss since fields not supported by version 1 will be
	 * silently dropped.
	 * 
	 * @param oldFile the file to be converted.
	 * 
	 * @return The version 1 database.
	 * 
	 * @throws IOException
	 */
	public static PwsFile convertV2ToV1( PwsFileV2 oldFile )
	throws IOException
	{
		PwsFileV1	newFile;
		PwsRecordV2	oldRec;
		PwsRecord	newRec;

		newFile = new PwsFileV1();

		newFile.setPassphrase( oldFile.getPassphrase() );
		newFile.setFilename( makeNewFilename(oldFile.getFilename(), "v1-") );

		for ( Iterator iter = oldFile.getRecords(); iter.hasNext(); )
		{
			oldRec	= (PwsRecordV2) iter.next();
			newRec	= newFile.newRecord();

			newRec.setField( oldRec.getField(PwsRecordV2.TITLE) );
			newRec.setField( oldRec.getField(PwsRecordV2.USERNAME) );
			newRec.setField( oldRec.getField(PwsRecordV2.PASSWORD) );
			newRec.setField( oldRec.getField(PwsRecordV2.NOTES) );
		}

		return newFile;
	}

	/**
	 * Makes a new filename from the given filename and prefix.  The prefix
	 * is prepended to the name.  For example if <code>filename</code> is
	 * "C:\Program Files\Java\PasswordSafe\MyPasswords.dat" and
	 * <code>prefix</code> is "V2-", the new filename would be
	 * "C:\Program Files\Java\PasswordSafe\V2-MyPasswords.dat"
	 *  
	 * @param filename
	 * @param prefix
	 * @return
	 */
	private static String makeNewFilename( String filename, String prefix )
	{
		File			file;
		String			path;
		String			name;
		StringBuffer	sb;
		String			newName;

		file	= new File( filename );
		path	= file.getParent();
		name	= file.getName();
		sb		= new StringBuffer(path.length() + prefix.length() + name.length());
		newName	= sb.append(path).append(prefix).append(name).toString();
		file	= new File( newName );

		if ( file.exists() )
		{
			// TODO generate a temporary filename
		}

		return newName;
	}
}
