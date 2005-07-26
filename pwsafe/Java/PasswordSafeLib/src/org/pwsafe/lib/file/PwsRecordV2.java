/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import java.io.IOException;
import java.util.Iterator;

import org.pwsafe.lib.Log;
import org.pwsafe.lib.UUID;
import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.UnimplementedConversionException;

/**
 * @author Kevin
 */
public class PwsRecordV2 extends PwsRecord
{
	private static final Log LOG = Log.getInstance(PwsRecordV2.class.getPackage().getName());

	/**
	 * Constant for the version 2 ID string field.
	 */
	public static final int		V2_ID_STRING		= 0;

	/**
	 * Constant for the Universally Unique ID (UUID) field. 
	 */
	public static final int		UUID				= 1;

	/**
	 * Constant for the group field.
	 */
	public static final int		GROUP				= 2;

	/**
	 * Constant for the title field.
	 */
	public static final int		TITLE				= 3;

	/**
	 * Constant for the username field.
	 */
	public static final int		USERNAME			= 4;

	/**
	 * Constant for the notes field.
	 */
	public static final int		NOTES				= 5;

	/**
	 * Constant for the passphrase field.
	 */
	public static final int		PASSWORD			= 6;

	/**
	 * Constant for the creation date field.
	 */
	public static final int		CREATION_TIME		= 7;

	/**
	 * Constant for the passphrase modification time field.
	 */
	public static final int		PASSWORD_MOD_TIME	= 8;

	/**
	 * Constant for the last access time field.
	 */
	public static final int		LAST_ACCESS_TIME	= 9;

	/**
	 * Constant for the passphrase lifetime field.
	 */
	public static final int		PASSWORD_LIFETIME	= 10;

	/**
	 * Constant for the passphrase policy field.
	 */
	public static final int		PASSWORD_POLICY		= 11;

	/**
	 * Constant for the end of record marker field.
	 */
	public static final int		END_OF_RECORD		= 255;

	/**
	 * All the valid type codes.
	 */
	private static final Object []	VALID_TYPES	= new Object []
	{
		new Object [] { new Integer(V2_ID_STRING),		"V2_ID_STRING",			PwsStringField.class },
		new Object [] { new Integer(UUID),				"UUID",					PwsUUIDField.class },
		new Object [] { new Integer(GROUP),				"GROUP",				PwsStringField.class },
		new Object [] { new Integer(TITLE),				"TITLE",				PwsStringField.class },
		new Object [] { new Integer(USERNAME),			"USERNAME",				PwsStringField.class },
		new Object [] { new Integer(NOTES),				"NOTES",				PwsStringField.class },
		new Object [] { new Integer(PASSWORD),			"PASSWORD",				PwsStringField.class },
		new Object [] { new Integer(CREATION_TIME),		"CREATION_TIME",		PwsTimeField.class },
		new Object [] { new Integer(PASSWORD_MOD_TIME),	"PASSWORD_MOD_TIME",	PwsTimeField.class },
		new Object [] { new Integer(LAST_ACCESS_TIME),	"LAST_ACCESS_TIME",		PwsTimeField.class },
		new Object [] { new Integer(PASSWORD_LIFETIME),	"PASSWORD_LIFETIME",	PwsIntegerField.class },
		new Object [] { new Integer(PASSWORD_POLICY),	"PASSWORD_POLICY",		PwsStringField.class }
	};

	/**
	 * Create a new record with all mandatory fields given their default value.
	 */
	PwsRecordV2()
	{
		super( VALID_TYPES );

		setField( new PwsUUIDField(UUID, new UUID()) );
		setField( new PwsStringField(TITLE,    "") );
		setField( new PwsStringField(PASSWORD, "") );
	}
	
	/**
	 * Create a new record by reading it from <code>file</code>.
	 * 
	 * @param file the file to read data from.
	 * 
	 * @throws EndOfFileException If end of file is reached
	 * @throws IOException        If a read error occurs.
	 */
	PwsRecordV2( PwsFile file )
	throws EndOfFileException, IOException
	{
		super( file, VALID_TYPES );
	}

	/**
	 * Creates a new record that is a copy <code>base</code>.
	 * 
	 * @param base the record to copy.
	 */
	PwsRecordV2( PwsRecord base )
	{
		super(base);
	}

	/**
	 * Creates a deep clone of this record.
	 * 
	 * @return the new record.
	 */
	public Object clone()
//	throws CloneNotSupportedException
	{
		return new PwsRecordV2( this );
	}

	/**
	 * Compares this record to another returning a value that is less than zero if
	 * this record is "less than" <code>other</code>, zero if they are "equal", or
	 * greater than zero if this record is "greater than" <code>other</code>. 
	 * 
	 * @param other the record to compare this record to.
	 * 
	 * @return A value &lt; zero if this record is "less than" <code>other</code>,
	 *         zero if they're equal and &gt; zero if this record is "greater than"
	 *         <code>other</code>.
	 * 
	 * @throws ClassCastException If <code>other</code> is not a <code>PwsRecordV1</code>.
	 * 
	 * @see java.lang.Comparable#compareTo(java.lang.Object)
	 */
	public int compareTo( Object other )
	{
		// TODO Implement me
		return 0;
	}

