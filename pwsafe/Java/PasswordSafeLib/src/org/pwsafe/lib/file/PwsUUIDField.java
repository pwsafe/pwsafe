package org.pwsafe.lib.file;

import org.pwsafe.lib.UUID;

/**
 * @author Kevin Preece
 */
public class PwsUUIDField extends PwsField
{
	/**
	 * @param value
	 */
	public PwsUUIDField( int type, byte [] value )
	{
		super( type, new UUID(value) );
	}

	public byte[] getBytes()
	{
		return ((UUID) super.getValue()).getBytes();
	}

	public int compareTo( Object that )
	{
		return ((UUID) this.getValue()).compareTo( (UUID) ((PwsTimeField) that).getValue() );
	}

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

	public boolean equals( PwsUUIDField arg0 )
	{
		return ((UUID) getValue()).equals(arg0.getValue());
	}

	public boolean equals( UUID arg0 )
	{
		return ((UUID) getValue()).equals(arg0);
	}
}
