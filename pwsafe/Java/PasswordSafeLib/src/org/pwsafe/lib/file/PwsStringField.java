package org.pwsafe.lib.file;

/**
 * @author Kevin Preece
 */
public class PwsStringField extends PwsField
{
	public PwsStringField( int type, String value )
	{
		super( type, value );
	}

	public byte[] getBytes()
	{
		return ((String) super.getValue()).getBytes();
	}

	public int compareTo( Object that )
	{
		return ((String) this.getValue()).compareTo((String) ((PwsStringField) that).getValue());
	}

	public boolean equals( Object arg0 )
	{
		if ( arg0 instanceof PwsStringField )
		{
			return equals( (PwsStringField) arg0 );
		}
		else if ( arg0 instanceof String )
		{
			return equals( (String) arg0 );
		}
		throw new ClassCastException();
	}

	public boolean equals( PwsStringField arg0 )
	{
		return ((String) super.getValue()).equals(arg0.getValue());
	}

	public boolean equals( String arg0 )
	{
		return ((String) super.getValue()).equals(arg0);
	}
}
