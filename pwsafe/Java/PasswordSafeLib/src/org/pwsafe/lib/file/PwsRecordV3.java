/*
 * $Id: PwsRecordV2.java 561 2005-07-26 10:00:19 +0000 (Tue, 26 Jul 2005) glen_a_smith $
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import java.io.IOException;
import java.util.Date;
import java.util.Iterator;

import org.pwsafe.lib.Log;
import org.pwsafe.lib.UUID;
import org.pwsafe.lib.Util;
import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.UnimplementedConversionException;

/**
 * Support for new v3 Record type.
 * 
 * @author Glen Smith (based on Kevin's code for V2 records)
 */
public class PwsRecordV3 extends PwsRecord
{
	private static final Log LOG = Log.getInstance(PwsRecordV3.class.getPackage().getName());

	/**
	 * Constant for the version 3 ID string field.
	 */
	public static final int		V3_ID_STRING		= 0;

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
	 * Constant for the last modification time field.
	 */
	public static final int		LAST_MOD_TIME	= 12;
	
	/**
	 * Constant for URL related to this entry.
	 */
	public static final int		URL	= 13;
	
	/**
	 * Constant for Autotype information related to this entry.
	 */
	public static final int		AUTOTYPE	= 14;
	
	/**
	 * History of recently used passwords.
	 */
	public static final int     PASSWORD_HISTORY = 15;
	
	/**
	 * Constant for the end of record marker field.
	 */
	public static final int		END_OF_RECORD		= 255;

	/**
	 * All the valid type codes.
	 */
	private static final Object []	VALID_TYPES	= new Object []
	{
		new Object [] { new Integer(V3_ID_STRING),		"V3_ID_STRING",			PwsVersionField.class },
		new Object [] { new Integer(UUID),				"UUID",					PwsUUIDField.class },
		new Object [] { new Integer(GROUP),				"GROUP",				PwsStringUnicodeField.class },
		new Object [] { new Integer(TITLE),				"TITLE",				PwsStringUnicodeField.class },
		new Object [] { new Integer(USERNAME),			"USERNAME",				PwsStringUnicodeField.class },
		new Object [] { new Integer(NOTES),				"NOTES",				PwsStringUnicodeField.class },
		new Object [] { new Integer(PASSWORD),			"PASSWORD",				PwsStringUnicodeField.class },
		new Object [] { new Integer(CREATION_TIME),		"CREATION_TIME",		PwsTimeField.class },
		new Object [] { new Integer(PASSWORD_MOD_TIME),	"PASSWORD_MOD_TIME",	PwsTimeField.class },
		new Object [] { new Integer(LAST_ACCESS_TIME),	"LAST_ACCESS_TIME",		PwsTimeField.class },
		new Object [] { new Integer(PASSWORD_LIFETIME),	"PASSWORD_LIFETIME",	PwsTimeField.class },
		new Object [] { new Integer(PASSWORD_POLICY),	"PASSWORD_POLICY",		PwsStringUnicodeField.class },
		new Object [] { new Integer(LAST_MOD_TIME),		"LAST_MOD_TIME",		PwsTimeField.class },
		new Object [] { new Integer(URL),				"URL",					PwsStringUnicodeField.class },
		new Object [] { new Integer(AUTOTYPE),			"AUTOTYPE",				PwsStringUnicodeField.class },
		new Object [] { new Integer(PASSWORD_HISTORY),	"PASSWORD_HISTORY",		PwsStringUnicodeField.class },
	};
	

	/**
	 * Create a new record with all mandatory fields given their default value.
	 */
	PwsRecordV3()
	{
		super( VALID_TYPES );

		setField( new PwsUUIDField(UUID, new UUID()) );
		setField( new PwsStringUnicodeField(TITLE,    "") );
		setField( new PwsStringUnicodeField(PASSWORD, "") );
		setField( new PwsTimeField(CREATION_TIME, new Date()) );
	       
	}
	
