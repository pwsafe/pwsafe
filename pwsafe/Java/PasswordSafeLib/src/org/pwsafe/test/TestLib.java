/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.test;

import java.util.Iterator;

import org.pwsafe.lib.Log;
import org.pwsafe.lib.file.PwsFile;
import org.pwsafe.lib.file.PwsFileFactory;
import org.pwsafe.lib.file.PwsFileV1;
import org.pwsafe.lib.file.PwsFileV2;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;
import org.pwsafe.lib.file.PwsStringField;

/**
 * @author Kevin Preece
 */
public class TestLib
{
	private static final Log LOG = Log.getInstance(TestLib.class.getPackage().getName());

	/**
	 * Main entry point for this test class.
	 * 
	 * @param args
	 */
	public static void main(String[] args)
	{
		LOG.enterMethod( "main" );

		try
		{
//			PwsFile	file;
//	
//			file = PwsFileFactory.loadFile( "w.dat", "w" );
//			printFile( file );
//			file.setFilename( "w-new.dat" );
//			file.save();
//			LOG.debug1( "********************************************************************************" );
//			file = PwsFileFactory.loadFile( "w-new.dat", "w" );
//			printFile( file );
//			LOG.debug1( "********************************************************************************" );
//			file = PwsFileFactory.loadFile( "w-v2.dat", "w" );
//			printFile( file );
//			LOG.debug1( "********************************************************************************" );
//			file.setFilename( "w-v2-new.dat" );
//			file.save();
//			LOG.debug1( "********************************************************************************" );
//			file = PwsFileFactory.loadFile( "w-v2-new.dat", "w" );
//			printFile( file );
//			
//			file = PwsFileFactory.loadFile( "x.dat", "abcdefghijk" );
//			printFile( file );

			createV1File();
			createV2File();
		}
		catch ( Exception e )
		{
			LOG.error( e );
		}

		LOG.leaveMethod( "main" );
	}

	private static void printFile( PwsFile file )
	{
		LOG.debug1( "Begin print of file" );
		LOG.debug1( "Number of records = " + file.getRecordCount() );
		LOG.debug1( "File is modified = " + file.isModified() );

		for ( Iterator iter = file.getRecords(); iter.hasNext(); )
		{
			PwsRecord	rec;

			rec = (PwsRecord) iter.next();
			LOG.debug1( rec.toString() );
		}
	}

	private static void createV1File()
	throws Exception
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

		v1File.save();

		v1File = PwsFileFactory.loadFile( "V1 New File.dat", "passphrase" );
		printFile( v1File );
	}

	private static void createV2File()
	throws Exception
	{
		PwsFile		v2File;
		PwsRecord	rec;

		v2File	= new PwsFileV2();

		v2File.setFilename( "V2 New File.dat" );
		v2File.setPassphrase( "passphrase" );

		rec = v2File.newRecord();
		
		rec.setField( new PwsStringField(PwsRecordV2.TITLE, "Entry number 1") );
		rec.setField( new PwsStringField(PwsRecordV2.PASSWORD, "Password 1") );

		v2File.add( rec );
		
		rec = v2File.newRecord();
		
		rec.setField( new PwsStringField(PwsRecordV2.TITLE, "Entry number 2") );
		rec.setField( new PwsStringField(PwsRecordV2.PASSWORD, "Password 2") );
		rec.setField( new PwsStringField(PwsRecordV2.USERNAME, "Username 2") );
		rec.setField( new PwsStringField(PwsRecordV2.NOTES, "Notes line 1\r\nNotes line 2") );
		
		v2File.add( rec );

		v2File.save();

		v2File = PwsFileFactory.loadFile( "V2 New File.dat", "passphrase" );
		printFile( v2File );
	}

	private void showI18nStrings()
	{
//		Enumeration	enum;
//		
//		enum = I18nHelper.getBundle().getKeys();
//		
//		while ( enum.hasMoreElements() )
//		{
//			String	key;
//			String	val;
//			
//			key	= (String) enum.nextElement();
//			val	= I18nHelper.formatMessage( key );
//
//			System.out.println( key + " : " + val );
//		}
	}
}
