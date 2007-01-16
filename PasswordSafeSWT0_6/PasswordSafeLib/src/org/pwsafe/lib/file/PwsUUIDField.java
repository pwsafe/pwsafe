/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import org.pwsafe.lib.UUID;

/**
 * @author Kevin Preece
 */
public class PwsUUIDField extends PwsField
{
	/**
	 * Constructor
	 * 
	 * @param type  the field's type.
	 * @param value the field's value.
	 */
	public PwsUUIDField( int type, byte [] value )
	{
		super( type, new UUID(value) );
	}

	/**
	 * Constructor
	 * 
	 * @param type  the field's type.
	 * @param value the field's value.
	 */
	public PwsUUIDField( int type, UUID value )
	{
		super( type, value );
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
		return ((UUID) super.getValue()).getBytes();
	}

	/**
	 * Compares this <code>PwsUUIDField</code> to another returning a value less than zero if
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
		return ((UUID) this.getValue()).compareTo( (UUID) ((PwsUUIDField) that).getValue() );
	}

	/**
	 * Compares this object to another <code>PwsUUIDField</code> or {@link UUID} returning
	 * <code>true</code> if they're equal or <code>false</code> otherwise.
	 * 
	 * @param arg0 the other object to compare to.
	 * 
	 * @return <code>true</code> if they're equal or <code>false</code> otherwise.
	 */
	public boolean equals( Object arg0 )
	{
		if ( arg0 instanceof PwsUUIDField )
		{
			return equals( (PwsUUIDField) arg0 );
		}
		else if ( arg0 instanceof UUID )
		{
			return equals( (UUID) arg0 );
		}
		throw new ClassCastException();
	}

	/**
	 * Compares this object to another <code>PwsUUIDField</code> returning
	 * <code>true</code> if they're equal or <code>false</code> otherwise.
	 * 
	 * @param arg0 the other object to compare to.
	 * 
	 * @return <code>true</code> if they're equal or <code>false</code> otherwise.
	 */
	public boolean equals( PwsUUIDField arg0 )
	{
		return ((UUID) getValue()).equals(arg0.getValue());
	}

	/**
	 * Compares this object to a {@link UUID} returning <code>true</code> if they're equal
	 * or <code>false</code> otherwise.
	 * 
	 * @param arg0 the other object to compare to.
	 * 
	 * @return <code>true</code> if they're equal or <code>false</code> otherwise.
	 */
	public boolean equals( UUID arg0 )
	{
		return ((UUID) getValue()).equals(arg0);
	}
}