	/**
	 * Create a new record by reading it from <code>file</code>.
	 * 
	 * @param file the file to read data from.
	 * 
	 * @throws EndOfFileException If end of file is reached
	 * @throws IOException        If a read error occurs.
	 */
	PwsRecordV3( PwsFile file )
	throws EndOfFileException, IOException
	{
		super( file, VALID_TYPES );
	}
	
	/**
	 * A special version which reads and ignores all headers since they have different
	 * ids to standard types.
	 * 
	 * @param file the file to read data from.
	 * @param validTypes the types allowable in the incoing data
	 * @throws EndOfFileException If end of file is reached
	 * @throws IOException        If a read error occurs.
	 */
	PwsRecordV3( PwsFile file, boolean ignoreFieldTypes)
	throws EndOfFileException, IOException	{
		super(file, VALID_TYPES, ignoreFieldTypes);
	}

	/**
	 * Creates a new record that is a copy <code>base</code>.
	 * 
	 * @param base the record to copy.
	 */
	PwsRecordV3( PwsRecord base )
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
		return new PwsRecordV3( this );
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

		if (that instanceof PwsRecordV3) {
			thisUUID = (UUID) ((PwsUUIDField) getField( UUID )).getValue();
			thatUUID = (UUID) ((PwsUUIDField) ((PwsRecord) that).getField( UUID )).getValue();

			return thisUUID.equals( thatUUID );
		} else {
			return false;
		}
	}

	/**
	 * Checks to see whether this record is one that we should display to the user
	 * or not. The header record is the only one we suppress, and we determine
	 * the header record by checking for the presence of the type 0 field which
	 * represents the file format version. 
	 * 
	 * @return <code>true</code> if it's valid or <code>false</code> if unequal.
	 */
	protected boolean isValid()
	{
		//TODO Ignore those records we read from the header....
		PwsField idField = getField(V3_ID_STRING);
		
		if ( idField != null ) {
			LOG.debug1( "Ignoring record " + this.toString() );
			return false;
		}
		return true;
	}
	
	protected boolean isHeaderRecord() {

		PwsField idField = getField(V3_ID_STRING);

		if (idField != null) {
			LOG.debug1("Ignoring record " + this.toString());
			return true;
		}
		return false;
	}
	
	static byte[] EOF_BYTES_RAW = "PWS3-EOFPWS3-EOF".getBytes();
	
	protected class ItemV3 extends Item {
		public ItemV3( PwsFileV3 file )
		throws EndOfFileException, IOException
		{
			super();
			try {
				RawData = file.readBlock();
			} catch (EndOfFileException eofe) {
				Data = new byte[32]; // to hold closing HMAC
				file.readBytes(Data);
				byte[] hash = file.hasher.doFinal();
				if (!Util.bytesAreEqual(Data, hash)) {
					LOG.error("HMAC record did not match. File may have been tampered");
					throw new IOException("HMAC record did not match. File has been tampered");
				}
				throw eofe;
			}
			
			Length	= Util.getIntFromByteArray( RawData, 0 );
			Type = RawData[4] & 0x000000ff; // rest of header is now random data
			Data    = new byte[Length];
			byte[] remainingDataInRecord = Util.getBytes(RawData, 5, 11);
			if (Length <= 11) {
				Util.copyBytes(Util.getBytes(remainingDataInRecord, 0, Length), Data);
			} else if (Length > 11) {
				int bytesToRead = Length - 11;
				int blocksToRead = bytesToRead / file.getBlockSize();
				
				// if blocksToRead doesn't fit neatly into current block 
				// size, add an extra block for the remaining bytes
				if (bytesToRead % file.getBlockSize() != 0)
					blocksToRead++;
				
				byte[] remainingRecords = new byte[0];
				for (int i = 0; i < blocksToRead; i++) {
					byte[] nextBlock = new byte[file.getBlockSize()];
					file.readDecryptedBytes( nextBlock );
					if (i == blocksToRead - 1) {
						// last block, do magic
						nextBlock = Util.getBytes(nextBlock, 0, bytesToRead - remainingRecords.length);
					}
					remainingRecords = Util.mergeBytes(remainingRecords, nextBlock);
				}
				Data = Util.mergeBytes(remainingDataInRecord, remainingRecords);
			}
			byte[] dataToHash = Data;
			file.hasher.digest(dataToHash);
		
		}
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
			item = new ItemV3( (PwsFileV3)file );

			if ( item.getType() == END_OF_RECORD )
			{
				LOG.debug2( "-- END OF RECORD --" );
				break; // out of the for loop
			}
			
			if (ignoreFieldTypes) {
				// header record has no valid types...
				itemVal = new PwsUnknownField(item.getType(), item.getByteData());
				Attributes.put( new Integer(item.getType()), itemVal );
			} else {
				
				switch ( item.getType() )
				{
					case V3_ID_STRING :
						//itemVal	= new PwsIntegerField( item.getType(), new byte[] {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} );
						itemVal	= new PwsVersionField( item.getType(), item.getByteData() );
						break;
						
					case UUID :
						itemVal = new PwsUUIDField( item.getType(), item.getByteData() );
						break;
				
					case GROUP :
					case TITLE :
					case USERNAME :
					case NOTES :
					case PASSWORD :
					case PASSWORD_POLICY :
					case PASSWORD_HISTORY :
					case URL :
					case AUTOTYPE :
						itemVal	= new PwsStringUnicodeField( item.getType(), item.getByteData() );
						break;
	
					case CREATION_TIME :
					case PASSWORD_MOD_TIME :
					case LAST_ACCESS_TIME :
					case LAST_MOD_TIME : 
						itemVal	= new PwsTimeField( item.getType(), item.getByteData() );
						break;
	
					case PASSWORD_LIFETIME :
						itemVal	= new PwsTimeField( item.getType(), item.getByteData() );
						break;
	
//					case PASSWORD_POLICY :
//						break;
//					
//					case PASSWORD_HISTORY : 
//						break;
					
					default :
						itemVal = new PwsUnknownField(item.getType(), item.getByteData());
						break;
						//throw new UnimplementedConversionException();
				}
				if ( LOG.isDebug2Enabled() ) LOG.debug2( "type=" + item.getType() + " (" + ((Object[])VALID_TYPES[item.getType()])[1] + "), value=\"" + itemVal.toString() + "\"" );
				setField( itemVal );
			}
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
			
			PwsFileV3 fileV3 = (PwsFileV3) file;
			fileV3.hasher.digest(value.getBytes());
		}
		writeField( file, new PwsStringField( END_OF_RECORD, "" ) );
		LOG.debug2( "----- END OF RECORD -----" );
	}
	
	/**
	 * Writes a single field to the file.
	 * 
	 * @param file  the file to write the field to.
	 * @param field the field to be written.
	 * @param type  the type to write to the file instead of <code>field.getType()</code> 
	 * 
	 * @throws IOException
	 */
	protected void writeField( PwsFile file, PwsField field, int type )
	throws IOException
	{
		byte	lenBlock[];
		byte	dataBlock[];

		lenBlock	= new byte[ 5 ];
		dataBlock	= field.getBytes();
		
		Util.putIntToByteArray( lenBlock, dataBlock.length, 0 );
		//Util.putIntToByteArray( lenBlock, type, 4 );
		lenBlock[4] = (byte) type;

		// ensure encryption payload is equal blocks of 16
		int bytesToPad = 0;
		int calcWriteLen = lenBlock.length + dataBlock.length;
		if (calcWriteLen % 16 != 0) {
			bytesToPad = 16 - (calcWriteLen % 16);
		}
		
		// TODO put random bytes here
		dataBlock	= Util.cloneByteArray( dataBlock, dataBlock.length + bytesToPad );

		//file.writeBytes(lenBlock);
		byte[] dataToWrite = Util.mergeBytes(lenBlock, dataBlock);
		
		for (int i = 0; i < (dataToWrite.length / 16); i++) {
			byte[] nextBlock = Util.getBytes(dataToWrite, i * 16, 16);
			file.writeEncryptedBytes(nextBlock);
		}
		
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
