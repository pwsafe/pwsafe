/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;

import org.pwsafe.lib.I18nHelper;
import org.pwsafe.lib.Util;
import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.PasswordSafeException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;

/**
 * This abstract class implements the common features of PasswordSafe records.
 * </p><p>
 * When a record is added to a file it becomes "owned" by that file.  Records can only be owned by
 * one file at a time and an exception will be thrown if an attempt is made to add it to another
 * file.  If a record needs to be added to more than one file, call the {link clone} method on
 * the original record and add the clone to the other file.  For example:
 * </p><p>
 * <tt>
 * <pre> PwsFile file1;
 * PwsFile file2;
 * PwsFile file3;
 * RwsRecord rec;
 *     :
 * rec = file1.readRecord();
 * file2.add( (PwsRecord) rec.clone() );
 * file3.add( (PwsRecord) rec.clone() );</pre>
 * </tt>
 * </p>
 * 
 * @author Kevin Preece
 */
public abstract class PwsRecord implements Comparable
{
	/**
	 * The default character set used for <code>byte[]</code> to <code>String</code> conversions.
	 */
	public static final String	DEFAULT_CHARSET	= "ISO-8859-1";

	private boolean		Modified		= false;
	private PwsFile		OwningFile		= null;
	private boolean		IsLoaded		= false;
	protected Map		Attributes		= new TreeMap();
	private final Object ValidTypes[];

	protected boolean ignoreFieldTypes = false;
	
	
	/**
	 * A holder class for all the data about a single field.  It holds the field's length,
	 * data and, for those formats that use it, the field's type.
	 */
	protected class Item
	{
		protected byte []	RawData;
		protected byte []	Data;
		protected int		Length;
		protected int		Type;

		/** No args constructor helps subclassing. */
		protected Item() {
			
		}
		/**
		 * Reads a single item of data from the file.
		 * 
		 * @param file the file the data should be read from.
		 * 
		 * @throws EndOfFileException
		 * @throws IOException
		 */
		public Item( PwsFile file )
		throws EndOfFileException, IOException
		{
			RawData = file.readBlock();
			Length	= Util.getIntFromByteArray( RawData, 0 );
			//Type	= Util.getIntFromByteArray( RawData, 4 );
			Type = RawData[4] & 0x000000ff; // rest of header is now random data
			Data	= PwsFile.allocateBuffer( Length );
			
			file.readDecryptedBytes( Data );
		}
		
		/**
		 * Gets this items data as an array of bytes.
		 * 
		 * @return This items data as an array of bytes.
		 */
		public byte [] getByteData()
		{
			if ( Length != Data.length )
			{
				return Util.cloneByteArray( Data, Length );
			}
			return Data;
		}

		/**
		 * Gets this items data as a <code>String</code>.  The byte array is converted
		 * to a <code>String</code> using <code>DEFAULT_CHARSET</code> as the encoding.
		 * 
		 * @return The item data as a <code>String</code>.
		 */
		public String getData()
		{
			try
			{
				// Use ISO-8859-1 because we may have some values outside the valid ASCII
				// range.
				//
				// TODO This needs to be reviewed if the format ever changes to unicode

				return new String( Data, 0, Length, DEFAULT_CHARSET );
			}
			catch ( UnsupportedEncodingException e )
			{
				// Should never get here since all Java implementations must support the above charset.
				return new String( Data, 0, Length );
			}
		}

		/**
		 * Returns this items type.  For V1 files this will always be zero.
		 * 
		 * @return This items type.
		 */
		public int getType()
		{
			return Type;
		}

		/**
		 * Returns details about this field as a <code>String</code> suitable for dubugging.
		 * 
		 * @return A <code>String</code> representation of this object.
		 */
		public String toString()
		{
			StringBuffer	sb;

			sb = new StringBuffer();
			sb.append( "{ type=" );
			sb.append( Type );
			sb.append( ", data=\"" );
			sb.append( getData() );
			sb.append( "\" }" );

			return sb.toString();
		}
	}

	/**
	 * Simple constructor.  Used when creating a new record to add to a file.
	 * 
	 * @param validTypes an array of valid field types.
	 */
	PwsRecord( Object [] validTypes )
	{
		super();

		ValidTypes	= validTypes;
	}

