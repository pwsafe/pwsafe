/*
 * $Id: PwsIntegerField.java 297 2004-02-24 22:29:25Z preecek $
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;


/**
 * Provides a wrapper for fields that holds the passwordsafe file version.  
 * Integer values are stored in the database in little-endian
 * order and are converted to and from this format on writing and reading.
 * 
 * @author Glen Smith
 */
public class PwsVersionField extends PwsIntegerField
{
	/**
	 * Constructs the object 
	 * 
	 * @param type  the field type.  Values depend on the version of the file being read.
	 * @param value the byte array holding the integer value.
	 * 
	 * @throws IndexOutOfBoundsException If <code>value.length</code> &lt; 4.
	 */
	public PwsVersionField( int type, byte [] value )
	{
		super( type, new byte[] { 0, 0, value.length > 0 ? value[0] : 0, value.length > 1 ? value[1] : 0} );
	}

	/**
	 * Returns this integer as an array of bytes.  The returned array will have
	 * a length of PwsFile.BLOCK_LENGTH and is thus suitable to be written to the
	 * database.
	 * 
	 * @return a byte array containing the field's integer value.
	 * 
	 * @see org.pwsafe.lib.file.PwsField#getBytes()
	 */
	public byte[] getBytes()
	{
		byte[] intRetval	= super.getBytes();

		return new byte[] { intRetval[2], intRetval[3] };
	}


}
