package org.pwsafe.lib.file;

import java.io.IOException;
import java.io.UnsupportedEncodingException;

import org.pwsafe.lib.exception.EndOfFileException;

/**
 * @author Kevin Preece
 */
public class PwsRecordV1 extends PwsRecord implements Comparable
{
	private static final int		DEFAULT_TYPE	= 0;
	private static final String		DEFAULT_VALUE	= "";

	private static String		DefUserString;
	private static String		SplitChar;
	private static String		SplitString;

	public static final int		TITLE		= 3;
	public static final int		USERNAME	= 4;
	public static final int		NOTES		= 5;
	public static final int		PASSWORD	= 6;
	
	private static final int []	VALID_TYPES	= new int [] { TITLE, USERNAME, NOTES, PASSWORD };

	static
	{
		try
		{
			DefUserString	= new String( new byte[] { -96 }, PwsRecord.DEFAULT_CHARSET );
			SplitChar		= new String( new byte[] { -83 }, PwsRecord.DEFAULT_CHARSET ); 
			SplitString		= new String( new byte[] { 32, 32, -83, 32, 32 }, PwsRecord.DEFAULT_CHARSET ); 
		}
		catch ( UnsupportedEncodingException e )
		{
			// Should never get here - DEFAULT_CHARSET must be supported by all Java implementations.
		}
	}

	public PwsRecordV1()
	{
		super( VALID_TYPES );

		// Set default values
		setField( new PwsStringField(TITLE,    DEFAULT_VALUE) );
		setField( new PwsStringField(USERNAME, DEFAULT_VALUE) );
		setField( new PwsStringField(PASSWORD, DEFAULT_VALUE) );
		setField( new PwsStringField(NOTES,    DEFAULT_VALUE) );
	}

	PwsRecordV1( PwsFile file )
	throws EndOfFileException, IOException
	{
		super( file, VALID_TYPES );
	}
	
	PwsRecordV1( PwsRecordV1 base )
	{
		super( base );
	}

	public Object clone() throws CloneNotSupportedException
	{
		return new PwsRecordV1( this );
	}
	
	public int compareTo( Object ob )
	{
		return compareTo( (PwsRecordV1) ob );
	}

	private int compareTo( PwsRecordV1 rec )
	{
		int	retCode;
		
		if ( (retCode = getField(TITLE).compareTo(rec.getField(TITLE))) == 0 )
		{
			if ( (retCode = getField(USERNAME).compareTo(rec.getField(USERNAME))) == 0 )
			{
				return getField(NOTES).compareTo( rec.getField(NOTES) );
			}
		}
		return retCode;
	}

	public boolean equals( Object rec )
	{
		return equals( (PwsRecordV1) rec );
	}

	private boolean equals( PwsRecordV1 rec )
	{
		return ( getField(NOTES).equals( rec.getField(NOTES) )
		&& getField(PASSWORD).equals( rec.getField(PASSWORD) )
		&& getField(TITLE).equals( rec.getField(TITLE) )
		&& getField(USERNAME).equals( rec.getField(USERNAME) ) );
	}

	protected boolean isValid()
	{
		// All records should be added to the file.
		return true;
	}
	
	/**
	 * Initialises this record from the the given file.
	 */
	protected void loadRecord( PwsFile file )
	throws EndOfFileException, IOException
	{
		ReadItem	itm;
		String		str;
		int			pos;
		
		str	= new ReadItem(file).getData();
		
		pos = str.indexOf( SplitChar );
		if ( pos == -1 )
		{
			// This is not a composite of title and username

			pos = str.indexOf( DefUserString );
			if ( pos == -1 )
			{
				setField( new PwsStringField(TITLE, str) );
			}
			else
			{
				setField( new PwsStringField(TITLE, str.substring( 0, pos )) );
			}
			setField( new PwsStringField(USERNAME, DEFAULT_VALUE) );
		}
		else
		{
			setField( new PwsStringField(TITLE, str.substring( 0, pos ).trim()) );
			setField( new PwsStringField(USERNAME, str.substring( pos + 1 ).trim()) );
		}
		setField( new PwsStringField(PASSWORD, new ReadItem(file).getData()) );
		setField( new PwsStringField(NOTES, new ReadItem(file).getData()) );
	}

	/* (non-Javadoc)
	 * @see org.pwsafe.lib.file.PwsRecord#save(org.pwsafe.lib.file.PwsFile)
	 */
	protected void saveRecord(PwsFile file) throws IOException
	{
		PwsField	title;

		if ( getField(USERNAME).toString().trim().length() == 0 )
		{
			title = getField(TITLE);
		}
		else
		{	
			title = new PwsStringField( TITLE, getField(TITLE).toString() + SplitString + getField(USERNAME).toString() );
		}

		writeField( file, title, DEFAULT_TYPE );
		writeField( file, getField(PASSWORD), DEFAULT_TYPE );
		writeField( file, getField(NOTES), DEFAULT_TYPE );
	}

	/**
	 * Returns a string representation of this record.
	 * 
	 * @return A string representation of this object.
	 */
	public String toString()
	{
		StringBuffer	sb;
		String			notes;

		sb = new StringBuffer();
		sb.append( "{ \"" );
		sb.append( getField(TITLE) );
		sb.append( "\", \"" );
		sb.append( getField(USERNAME) );
		sb.append( "\", \"" );
		sb.append( getField(PASSWORD) );
		sb.append( "\" }" );

		return sb.toString();
	}
}