	/**
	 * This constructor is called when a record is to be read from the database.
	 * 
	 * @param owner      the file that data is to be read from and which "owns" this record. 
	 * @param validTypes an array of valid field types.
	 * 
	 * @throws EndOfFileException
	 * @throws IOException
	 */
	PwsRecord( PwsFile owner, Object [] validTypes )
	throws EndOfFileException, IOException
	{
		super();

		OwningFile	= owner;
		ValidTypes	= validTypes;

		loadRecord( owner );

		IsLoaded	= true;
	}
	
	/**
	 * Special constructor for use when ignoring field types.
	 * 
	 * @param owner      the file that data is to be read from and which "owns" this record. 
	 * @param validTypes an array of valid field types.
	 * @param ignoreFieldTypes true if all fields types should be ignored, false otherwise
	 * 
	 * @throws EndOfFileException
	 * @throws IOException
	 */
	public PwsRecord(PwsFile owner, Object[] validTypes, boolean ignoreFieldTypes) throws EndOfFileException, IOException {
		super();
		
		OwningFile	= owner;
		ValidTypes	= validTypes;
		this.ignoreFieldTypes = ignoreFieldTypes;

		loadRecord( owner );

		IsLoaded	= true;
		
		
	}


	/**
	 * 
	 * @param base
	 */
	PwsRecord( PwsRecord base )
	{
		IsLoaded	= true;
		ValidTypes	= base.ValidTypes;
		
		for ( Iterator iter = getFields(); iter.hasNext(); )
		{
			Integer	key = (Integer) iter.next();
			Attributes.put( key, getField(key) );
		}
	}

	//*************************************************************************
	//* Abstract methods
	//*************************************************************************


	/**
	 * Returns a clone of this record that is a deep copy of it.
	 * 
	 * @return A clone of this record.
	 * 
	 * @throws CloneNotSupportedException
	 */
	public abstract Object clone() throws CloneNotSupportedException;

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
	 * @see java.lang.Comparable#compareTo(java.lang.Object)
	 */
	public abstract int compareTo( Object other );

	/**
	 * Compares this record to another returning <code>true</code> if they're equal
	 * and <code>false</code> if they're unequal.
	 * 
	 * @param other the record this one is compared to.
	 * 
	 * @return <code>true</code> if the records are equal, <code>false</code> if 
	 *         they're unequal.
	 */
	public abstract boolean equals( Object other );

	/**
	 * Validates the record, returning <code>true</code> if it's valid or <code>false</code>
	 * if unequal.
	 * 
	 * @return <code>true</code> if it's valid or <code>false</code> if unequal.
	 */
	protected abstract boolean isValid();
	
	/**
	 * Loads this record from <code>file</code>.
	 * 
	 * @param file the file to load the record from.
	 * 
	 * @throws EndOfFileException
	 * @throws IOException
	 */
	protected abstract void loadRecord( PwsFile file ) throws EndOfFileException, IOException;

	/**
	 * Saves this record to <code>file</code>.
	 * 
	 * @param file the file to save the record to.
	 * 
	 * @throws IOException
	 */
	protected abstract void saveRecord( PwsFile file ) throws IOException;

	//*************************************************************************
	//* Class methods
	//*************************************************************************

	/**
	 * Removes this record from the owning file.
	 */
	public void delete()
	{
		if ( OwningFile != null )
		{
			OwningFile.removeRecord( this );
		}
	}

	/**
	 * Gets the value of a field.  See the subclass documentation for valid values for
	 * <code>type</code>.
	 * 
	 * @param type the field to get.
	 * 
	 * @return The value of the field.
	 */
	public PwsField getField( int type )
	{
		return getField( new Integer(type) );
	}

	/**
	 * Gets the value of a field.  See the subclass documentation for valid values for
	 * <code>type</code>.
	 * 
	 * @param type the field to get.
	 * 
	 * @return The value of the field.
	 */
	public PwsField getField( Integer type )
	{
		return (PwsField) Attributes.get( type );
	}

