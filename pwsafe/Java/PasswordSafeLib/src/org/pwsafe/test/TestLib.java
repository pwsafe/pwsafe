package org.pwsafe.test;

import java.util.Iterator;

import org.pwsafe.lib.Log;
import org.pwsafe.lib.UUID;
import org.pwsafe.lib.file.PwsFile;
import org.pwsafe.lib.file.PwsFileFactory;
import org.pwsafe.lib.file.PwsRecord;

/**
 * @author Kevin Preece
 */
public class TestLib
{
	private static final Log LOG = Log.getInstance(TestLib.class.getPackage().getName());

	public static void main(String[] args)
	{
		LOG.enterMethod( "main" );

		try
		{
			PwsFile	file;
			UUID	uuid;

			uuid = new UUID();
	
			file = PwsFileFactory.loadFile( "w.dat", "w" );
			printFile( file );
//			file.setFilename( "w-new.dat" );
//			file.save();
//			LOG.debug1( "********************************************************************************" );
//			file = PwsFileFactory.loadFile( "w-new.dat", "w" );
//			printFile( file );
			LOG.debug1( "********************************************************************************" );
			file = PwsFileFactory.loadFile( "w-v2.dat", "w" );
			printFile( file );
			LOG.debug1( "********************************************************************************" );
//			file.setFilename( "w-v2-new.dat" );
//			file.save();
//			LOG.debug1( "********************************************************************************" );
//			file = PwsFileFactory.loadFile( "w-v2-new.dat", "w" );
//			printFile( file );
			
//			file = PwsFileFactory.loadFile( "x.dat", "abcdefghijk" );
//			printFile( file );

			for ( Iterator iter = file.getRecords(); iter.hasNext(); )
			{
				PwsRecord rec = (PwsRecord) iter.next();
				LOG.debug1( "Record = " + rec.toString() );
				iter.remove();
				LOG.debug1( "Number of records = " + file.getRecordCount() );
				LOG.debug1( "File is modified = " + file.isModified() );
			}
		}
		catch ( Exception e )
		{
			LOG.error( e );
		}

		LOG.leaveMethod( "main" );
	}

	public static void printFile( PwsFile file )
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
}
