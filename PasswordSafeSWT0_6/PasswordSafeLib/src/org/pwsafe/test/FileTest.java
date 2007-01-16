/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.test;

import org.pwsafe.lib.file.PwsFile;
import org.pwsafe.lib.file.PwsFileV1;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsStringField;

import junit.framework.TestCase;

/**
 *
 */
public class FileTest extends TestCase
{
	/**
	 * Tests that the library can create a version 1 database.
	 */
	public void testCreateV1File()
	{
		try
		{
			PwsFile		v1File;
			PwsRecord	rec;
	
			v1File	= new PwsFileV1();
	
			v1File.setFilename( "V1 New File.dat" );
			v1File.setPassphrase( "passphrase" );
	
			rec = v1File.newRecord();
			
			rec.setField( new PwsStringField(PwsRecordV1.TITLE, "Entry number 1") );
			rec.setField( new PwsStringField(PwsRecordV1.PASSWORD, "Password 1") );

			v1File.add( rec );
			
			rec = v1File.newRecord();
			
			rec.setField( new PwsStringField(PwsRecordV1.TITLE, "Entry number 2") );
			rec.setField( new PwsStringField(PwsRecordV1.PASSWORD, "Password 2") );
			rec.setField( new PwsStringField(PwsRecordV1.USERNAME, "Username 2") );
			rec.setField( new PwsStringField(PwsRecordV1.NOTES, "Notes line 1\r\nNotes line 2") );
			
			v1File.add( rec );

			assertTrue( "Modified flag is not TRUE", v1File.isModified() );
			assertEquals( "Record count is not = 2", 2, v1File.getRecordCount() );

			v1File.save();

			assertTrue( "Modified flag is not FALSE", !v1File.isModified() );
		}
		catch ( Exception e )
		{
			fail( "Unexpected exception caught - " + e.getMessage() );
		}
	}
}
