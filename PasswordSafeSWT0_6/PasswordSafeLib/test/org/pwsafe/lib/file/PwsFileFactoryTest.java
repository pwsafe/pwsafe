package org.pwsafe.lib.file;

import org.pwsafe.lib.exception.PasswordSafeException;

import junit.framework.TestCase;

public class PwsFileFactoryTest extends TestCase {

	public void testFile()
	throws PasswordSafeException
	{
		PwsFileV2	file;
		PwsRecordV2	rec;

		file	= new PwsFileV2();
		rec		= (PwsRecordV2) file.newRecord();

		rec.setField( new PwsStringField( PwsRecordV2.USERNAME, "User 1") );
		rec.setField( new PwsStringField( PwsRecordV2.PASSWORD, "Pass 1") );
		rec.setField( new PwsStringField( PwsRecordV2.TITLE, "Online Bank 1") );
		rec.setField( new PwsStringField( PwsRecordV2.GROUP, "bank.online") );
		
		file.add( rec );

		rec		= (PwsRecordV2) file.newRecord();

		rec.setField( new PwsStringField( PwsRecordV2.USERNAME, "User 2") );
		rec.setField( new PwsStringField( PwsRecordV2.PASSWORD, "Pass 2") );
		rec.setField( new PwsStringField( PwsRecordV2.TITLE, "Online Bank 2") );
		rec.setField( new PwsStringField( PwsRecordV2.GROUP, "bank.online") );
		
		file.add( rec );

		rec		= (PwsRecordV2) file.newRecord();

		rec.setField( new PwsStringField( PwsRecordV2.USERNAME, "User 3") );
		rec.setField( new PwsStringField( PwsRecordV2.PASSWORD, "Pass 3") );
		rec.setField( new PwsStringField( PwsRecordV2.TITLE, "Lone entry") );
		rec.setField( new PwsStringField( PwsRecordV2.GROUP, "bank") );
		
		file.add( rec );

		rec		= (PwsRecordV2) file.newRecord();

		rec.setField( new PwsStringField( PwsRecordV2.USERNAME, "User 4") );
		rec.setField( new PwsStringField( PwsRecordV2.PASSWORD, "Pass 4") );
		rec.setField( new PwsStringField( PwsRecordV2.TITLE, "Telephone Bank 1") );
		rec.setField( new PwsStringField( PwsRecordV2.GROUP, "bank.telephone") );
		
		file.add( rec );

		rec		= (PwsRecordV2) file.newRecord();

		rec.setField( new PwsStringField( PwsRecordV2.USERNAME, "User 5") );
		rec.setField( new PwsStringField( PwsRecordV2.PASSWORD, "Pass 5") );
		rec.setField( new PwsStringField( PwsRecordV2.TITLE, "Some Online Store") );
		rec.setField( new PwsStringField( PwsRecordV2.GROUP, "websites") );
		
		file.add( rec );

		assertEquals(5, file.getRecordCount());
		
		//TODO proper tests here
	}
	
}
