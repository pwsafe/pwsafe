package org.pwsafe.lib.file;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Iterator;

import org.pwsafe.lib.I18nHelper;
import org.pwsafe.lib.Util;
import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.PasswordSafeException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;

/**
 * This abstract class implements the common features of PasswordSafe records.
 * <p />
 * When a record is added to a file it becoms "owned" by that file.  Records can only be owned by
 * one file at a time and an exception will be thrown if an attempt is made to add it to another
 * file.  If a record needs to be added to more than one file, call the {link clone} method on
 * the original record and add the clone to the other file.  For example:
 * <p />
 * <tt>
 * <pre>PwsFile file1;
 * PwsFile file2;
 * PwsFile file3;
 * RwsRecord rec;
 *     :
 * rec = file1.readRecord();
 * file2.add( (PwsRecord) rec.clone() );
 * file3.add( (PwsRecord) rec.clone() );</pre>
 * </tt>
 */
public abstract class PwsRecord implements Comparable
{
	public static final String	DEFAULT_CHARSET	= "ISO-8859-1";

	private boolean		Modified		= false;
	private PwsFile		OwningFile		= null;
	private boolean		IsLoaded		= false;
	private HashMap		Attributes		= new HashMap();
	private int			ValidTypes []	= null;

	private class Item
	{
		protected byte []	RawData;
		protected byte []	Data;
		protected int		Length;
		protected int		Type;

		/**
		 * Reads a single item of data from the file.
		 */
		public Item()
		{
		}

		/**
		 * Gets this items data as a string.
		 * 
		 * @return
		 */
		public byte [] getByteData()
		{
			return Data;
		}

		/**
		 * Gets this items data as a string.
		 * 
		 * @return
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

	class ReadItem extends Item
	{
		/**
		 * Reads a single item of data from the file.
		 * 
		 * @param file
		 * 
		 * @throws IOException
		 */
		public ReadItem( PwsFile file )
		throws EndOfFileException, IOException
		{
			RawData = file.readBlock();
			Length	= Util.getIntFromByteArray( RawData, 0 );
			Type	= Util.getIntFromByteArray( RawData, 4 );
			Data	= PwsFile.allocateBuffer( Length );
			
			file.readDecryptedBytes( Data );
		}
	}

	/**
	 * No-argument constructor.  Used when creating a new record to add to a file.
	 *
	 */
	public PwsRecord( int [] validTypes )
	{
		super();

		ValidTypes	= validTypes;
	}

	/**
	 * 
	 */
	PwsRecord( PwsFile owner, int [] validTypes )
	throws EndOfFileException, IOException
	{
		super();

		OwningFile	= owner;
		ValidTypes	= validTypes;

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

	public abstract Object clone() throws CloneNotSupportedException;

	public abstract int compareTo( Object ob );

	public abstract boolean equals( Object arg0 );

	protected abstract boolean isValid();
	
	/**
	 * Loads this record from <code>file</code>.
	 * 
	 * @param file the file to load the record from.
	 */
	protected abstract void loadRecord( PwsFile file ) throws EndOfFileException, IOException;

	/**
	 * Saves this record to <code>file</code>.
	 * 
	 * @param file the file to save the record to.
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
		return (PwsField) Attributes.get( new Integer(type) );
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
		return getField( type.intValue() );
	}

	/**
	 * Returns an <code>Iterator</code> that returns the field types (but not the values) that
	 * have been stored.  Use one of the <code>getField</code> methods to get the value.  The
	 * iterators <code>next()</code> method returns an <code>Integer</code>
	 * 
	 * @return An iterator over the stored field codes.
	 */
	public Iterator getFields()
	{
		return Attributes.keySet().iterator();
	}

	/**
	 * Returns <code>true</code> if the record has been modified or <code>false</code> if not.
	 * 
	 * @return
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
	 * 
	 * @param item
	 */
	public void setField( ReadItem item )
	{
		setField( new PwsStringField( item.getType(), item.getData() ) );
	}

	/**
	 * 
	 * @param type
	 * @param value
	 */
	public void setField( PwsField value )
	{
		int type;

		type = value.getType();

		for ( int ii = 0; ii < ValidTypes.length; ++ii )
		{
			if ( ValidTypes[ii] == type )
			{
				Attributes.put( new Integer(type), value );
				setModified();
				return;
			}
		}
		throw new IllegalArgumentException( I18nHelper.formatMessage("E00003", new Object [] { new Integer(type) } ) );
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
	 * 
	 * @param owner
	 * @throws PasswordSafeException
	 */
	void setOwner( PwsFile owner )
	throws PasswordSafeException
	{
		if ( OwningFile != null )
		{
			throw new PasswordSafeException( I18nHelper.formatMessage("E00005") );
		}
		OwningFile = owner;
	}

	protected void writeField( PwsFile file, PwsField field, int type )
	throws IOException
	{
		byte	lenBlock[];
		byte	dataBlock[];

		lenBlock	= new byte[ PwsFile.calcBlockLength(8) ];
		dataBlock	= field.getBytes();
		
		Util.putIntToByteArray( lenBlock, dataBlock.length, 0 );
		Util.putIntToByteArray( lenBlock, type, 4 );

		dataBlock	= Util.cloneByteArray( dataBlock, PwsFile.calcBlockLength(dataBlock.length) );

		file.writeEncryptedBytes( lenBlock );
		file.writeEncryptedBytes( dataBlock );
	}

	protected void writeField( PwsFile file, PwsField field )
	throws IOException
	{
		writeField( file, field, field.getType() );
	}
}
