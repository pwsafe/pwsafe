package org.pwsafe.lib;

import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.TimeZone;


/**
 * A naive implementation of a UUID class.
 * 
 * @author Kevin Preece
 */
public class UUID implements Comparable
{
	private byte []		TheUUID	= new byte[ 16 ];

	public UUID()
	{
		Calendar	cal;
		long		time;
		int			time_hi;
		int			time_lo;

		time	  = new GregorianCalendar( 1582, 9, 15 ).getTimeInMillis();	// 15-Oct-1582
		time	  = Calendar.getInstance(TimeZone.getTimeZone("UCT")).getTimeInMillis() - time;
		time	<<= 19;
		time	 &= 0x0fffffffffffffffL;
		time	 |= 0x1000000000000000L;
		time_hi	  = (int)(time >>> 32);
		time_lo	  = (int) time;

		Util.putIntToByteArray( TheUUID, time_lo, 0 );
		Util.putIntToByteArray( TheUUID, time_hi, 4 );
		
		TheUUID[0]	= Util.newrand();
		TheUUID[1]	= Util.newrand();
		TheUUID[2]	= (byte)(Util.newrand() & 0x07);
		TheUUID[8]	= (byte)(Util.newrand() & 0x3f);
		TheUUID[9]	= Util.newrand();
		TheUUID[10]	= Util.newrand();
		TheUUID[11]	= Util.newrand();
		TheUUID[12]	= Util.newrand();
		TheUUID[13]	= Util.newrand();
		TheUUID[14]	= Util.newrand();
		TheUUID[15]	= Util.newrand();
	}

	public UUID( byte [] uuid )
	{
		if ( uuid.length != TheUUID.length )
		{
			throw new IllegalArgumentException();
		}
		System.arraycopy( uuid, 0, TheUUID, 0, TheUUID.length );
	}
	
	public boolean equals( Object ob )
	{
		if ( ob instanceof UUID )
		{
			return equals( (UUID) ob );
		}
		return false;
	}

	public int compareTo( Object arg0 )
	{
		return compareTo( (UUID) arg0 );
	}

	public int compareTo( UUID other )
	{
		for ( int ii = 0; ii < TheUUID.length; ++ii )
		{
			int	b1 = ((int) this.TheUUID[ii]) & 0x0ff;
			int	b2 = ((int) other.TheUUID[ii]) & 0x0ff;
			
			if ( b1 < b2 )
			{
				return -1;
			}
			else if ( b1 > b2 )
			{
				return 1;
			}
		}
		return 0;
	}

	public byte [] getBytes()
	{
		return Util.cloneByteArray( TheUUID );
	}

	public String toString()
	{
		return toString( TheUUID );
	}

	/**
	 * Converts this UUID into human-readable form.  The string has the format:
	 * {01234567-89ab-cdef-0123-456789abcdef} 
	 */
	public static String toString( byte [] uuid )
	{
		if ( uuid.length != 16 )
		{
			throw new IllegalArgumentException();
		}

		StringBuffer	sb;

		sb = new StringBuffer();

		sb.append( '{' );
		sb.append( Util.bytesToHex(uuid, 0, 4) );
		sb.append( '-' );
		sb.append( Util.bytesToHex(uuid, 4, 2) );
		sb.append( '-' );
		sb.append( Util.bytesToHex(uuid, 6, 2) );
		sb.append( '-' );
		sb.append( Util.bytesToHex(uuid, 8, 2) );
		sb.append( '-' );
		sb.append( Util.bytesToHex(uuid, 10, 6) );
		sb.append( '}' );
		
		return sb.toString();
	}
}
