package org.pwsafe.lib;

/**
 * This class exposes various utilty methods.
 */
public class Util
{
	private static final Log LOG = Log.getInstance(Util.class.getPackage().getName());

	public static final char	HEX_CHARS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	/**
	 * Private to prevent instatiation.
	 */
	private Util()
	{
	}

	/**
	 * Allocates a byte array with a length of <code>length</code> and fills it with
	 * random data.
	 * <p />
	 * @param length the length of the array.
	 * <p />
	 * @return A byte array initialised with random data.
	 */
	public static byte [] allocateByteArray( int length )
	{
		byte	array [];

		array = new byte [ length ];

		for ( int ii = 0; ii < length; ++ii )
		{
			array[ii] = newrand();
		}
		return array;
	}

	/**
	 * Compares two byte arrays returning <code>true</code> if they're equal in length and
	 * content <code>false</code> if they're not.
	 * <p />
	 * @param b1 the first byte array.
	 * @param b2 the seconf byte array.
	 * <p />
	 * @return <code>true</code> if the arrays are equal, <code>false</code> if they are not.
	 */
	public static boolean bytesAreEqual( byte [] b1, byte [] b2 )
	{
		if ( b1.length == b2.length )
		{
			for ( int ii = 0; ii < b1.length; ++ii )
			{
				if ( b1[ii] != b2[ii] )
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}

	/**
	 * Converts a byte to its unsigned hexadecimal equivalent.  For example a value of -92 converts
	 * to "a4".
	 * 
	 * @param b the byte to convert
	 * 
	 * @return A <code>String</code> representation of the value in hexadecimal
	 */
	public static String byteToHex( byte b )
	{
		LOG.enterMethod( "Util.byteToHex(byte)" );

		String str = new StringBuffer().append(HEX_CHARS[ (b >>> 4) & 0x0f ]).append(HEX_CHARS[ b & 0x0f ]).toString();

		LOG.leaveMethod( "Util.byteToHex(byte)" );

		return str;
	}

	/**
	 * Converts a byte array to a hexadecimal string.
	 * 
	 * @param b the byte array to be converted to a hex string.
	 * 
	 * @return The hexadecimal representation of the byte array contents.
	 */
	public static String bytesToHex( byte [] b )
	{
		LOG.enterMethod( "Util.bytesToHex(byte[])" );

		String str = bytesToHex( b, 0, b.length );
		
		LOG.leaveMethod( "Util.bytesToHex(byte[])" );

		return str;
	}

	/**
	 * Converts a byte array to a hexadecimal string.  Conversion starts at byte <code>offset</code>
	 * and continues for <code>length</code> bytes.
	 * 
	 * @param b      the array to be converted.
	 * @param offset the start offset within <code>b</code>.
	 * @param length the number of bytes to convert.
	 * 
	 * @return A string representation of the byte array.
	 * 
	 * @throws IllegalArgumentException if <code>length</code> is negative.
	 * @throws ArrayIndexOutOfBoundsException if <code>(offest + length) &gt; b.length</code>. 
	 */
	public static String bytesToHex( byte [] b, int offset, int length )
	{
		LOG.enterMethod( "Util.bytesToHex(byte[],int,int)" );

		if (LOG.isDebug2Enabled()) LOG.debug2( "offset = " + offset + ", length = " + length + ", byte array = " + bytesToString(b) );

		StringBuffer	sb;
		String			result;

		if ( length < 0 )
		{
			LOG.error( I18nHelper.formatMessage("E00008", new Object [] { new Integer(length) } ) );
			LOG.leaveMethod( "Util.bytesToHex(byte[],int,int) - (by throwing IllegalArgumentException)" );
			throw new IllegalArgumentException( "Lengh must be not be negative." );
		}

		sb = new StringBuffer();
		
		for ( int ii = offset; ii < (offset + length); ++ii )
		{
			sb.append( byteToHex(b[ii]) );
		}
		result = sb.toString();

		LOG.debug2( "Result is \"" + result + "\"" );
		LOG.leaveMethod( "Util.bytesToHex(byte[],int,int)" );
		
		return result;
	}

	/**
	 * Produces a string prepresentation of the byte array.
	 * <p />
	 * @param b the array to be processed.
	 * <p />
	 * @return A string representation of the byte array.
	 */
	public static String bytesToString( byte [] b )
	{
		StringBuffer	sb;
		
		sb = new StringBuffer();
		
		sb.append( "{ " );
		for ( int ii = 0; ii < b.length; ++ii )
		{
			if ( ii > 0 )
			{
				sb.append( ", " );
			}
			sb.append( b[ii] );
		}
		sb.append( " }" );

		return sb.toString();
	}

	/**
	 * Converts an array from the native big-endian order to the little-endian order
	 * used by PasswordSafe.  The array is transformed in-place.
	 * 
	 * @param src the array to be byte-swapped.
	 * 
	 * @throws IllegalArgumentException if the array length is zero or not a multiple of four bytes.
	 */
	public static void bytesToLittleEndian( byte [] src )
	{
		LOG.enterMethod( "Util.bytesToLittleEndian(byte[])" );

		byte	temp;

		if ( (src.length == 0) || ((src.length % 4) != 0) )
		{
			String	msg;

			msg = I18nHelper.formatMessage( "E00009", new Object [] { new Integer(src.length) } );

			LOG.error( msg );
			LOG.leaveMethod( "Util.bytesToLittleEndian(byte[]) - by throwing IllegalArgumentException" );
			throw new IllegalArgumentException( msg );
		}

		if (LOG.isDebug2Enabled()) LOG.debug2( "Bytes to be swapped = " + bytesToString(src) );

		for ( int ii = 0; ii < src.length; ii += 4 )
		{	
			temp		= src[ii];
			src[ii]		= src[ii+3];
			src[ii+3]	= temp;

			temp		= src[ii+1];
			src[ii+1]	= src[ii+2];
			src[ii+2]	= temp;
		}

		if (LOG.isDebug2Enabled()) LOG.debug2( "Bytes after swapping = " + bytesToString(src) );

		LOG.leaveMethod( "Util.bytesToLittleEndian(byte[])" );
	}

	/**
	 * Creates a clone of the given byte array.
	 * 
	 * @param src the array to be cloned.
	 * 
	 * @return An array of bytes equal in length and content to <code>src</code>.
	 */
	public static byte [] cloneByteArray( byte [] src )
	{
		LOG.enterMethod( "Util.cloneByteArray(byte[])" );

		byte []	dst = new byte[ src.length ];

		System.arraycopy( src, 0, dst, 0, src.length );

		LOG.leaveMethod( "Util.cloneByteArray(byte[])" );

		return dst;
	}

	/**
	 * Creates a new array of the given length.  If length is shorter than <code>src.length</code>
	 * then the new array contains the contents of <code>src</code> truncated at <code>length</code>
	 * bytes.  If length is greater than <code>src.length</code> then the new array is a copy of
	 * <code>src</code> with the excess bytes set to zero.
	 * 
	 * @param src     the array to be cloned.
	 * @param length  the size of the new array.
	 * 
	 * @return The new array.
	 */
	public static byte [] cloneByteArray( byte [] src, int length )
	{
		LOG.enterMethod( "Util.cloneByteArray(byte[],int)" );

		int		max	= (length < src.length) ? length : src.length;
		byte []	dst = new byte[ length ];

		System.arraycopy( src, 0, dst, 0, max );

		LOG.leaveMethod( "Util.cloneByteArray(byte[],int)" );

		return dst;
	}

	/**
	 * Extracts an int from a byte array.  The value is four bytes in little-endian order starting
	 * at <code>offset</code>.
	 * 
	 * @param buff the array to extract the int from
	 * 
	 * @return The value extracted.
	 * 
	 * @throws IndexOutOfBoundsException if offset is negative or <code>buff.length</code> &lt; <code>offset + 4</code>.
	 */
	public static int getIntFromByteArray( byte [] buff, int offset )
	{
		LOG.enterMethod( "Util.cloneByteArray(byte[],int)" );

		int result;

		result = ((int) buff[offset+0] & 0x0ff) 
			| (((int) buff[offset+1] & 0x0ff) << 8 )
			| (((int) buff[offset+2] & 0x0ff) << 16 )
			| (((int) buff[offset+3] & 0x0ff) << 24 );

		LOG.leaveMethod( "Util.cloneByteArray(byte[],int)" );
		
		return result;
	}

	/**
	 * Stores an integer in little endian order into <code>buff</code> starting at
	 * offset <code>offset</code>.
	 * 
	 * @param buff   the buffer to store the integer into.
	 * @param value  the integer value to store.
	 * @param offset the offset at which to store the value.
	 */
	public static void putIntToByteArray( byte [] buff, int value, int offset )
	{
		LOG.enterMethod( "Util.putIntToByteArray" );

		buff[offset+0]	= (byte)(value & 0xff);
		buff[offset+1]	= (byte)((value & 0xff00) >>> 8);
		buff[offset+2]	= (byte)((value & 0xff0000) >>> 16);
		buff[offset+3]	= (byte)((value & 0xff000000) >>> 24);
		
		LOG.leaveMethod( "Util.putIntToByteArray" );
	}

	/**
	 * Returns a random byte in the range -128 to +127.
	 * 
	 * @return A random byte.
	 */
	public static byte newrand()
	{
		int rand;
		
		while ( (rand = (int)(Math.random() * (double)Integer.MAX_VALUE) % 257) == 256);
		return (byte) rand;
	}
}
