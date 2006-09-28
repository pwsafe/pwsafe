package org.pwsafe.lib.file;

import java.io.IOException;

import org.pwsafe.lib.exception.EndOfFileException;
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

}
