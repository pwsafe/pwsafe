package org.pwsafe.lib.crypto;

import org.pwsafe.lib.Util;

import BlowfishJ.BlowfishCBC;

/**
 * An extension to the BlowfishJ.BlowfishCBC to allow it to be used for PasswordSafe. Byte 
 * order differences prevent BlowfishCBC being used directly.
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
	 * 
	 * @see BlowfishJ.BlowfishCBC#decrypt
	 */
	public void decrypt( byte[] buffer )
	{
		Util.bytesToLittleEndian( buffer );
		super.decrypt( buffer, 0, buffer, 0, buffer.length );
		Util.bytesToLittleEndian( buffer );
	}

//	/**
//	 * Decrypts the data in <code>inbuffer</code> putting the decrypted data into
//	 * <code>outbuffer</code>.
//	 * <p />
//	 * @param inbuffer  the buffer containing the encrypted data.
//	 * @param outbuffer the buffer to receive the decrypted data.
//	 */
//	public void decrypt( byte[] inbuffer, byte [] outbuffer )
//	{
//		System.arraycopy( inbuffer, 0, outbuffer, 0, inbuffer.length );
//		this.decrypt( outbuffer );
//	}

	/**
	 * Encrypts <code>buffer</code> in place.
	 * 
	 * @param buffer the buffer to be encrypted.
	 * 
	 * @see BlowfishJ.BlowfishCBC#encrypt(byte[])
	 */
	public void encrypt( byte[] buffer )
	{
		Util.bytesToLittleEndian( buffer );
		super.encrypt(buffer, 0, buffer, 0, buffer.length);
		Util.bytesToLittleEndian( buffer );
	}

	/**
	 * Sets the initial vector.
	 */
	public void setCBCIV( byte[] newCBCIV )
	{
		byte temp[] = new byte [ newCBCIV.length ];
		System.arraycopy( newCBCIV, 0, temp, 0, newCBCIV.length );
		Util.bytesToLittleEndian( temp );
		super.setCBCIV( temp, 0 );
	}

}
