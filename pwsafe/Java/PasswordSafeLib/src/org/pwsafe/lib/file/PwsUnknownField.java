/*
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import org.pwsafe.lib.Util;

/**
 * Bit bucket for fields we are not interested in processing but must
 * preserve in the output.
 * 
 * @author Glen Smith (based on the work of Kevin Preece)
 */
public class PwsUnknownField extends PwsField
{
	/**
	 * Constructor
	 * 
	 * @param type  the field's type.
	 * @param value the field's value.
	 */
	public PwsUnknownField( int type, byte [] value )
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
		return ((byte[]) super.getValue());
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
		byte[] thisB = (byte[]) this.getValue();
		byte[] thatB = (byte[]) ((PwsUnknownField) that).getValue();
		return thisB[0] > thatB[0] ? 1 : 0;
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
		if ( arg0 instanceof PwsUnknownField )
		{
			return equals( (PwsUnknownField) arg0 );
		}
		throw new ClassCastException();
	}

	/**
	 * Compares this object to another <code>PwsUnknownField</code> returning
	 * <code>true</code> if they're equal or <code>false</code> otherwise.
	 * 
	 * @param other the other object to compare to.
	 * 
	 * @return <code>true</code> if they're equal or <code>false</code> otherwise.
	 */
	public boolean equals( PwsUnknownField other )
	{
		return Util.bytesAreEqual((byte[])getValue(), (byte[])other.getValue());
	}
	
	public String toString() {
		return new String(((byte[]) super.getValue()));
	}

}
