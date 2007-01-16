/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.crypto;

import org.pwsafe.lib.Util;

import net.sourceforge.blowfishj.BlowfishCBC;

/**
 * An extension to the BlowfishJ.BlowfishCBC to allow it to be used for PasswordSafe. Byte 
 * order differences prevent BlowfishCBC being used directly.
 * 
 * @author Kevin Preece
 */
public class BlowfishPws extends BlowfishCBC
{
	/**
	 * Constructor, sets the initial vector to zero.
	 * 
	 * @param bfkey the encryption/decryption key.
	 */
	public BlowfishPws( byte[] bfkey )
	{
		super(bfkey, 0, bfkey.length);
	}

	/**
	 * Constructor, sets the initial vector to the value given.
	 * 
	 * @param bfkey      the encryption/decryption key.
	 * @param lInitCBCIV the initial vector.
	 */
	public BlowfishPws( byte[] bfkey, long lInitCBCIV )
	{
		super(bfkey, 0, bfkey.length, lInitCBCIV);
	}

	/**
	 * Constructor, sets the initial vector to the value given.
	 * 
	 * @param bfkey      the encryption/decryption key.
	 * @param initCBCIV the initial vector.
	 */
	public BlowfishPws( byte[] bfkey, byte[] initCBCIV )
	{
		super( bfkey, 0, bfkey.length );
		setCBCIV( initCBCIV );
	}

	/**
	 * Decrypts <code>buffer</code> in place.
	 * 
	 * @param buffer the buffer to be decrypted.
	 */
	public void decrypt( byte[] buffer )
	{
		Util.bytesToLittleEndian( buffer );
		super.decrypt( buffer, 0, buffer, 0, buffer.length );
		Util.bytesToLittleEndian( buffer );
	}

	/**
	 * Encrypts <code>buffer</code> in place.
	 * 
	 * @param buffer the buffer to be encrypted.
	 */
	public void encrypt( byte[] buffer )
	{
		Util.bytesToLittleEndian( buffer );
		super.encrypt(buffer, 0, buffer, 0, buffer.length);
		Util.bytesToLittleEndian( buffer );
	}

	/**
	 * Sets the initial vector.
	 * 
	 * @param newCBCIV the new value for the initial vector.
	 */
	public void setCBCIV( byte[] newCBCIV )
	{
		byte temp[] = new byte [ newCBCIV.length ];
		System.arraycopy( newCBCIV, 0, temp, 0, newCBCIV.length );
		Util.bytesToLittleEndian( temp );
		super.setCBCIV( temp, 0 );
	}

}
