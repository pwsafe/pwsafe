package org.pwsafe.lib.file;

import java.io.IOException;

import org.pwsafe.lib.UUID;
import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.PasswordSafeException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;

import junit.framework.TestCase;

/**
 * Test cases for sample v3 record.
 * @author Glen Smith
 *
 */
public class PwsFileV3Test extends TestCase {
	
	public void testPassphrase() throws EndOfFileException, IOException, UnsupportedFileVersionException {
		String filename = "/data/Java/PasswordSafeLib/sample3.psafe3";
		String password = "Pa$$word";
		
		PwsFileV3 file = new PwsFileV3(filename, password);
		file.readAll();
		System.out.println(file.getRecordCount());
		
	}
	
	public void testLargeFile() throws EndOfFileException, IOException, UnsupportedFileVersionException, Exception {
		String filename = "/data/Java/PasswordSafeLib/bigsample3.psafe3";
		String password = "password";
		
		PwsFileV3 file = (PwsFileV3) PwsFileFactory.newFile();
		file.setPassphrase(password);
		for (int i = 0; i < 10; i++) {
			if (i%10 == 0) { System.out.print("."); }
			PwsRecordV3 v3 = (PwsRecordV3) file.newRecord();
			
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.GROUP , "group"));
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.TITLE , "title" + i));
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.USERNAME , "user"+i));
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.PASSWORD , "pw" + i));
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.NOTES , "notes"+i));
			file.add(v3);
		}
		file.setFilename(filename);
		System.out.println("\nDone.");
		
		file.save();
		System.out.println(file.getRecordCount());
		
	}
	
	

}
