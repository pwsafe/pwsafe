package org.pwsafe.lib.file;

import java.io.IOException;
import java.util.Iterator;

import org.pwsafe.lib.Log;
import org.pwsafe.lib.UUID;
import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.UnimplementedConversionException;

/**
 * @author Kevin
 *
 * To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Generation - Code and Comments
 */
public class PwsRecordV2 extends PwsRecord
{
	private static final Log LOG = Log.getInstance(PwsRecordV2.class.getPackage().getName());

	public static final int		V2_ID_STRING		= 0;
	public static final int		UUID				= 1;
	public static final int		GROUP				= 2;
	public static final int		TITLE				= 3;
	public static final int		USERNAME			= 4;
	public static final int		NOTES				= 5;
	public static final int		PASSWORD			= 6;
	public static final int		CREATION_TIME		= 7;
	public static final int		PASSWORD_MOD_TIME	= 8;
	public static final int		LAST_ACCESS_TIME	= 9;
	public static final int		PASSWORD_LIFETIME	= 10;
	public static final int		PASSWORD_POLICY		= 11;
	public static final int		END_OF_RECORD		= 255;
	
	private static final int []	VALID_TYPES	= new int [] {
		V2_ID_STRING,
		UUID,
		GROUP,
		TITLE,
		USERNAME,
		NOTES,
		PASSWORD,
		CREATION_TIME,
		PASSWORD_MOD_TIME,
		LAST_ACCESS_TIME,
		PASSWORD_LIFETIME,
		PASSWORD_POLICY
		};

	private static final String []	TYPE_NAMES	= new String [] {
		"V2_ID_STRING",
		"UUID",
		"GROUP",
		"TITLE",
		"USERNAME",
		"NOTES",
		"PASSWORD",
		"CREATION_TIME",
		"PASSWORD_MOD_TIME",
		"LAST_ACCESS_TIME",
		"PASSWORD_LIFETIME",
		"PASSWORD_POLICY"
		};

	/**
	 * 
	 */
	public PwsRecordV2()
	{
		super( VALID_TYPES );
	}
	
	PwsRecordV2( PwsFile file )
	throws EndOfFileException, IOException
	{
		super( file, VALID_TYPES );
	}

	/**
	 * @param base
	 */
	PwsRecordV2( PwsRecord base )
	{
		super(base);
	}

	/**
	 * Returns a clone of this record.
	 */
	public Object clone()
	throws CloneNotSupportedException
	{
		return new PwsRecordV2( this );
	}

	public int compareTo(Object ob)
	{
		// TODO Implement me
		return 0;
	}

	public boolean equals( Object that )
	{
		UUID	thisUUID;
		UUID	thatUUID;

		thisUUID = (UUID) ((PwsUUIDField) getField( UUID )).getValue();
		thatUUID = (UUID) ((PwsUUIDField) ((PwsRecord) that).getField( UUID )).getValue();

		return thisUUID.equals( thatUUID );
	}

	protected boolean isValid()
	{
		PwsStringField	title;

		if ( ((PwsStringField) getField( TITLE )).equals(PwsFileV2.ID_STRING) )
		{
			LOG.debug1( "Ignoring record " + this.toString() );
			return false;
		}
		return true;
	}
	
	/**
	 * Initialises this record from the the given file.
	 */
	protected void loadRecord(PwsFile file)
	throws EndOfFileException, IOException
	{
		ReadItem	item;
		PwsField	itemVal	= null;
		
		for ( ;; )
		{
			item = new ReadItem( file );

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
			if ( LOG.isDebug2Enabled() ) LOG.debug2( "type=" + item.getType() + " (" + TYPE_NAMES[item.getType()] + "), value=\"" + itemVal.toString() + "\"" );
			setField( itemVal );
		}
	}

	/**
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

			if ( LOG.isDebug2Enabled() ) LOG.debug2( "Writing field " + type + " (" + TYPE_NAMES[type] + ") : \"" + value.toString() + "\"" );

			writeField( file, value );
		}
		writeField( file, new PwsStringField( END_OF_RECORD, "" ) );
		LOG.debug2( "----- END OF RECORD -----" );
	}

	/**
	 * 
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

			sb.append( TYPE_NAMES[key.intValue()] );
			sb.append( "=" );
			sb.append( value );
		}
		sb.append( " }" );

		return sb.toString();
	}
}
