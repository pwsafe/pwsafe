/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.test;

import org.pwsafe.lib.Util;

import junit.framework.TestCase;

/**
 * @author Kevin Preece
 */
public class UtilTest extends TestCase
{
	/**
	 *
	 */
	public final void testAllocateByteArray()
	{
        final int length = 16;
        assertEquals(length, Util.allocateByteArray(length).length);
    }

	/**
	 * Unit test for {@link Util#bytesAreEqual(byte[], byte[])}.
	 */
	public final void testBytesAreEqual()
	{
		assertTrue(
			Util.bytesAreEqual(
				new byte [] { 1, -2, 3, -4, 5 },
				new byte [] { 1, -2, 3, -4, 5 }
			) );

		assertTrue(
			!Util.bytesAreEqual(
				new byte [] { 1, -2, 3, -4, 5, 6 },
				new byte [] { 1, -2, 3, -4, 5 }
			) );

		assertTrue(
			!Util.bytesAreEqual(
				new byte [] { 1, -2, 3, -4, 6 },
				new byte [] { 1, -2, 3, -4, 5 }
			) );
	}

	/**
	 * Unit test for {@link Util#byteToHex(byte)} for a single byte.
	 */
	public final void testByteToHex1()
	{
		byte	b = -92;

		assertEquals( Util.byteToHex(b), "a4" );
	}
	
	/**
	 * Unit test for {@link Util#bytesToHex(byte[])} for a byte array.
	 */
	public final void testByteToHex2()
	{
		byte	b[] = new byte [] { -92, 0, -1, 1, 127, -128 };
		
		assertEquals( Util.bytesToHex(b), "a400ff017f80" );
	}
	
	/**
	 * Unit test for {@link Util#bytesToHex(byte[], int, int)} for a byte array with offset and length.
	 */
	public final void testByteToHex3()
	{
		byte	b[] = new byte [] { -92, 0, -1, 1, 127, -128 };
		
		assertEquals( Util.bytesToHex(b,2,4), "ff017f80" );
	}
	
	/**
	 * Unit test for {@link Util#bytesToHex(byte[], int, int)} array index goes out of bounds.
	 */
	public final void testByteToHex4()
	{
		byte	b[] = new byte [] { -92, 0, -1, 1, 127, -128 };
		
		try
		{
			Util.bytesToHex(b,2,5);
			fail();
		}
		catch ( ArrayIndexOutOfBoundsException e )
		{
		}
	}
	
	/**
	 * Unit test for {@link Util#bytesToHex(byte[], int, int)} invalid length.
	 */
	public final void testByteToHex5()
	{
		byte	b[] = new byte [] { -92, 0, -1, 1, 127, -128 };
		
		try
		{
			Util.bytesToHex(b,2,-1);
			fail();
		}
		catch ( IllegalArgumentException e )
		{
		}
	}
	
	/**
	 * Unit test for {@link Util#bytesToHex(byte[], int, int)} length is zero.
	 */
	public final void testByteToHex6()
	{
		byte	b[] = new byte [] { -92, 0, -1, 1, 127, -128 };
		
		assertEquals( Util.bytesToHex(b,2,0), "" );
	}
	
	/**
	 * Unit test for {@link Util#cloneByteArray(byte[])}.
	 */
	public final void testCloneByteArray1()
	{
		byte	b[] = new byte[] { 1, 2, 3, 4, 5, 6, -1 };
		byte	c[] = Util.cloneByteArray(b);

		assertTrue( b.length == c.length );

		for ( int ii = 0; ii < b.length; ++ii )
		{
			if ( b[ii] != c[ii] )
			{
				fail( "Cloned array <> original array.");
			}
		}
	}
	
	/**
	 * Unit test for {@link Util#cloneByteArray(byte[], int)} length of clone is &lt; length of
	 * the original byte array.
	 */
	public final void testCloneByteArray2()
	{
		byte	b[] = new byte[] { 1, 2, 3, 4, 5, 6, -1 };
		byte	c[] = Util.cloneByteArray(b, 3);

		assertTrue( c.length == 3 );

		for ( int ii = 0; ii < c.length; ++ii )
		{
			if ( b[ii] != c[ii] )
			{
				fail( "Cloned array <> original array.");
			}
		}
	}
	
	/**
	 * Unit test for {@link Util#cloneByteArray(byte[], int)} - clone is longer than the
	 * original array.
	 */
	public final void testCloneByteArray3()
	{
		byte	b[] = new byte[] { 1, 2, 3, 4, 5, 6, -1 };
		byte	c[] = Util.cloneByteArray(b,8);

		assertTrue( c.length == 8 );

		for ( int ii = 0; ii < b.length; ++ii )
		{
			if ( b[ii] != c[ii] )
			{
				fail( "Cloned array <> original array.");
			}
		}
		for ( int ii = b.length; ii < c.length; ++ii )
		{	
			if ( c[ii] != 0 )
			{
				fail( "Excess bytes are not zero" );
			}
		}
	}
	
	/**
	 * Unit test for {@link Util#getIntFromByteArray(byte[], int)}.
	 */
	public final void testGetIntFromByteArray()
	{
		assertTrue( Util.getIntFromByteArray( new byte [] { 4, 0, 0, 0 }, 0 ) == 4 );
		assertTrue( Util.getIntFromByteArray( new byte [] { 120, 86, 52, 18 }, 0 ) == 0x12345678 );
	}
	
	/**
	 * Unit test for {@link Util#bytesToLittleEndian(byte[])}.
	 */
	public final void testBytesToLittleEndian()
	{
		byte	b1[] = new byte [] { 0, 1, 2, 3, 4, 5, 6, 7 };
		byte	b2[] = new byte [] { 3, 2, 1, 0, 7, 6, 5, 4 };

		Util.bytesToLittleEndian( b1 );
		
		for ( int ii = 0; ii < b1.length; ++ii )
		{
			if ( b1[ii] != b2[ii] )
			{
				fail( "Byte swapped array is incorrect at byte " + ii );
			}
		}
	}

	/**
	 * Unit test for {@link Util#bytesToString(byte[])}.
	 */
	public final void testBytesToString()
	{
		byte	b[]	= new byte [] { 1, 127, -128, -1 };

		assertEquals( Util.bytesToString(b), "{ 1, 127, -128, -1 }" );
	}
	
	public void testMillisAndByteArray() {
		byte[] longBuf = new byte[4];
		
		long input = 1000L;
		long output = millisRoundtrip(input, longBuf);
		assertEquals("Long time input / output not matching", input, output);
		
		input = 1001L;
		output = millisRoundtrip(input, longBuf);
		assertFalse("Long Rounding by 1000 failed", input == output);
		
		input = 1000L;
		byte[] shortBuf = new byte[2];
		try {
			output = millisRoundtrip(input, shortBuf);
			fail ("a time field needs a 4 bytes buffer");
		} catch (ArrayIndexOutOfBoundsException anEx) {
			// ok
		}
		
	}
	
	private long millisRoundtrip (long input, byte[] buffer) {
		Util.putMillisToByteArray(buffer, input, 0);
		return Util.getMillisFromByteArray(buffer, 0);
	}
}