	/**
	 * Returns an <code>Iterator</code> that returns the field types (but not the values) that
	 * have been stored.  Use one of the <code>getField</code> methods to get the value.  The
	 * iterators <code>next()</code> method returns an <code>Integer</code>
	 * 
	 * @return An <code>Iterator</code> over the stored field codes.
	 */
	public Iterator getFields()
	{
		return Attributes.keySet().iterator();
	}

	/**
	 * Returns <code>true</code> if the record has been modified or <code>false</code> if not.
	 * 
	 * @return <code>true</code> if the record has been modified or <code>false</code> if not.
	 */
	public boolean isModified()
	{
		return Modified;
	}

	/**
	 * Read a record from the given file.
	 * 
	 * @param file the file to read the record from.
	 * 
	 * @return The record that was read.
	 * 
	 * @throws EndOfFileException
	 * @throws IOException
	 * @throws UnsupportedFileVersionException
	 */
	public static PwsRecord read( PwsFile file )
	throws EndOfFileException, IOException, UnsupportedFileVersionException
	{
		switch ( file.getFileVersionMajor() )
		{
			case PwsFileV1.VERSION :
				return new PwsRecordV1( file );
	
			case PwsFileV2.VERSION :
				return new PwsRecordV2( file );
				
			case PwsFileV3.VERSION :
				return new PwsRecordV3( file );
		}
		throw new UnsupportedFileVersionException();
	}
	
	/**
	 * Resets the modified flag.
	 */
	void resetModified()
	{
		Modified = false;
	}

	/**
	 * Sets a field on this record from <code>item</code>.
	 * 
	 * @param item the <code>Item</code> containg the field's data.
	 */
	protected void setField( Item item )
	{
		setField( new PwsStringField( item.getType(), item.getData() ) );
	}

	/**
	 * Sets a field on this record from <code>value</code>.
	 * 
	 * @param value the field to set
	 * 
	 * @throws IllegalArgumentException if value is not the correct type for the file.
	 */
	public void setField( PwsField value )
	{
		int type;

		type = value.getType();

		for ( int ii = 0; ii < ValidTypes.length; ++ii )
		{
			int		vType;

			vType	= ((Integer) ((Object[]) ValidTypes[ii])[0]).intValue();

			if ( vType == type )
			{
				Class	cl = value.getClass();

				if ( cl == (((Object[]) ValidTypes[ii])[2]) )
				{	
					Attributes.put( new Integer(type), value );
					setModified();
					return;
				}
			}
		}
		throw new IllegalArgumentException( I18nHelper.getInstance().formatMessage("E00003", new Object [] { new Integer(type) } ) );
	}

	/**
	 * Sets the modified flag on this record, and also on the file this record belongs to.
	 */
	public void setModified()
	{
		if ( IsLoaded )
		{	
			Modified = true;
			OwningFile.setModified();
		}
	}

	/**
	 * Sets <code>file</code> as the owner of this record, i.e., the file it was read from 
	 * or will be written to.
	 * 
	 * @param owner the <code>PwsFile</code> that owns this record.
	 * 
	 * @throws PasswordSafeException if this record is already owned by a file.
	 */
	void setOwner( PwsFile owner )
	throws PasswordSafeException
	{
		if ( OwningFile != null )
		{
			throw new PasswordSafeException( I18nHelper.getInstance().formatMessage("E00005") );
		}
		OwningFile = owner;
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

		lenBlock	= new byte[ PwsFile.calcBlockLength(8) ];
		dataBlock	= field.getBytes();
		
		Util.putIntToByteArray( lenBlock, dataBlock.length, 0 );
		Util.putIntToByteArray( lenBlock, type, 4 );
		//TODO put random bytes here

		dataBlock	= Util.cloneByteArray( dataBlock, PwsFile.calcBlockLength(dataBlock.length) );

		file.writeEncryptedBytes( lenBlock );
		file.writeEncryptedBytes( dataBlock );
	}

	/**
	 * Writes a single field to the file.
	 * 
	 * @param file  the file to write the field to.
	 * @param field the field to be written.
	 * 
	 * @throws IOException
	 */
	protected void writeField( PwsFile file, PwsField field )
	throws IOException
	{
		writeField( file, field, field.getType() );
	}
}
