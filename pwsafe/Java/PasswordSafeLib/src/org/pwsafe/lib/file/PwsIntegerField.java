/*
 * Created on Feb 14, 2004
 *
 * To change the template for this generated file go to
 * Window - Preferences - Java - Code Generation - Code and Comments
 */
package org.pwsafe.lib.file;

import org.pwsafe.lib.Util;

/**
 * @author Kevin
 *
 * To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Generation - Code and Comments
 */
public class PwsIntegerField extends PwsField
{
	/**
	 * @param value
	 */
	public PwsIntegerField( int type, byte [] value )
	{
		super( type, new Integer( Util.getIntFromByteArray(value, 0) ) );
	}

	public byte[] getBytes()
	{
		int		value;
		byte	retval[];

		value	= (int) ((Integer) super.getValue()).intValue();
		retval	= PwsFile.allocateBuffer( 4 );

		Util.putIntToByteArray( retval, value, 0 );

		return retval;
	}

	/* (non-Javadoc)
	 * @see java.lang.Comparable#compareTo(java.lang.Object)
	 */
	public int compareTo( Object arg0 )
	{
		return ((Integer) getValue()).compareTo((Integer) ((PwsIntegerField) arg0).getValue());
	}

	public boolean equals( Object arg0 )
	{
		if ( arg0 instanceof PwsIntegerField )
		{
			return equals( (PwsIntegerField) arg0 );
		}
		else if ( arg0 instanceof Integer )
		{
			return equals( (Integer) arg0 );
		}
		throw new ClassCastException();
	}

	public boolean equals( PwsIntegerField arg0 )
	{
		return ((Integer) getValue()).equals(arg0.getValue());
	}

	public boolean equals( Integer arg0 )
	{
		return ((Integer) getValue()).equals(arg0);
	}
}
