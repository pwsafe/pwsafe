package org.pwsafe.test;

import org.pwsafe.lib.Util;

import junit.framework.TestCase;

/**
 * @author Kevin Preece
 */
public class UtilTest extends TestCase
{
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

	public final void testByteToHex1()
	{
		byte	b = -92;

		assertEquals( Util.byteToHex(b), "a4" );
	}
	
	public final void testByteToHex2()
	{
		byte	b[] = new byte [] { -92, 0, -1, 1, 127, -128 };
		
		assertEquals( Util.bytesToHex(b), "a400ff017f80" );
	}
	
	public final void testByteToHex3()
	{
		byte	b[] = new byte [] { -92, 0, -1, 1, 127, -128 };
		
		assertEquals( Util.bytesToHex(b,2,4), "ff017f80" );
	}
	
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
	
	public final void testByteToHex6()
	{
		byte	b[] = new byte [] { -92, 0, -1, 1, 127, -128 };
		
		assertEquals( Util.bytesToHex(b,2,0), "" );
	}
	
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
	
	public final void testGetIntFromByteArray()
	{
		assertTrue( Util.getIntFromByteArray( new byte [] { 4, 0, 0, 0 }, 0 ) == 4 );
		assertTrue( Util.getIntFromByteArray( new byte [] { 120, 86, 52, 18 }, 0 ) == 0x12345678 );
	}
	
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

	public final void testBytesToString()
	{
		byte	b[]	= new byte [] { 1, 127, -128, -1 };

		assertEquals( Util.bytesToString(b), "{ 1, 127, -128, -1 }" );
	}
}
