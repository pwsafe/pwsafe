/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import java.util.Date;

import org.pwsafe.lib.Util;

/**
 * @author Kevin Preece
 */
public class PwsTimeField extends PwsField
{
	/**
	 * Constructor
	 * 
	 * @param type  the field's type.
	 * @param value the field's value.
	 */
	public PwsTimeField( int type, byte [] value )
	{
		super( type, new Date( Util.getMillisFromByteArray(value, 0)) );
	}

	/**
	 * Constructor
	 * 
	 * @param type  the field's type.
	 * @param value the field's value.
	 */
	public PwsTimeField( int type, Date aDate)
	{
		super( type, aDate);
	}

	/**
	 * Returns the field's value as a byte array.
	 * 
	 * @return A byte array containing the field's data.
	 * 
	 * @see org.pwsafe.lib.file.PwsField#getBytes()
	 */
	public byte[] getBytes()
	{
		long		value;
		byte	retval[];

		value	= (long) ((Date) getValue()).getTime();
		retval	= PwsFile.allocateBuffer( 4 );

		Util.putMillisToByteArray( retval, value, 0 );

		return retval;
	}

	/**
	 * Compares this <code>PwsTimeField</code> to another returning a value less than zero if
	 * <code>this</code> is "less than" <code>that</code>, zero if they're equal and greater
	 * than zero if <code>this</code> is "greater than" <code>that</code>.
	 * 
	 * @param that the other field to compare to. 
	 * 
	 * @return A value less than zero if <code>this</code> is "less than" <code>that</code>,
	 *         zero if they're equal and greater than zero if <code>this</code> is "greater
	 *         than" <code>that</code>.
	 */
	public int compareTo( Object that )
	{
		return ((Date) this.getValue()).compareTo((Date) ((PwsTimeField) that).getValue());
	}

	/**
	 * Compares this object to another <code>PwsTimeField</code> or <code>java.util.Date</code> returning
	 * <code>true</code> if they're equal or <code>false</code> otherwise.
	 * 
	 * @param arg0 the other object to compare to.
	 * 
	 * @return <code>true</code> if they're equal or <code>false</code> otherwise.
	 */
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

	/**
	 * Compares this object to another <code>PwsTimeField</code> returning
	 * <code>true</code> if they're equal or <code>false</code> otherwise.
	 * 
	 * @param arg0 the other object to compare to.
	 * 
	 * @return <code>true</code> if they're equal or <code>false</code> otherwise.
	 */
	public boolean equals( PwsTimeField arg0 )
	{
		return ((Date) getValue()).equals(arg0.getValue());
	}

	/**
	 * Compares this object to a <code>java.util.Date</code> returning <code>true</code> 
	 * if they're equal or <code>false</code> otherwise.
	 * 
	 * @param arg0 the other object to compare to.
	 * 
	 * @return <code>true</code> if they're equal or <code>false</code> otherwise.
	 */
	public boolean equals( Date arg0 )
	{
		return ((Date) getValue()).equals(arg0);
	}
}
