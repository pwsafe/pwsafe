package org.pwsafe.lib.file;

import java.util.Date;

import org.pwsafe.lib.Util;

/**
 * @author Kevin Preece
 */
public class PwsTimeField extends PwsField
{
	/**
	 * @param value
	 */
	public PwsTimeField( int type, byte [] value )
	{
		super( type, new Date( (long) Util.getIntFromByteArray(value, 0) ) );
	}

	public byte[] getBytes()
	{
		int		value;
		byte	retval[];

		value	= (int) ((Date) super.getValue()).getTime();
		retval	= PwsFile.allocateBuffer( 4 );

		Util.putIntToByteArray( retval, value, 0 );

		return retval;
	}

	public int compareTo( Object that )
	{
		return ((Date) this.getValue()).compareTo((Date) ((PwsTimeField) that).getValue());
	}

	public boolean equals( Object arg0 )
	{
		if ( arg0 instanceof PwsTimeField )
		{
			return equals( (PwsTimeField) arg0 );
		}
		else if ( arg0 instanceof Date )
		{
			return equals( (Date) arg0 );
		}
		throw new ClassCastException();
	}

	public boolean equals( PwsTimeField arg0 )
	{
		return ((Date) getValue()).equals(arg0.getValue());
	}

	public boolean equals( Date arg0 )
	{
		return ((Date) getValue()).equals(arg0);
	}
}