	/**
	 * Compares this record to another returning <code>true</code> if they're equal
	 * and <code>false</code> if they're unequal.
	 * 
	 * @param that the record this one is compared to.
	 * 
	 * @return <code>true</code> if the records are equal, <code>false</code> if 
	 *         they're unequal.
	 * 
	 * @throws ClassCastException if <code>that</code> is not a <code>PwsRecordV1</code>.
	 */
	public boolean equals( Object that )
	{
		UUID	thisUUID;
		UUID	thatUUID;

		if (that instanceof PwsRecordV2) {
			thisUUID = (UUID) ((PwsUUIDField) getField( UUID )).getValue();
			thatUUID = (UUID) ((PwsUUIDField) ((PwsRecord) that).getField( UUID )).getValue();

			return thisUUID.equals( thatUUID );
		} else {
			return false;
		}
	}

	/**
	 * Validates the record, returning <code>true</code> if it's valid or <code>false</code>
	 * if unequal.
	 * 
	 * @return <code>true</code> if it's valid or <code>false</code> if unequal.
	 */
	protected boolean isValid()
	{
		if ( ((PwsStringField) getField( TITLE )).equals(PwsFileV2.ID_STRING) )
		{
			LOG.debug1( "Ignoring record " + this.toString() );
			return false;
		}
		return true;
	}
	
	/**
	 * Initialises this record by reading its data from <code>file</code>.
	 * 
	 * @param file the file to read the data from.
	 * 
	 * @throws EndOfFileException
	 * @throws IOException
	 */
	protected void loadRecord( PwsFile file )
	throws EndOfFileException, IOException
	{
		Item		item;
		PwsField	itemVal	= null;
		
		for ( ;; )
		{
			item = new Item( file );

			if ( item.getType() == END_OF_RECORD )
			{
				LOG.debug2( "-- END OF RECORD --" );
				break; // out of the for loop
			}
			switch ( item.getType() )
			{
				case UUID :
					itemVal = new PwsUUIDField( item.getType(), item.getByteData() );
					break;

				case V2_ID_STRING :
				case GROUP :
				case TITLE :
				case USERNAME :
				case NOTES :
				case PASSWORD :
					itemVal	= new PwsStringField( item.getType(), item.getData() );
					break;

				case CREATION_TIME :
				case PASSWORD_MOD_TIME :
				case LAST_ACCESS_TIME :
					itemVal	= new PwsTimeField( item.getType(), item.getByteData() );
					break;

				case PASSWORD_LIFETIME :
					itemVal	= new PwsIntegerField( item.getType(), item.getByteData() );
					break;

				case PASSWORD_POLICY :
					break;
				
				default :
					throw new UnimplementedConversionException();
			}
			if ( LOG.isDebug2Enabled() ) LOG.debug2( "type=" + item.getType() + " (" + ((Object[])VALID_TYPES[item.getType()])[1] + "), value=\"" + itemVal.toString() + "\"" );
			setField( itemVal );
		}
	}

	/**
	 * Saves this record to <code>file</code>.
	 * 
	 * @param file the file that the record will be written to.
	 * 
	 * @throws IOException if a write error occurs.
	 * 
	 * @see org.pwsafe.lib.file.PwsRecord#saveRecord(org.pwsafe.lib.file.PwsFile)
	 */
	protected void saveRecord(PwsFile file)
	throws IOException
	{
		LOG.debug2( "----- START OF RECORD -----" );
		for ( Iterator iter = getFields(); iter.hasNext(); )
		{
			int			type;
			PwsField	value;

			type	= ((Integer) iter.next()).intValue();
			value	= getField( type );

			if ( LOG.isDebug2Enabled() ) LOG.debug2( "Writing field " + type + " (" + ((Object[])VALID_TYPES[type])[1] + ") : \"" + value.toString() + "\"" );

			writeField( file, value );
		}
		writeField( file, new PwsStringField( END_OF_RECORD, "" ) );
		LOG.debug2( "----- END OF RECORD -----" );
	}

	/**
	 * Returns a string representation of this record.
	 * 
	 * @return A string representation of this object.
	 */
	public String toString()
	{
		StringBuffer	sb;
		boolean			first;

		first	= true;
		sb		= new StringBuffer();

		sb.append( "{ " );
		
		for ( Iterator iter = getFields(); iter.hasNext(); )
		{
			Integer	key;
			String	value;

			key		= (Integer) iter.next();
			value	= getField( key ).toString();

			if ( !first )
			{
				sb.append( ", " );
			}
			first	= false;

			sb.append( ((Object[])VALID_TYPES[key.intValue()])[1] );
			sb.append( "=" );
			sb.append( value );
		}
		sb.append( " }" );

		return sb.toString();
	}
}
