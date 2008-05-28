/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Iterator;

import org.pwsafe.lib.I18nHelper;
import org.pwsafe.lib.Log;
import org.pwsafe.lib.Util;
import org.pwsafe.lib.crypto.BlowfishPws;
import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.PasswordSafeException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;

import net.sourceforge.blowfishj.SHA1;

/**
 * This is the base class for all variations of the PasswordSafe file format.
 * <p>
 * <tt>
 * <pre> +--------------+-----------+----------------------------------------------------+
 * |       Length | Name      | Description                                        |
 * +--------------+-----------+----------------------------------------------------+
 * |            8 | RandStuff | Random bytes                                       |
 * |           20 | RandHash  | Random hash                                        |
 * |           20 | Salt      | Salt                                               |
 * |            8 | IpThing   | Initial vector for decryption                      |
 * +--------------+-----------+----------------------------------------------------+</pre>
 * </tt>
 * </p><p>
 * The records follow immediately after the header.  Each record has the same
 * layout:
 * </p><p>
 * <tt>
 * <pre> +--------------+-----------+----------------------------------------------------+
 * |  BLOCK_LENGTH| RecLen    | Actual length of the data that follows (encrypted) |
 * |n*BLOCK_LENGTH| RecData   | The encrypted data.  The length is RecLen rounded  |
 * |              |           | up to be a multiple of BLOCK_LENGTH                |
 * +--------------+-----------+----------------------------------------------------+</pre>
 * </tt>
 * </p>
 */
public abstract class PwsFile
{
	private static final Log LOG = Log.getInstance(PwsFile.class.getPackage().getName());

	/**
	 * Length of RandStuff in bytes.
	 */
	public static final int	STUFF_LENGTH	= 8;

	/**
	 * Length of RandHash in bytes.
	 */
	public static final int	HASH_LENGTH		= 20;

	/**
	 * Block length - the minimum size of a data block.  All data written to the database is
	 * in blocks that are an integer multiple of <code>BLOCK_LENGTH</code> in size. 
	 */
	public static final int BLOCK_LENGTH	= 4; // used to be 8, but time field has only 4

	/**
	 * This provides a wrapper around the <code>Iterator</code> that is returned by the
	 * <code>iterator()</code> method on the Collections class used to store the PasswordSafe
	 * records.  It allows us to mark the file as modified when records are deleted file
	 * using the iterator's <code>remove()</code> method.
	 */
	private class FileIterator implements Iterator
	{
		private final Log LOG = Log.getInstance(FileIterator.class.getPackage().getName());

		private PwsFile		TheFile;
		private Iterator	TheIterator;

		/**
		 * Construct the <code>Iterator</code> linking it to the given PasswordSafe
		 * file.
		 * 
		 * @param file the file this iterator is linked to.
		 * @param iter the <code>Iterator</code> over the records.
		 */
		public FileIterator( PwsFile file, Iterator iter )
		{
			LOG.enterMethod( "PwsFile$FileIterator" );

			TheFile		= file;
			TheIterator	= iter;
			
			LOG.leaveMethod( "PwsFile$FileIterator" );
		}

		/**
		 * Returns <code>true</code> if the iteration has more elements. (In other words, returns
		 * <code>true</code> if next would return an element rather than throwing an exception.)
		 * 
		 * @return <code>true</code> if the iterator has more elements.
		 * 
		 * @see java.util.Iterator#hasNext()
		 */
		public final boolean hasNext()
		{
			return TheIterator.hasNext();
		}

		/**
		 * Returns the next PasswordSafe record in the iteration.  The object returned will
		 * be a subclass of {@link PwsRecord}
		 * 
		 * @return the next element in the iteration.
		 * 
		 * @see java.util.Iterator#next()
		 */
		public final Object next()
		{
			return TheIterator.next();
		}

		/**
		 * Removes the last record returned by {@link PwsFile.FileIterator#next()} from the PasswordSafe
		 * file and marks the file as modified.
		 * 
		 * @see java.util.Iterator#remove()
		 */
		public final void remove()
		{
			LOG.enterMethod( "PwsFile$FileIterator.remove" );

			TheIterator.remove();
			TheFile.setModified();

			LOG.leaveMethod( "PwsFile$FileIterator.remove" );
		}
	}

	/** 
	 * The fully qualified path to the file.
	 */
	protected String			FilePath		= null;

	/**
	 * The file name.
	 */
	protected String			FileName		= null;

	/**
	 * The passphrase for the file.
	 */
	protected String			Passphrase		= null;

	/**
	 * The stream used to read data from the file.  It is non-null only whilst data
	 * are being read from the file.
	 */
	protected InputStream		InStream		= null;

	/**
	 * The stream used to write data to the file.  It is non-null only whilst data are
	 * being written to the file. 
	 */
	protected OutputStream	OutStream		= null;

	/**
	 * The file's standard header.
	 */
	private PwsFileHeader	Header			= null;

	/**
	 * The Blowfish object being used to encrypt or decrypt data as it is written to or
	 * read from the file. 
	 */
	private BlowfishPws		Algorithm		= null;

	/**
	 * The records that are part of the file.
	 */
	protected ArrayList		RecordSet		= new ArrayList();

	/**
	 * Flag indicating whether (<code>true</code>) or not (<code>false</code>) the file
	 * has been modified in memory and not yet written back to the filesystem.
	 */
	protected boolean			Modified		= false;

	
	/**
	 * Constructs and initialises a new, empty PasswordSafe database in memory.
	 */
	protected PwsFile()
	{
		Header	= new PwsFileHeader();
	}

	/**
	 * Construct the PasswordSafe file by reading it from the file.
	 * 
	 * @param filename   the name of the database to open.
	 * @param passphrase the passphrase for the database.
	 * 
	 * @throws EndOfFileException
	 * @throws IOException
	 * @throws UnsupportedFileVersionException
	 */
	protected PwsFile( String filename, String passphrase )
	throws EndOfFileException, IOException, UnsupportedFileVersionException
	{
		LOG.enterMethod( "PwsFile.PwsFile( String )" );

		open( new File(filename), passphrase );

		LOG.leaveMethod( "PwsFile.PwsFile( String )" );
	}

	/**
	 * Adds a record to the file.
	 * 
	 * @param rec the record to be added.
	 * 
	 * @throws PasswordSafeException if the record has already been added to another file. 
	 */
	public void add( PwsRecord rec )
	throws PasswordSafeException
	{
		LOG.enterMethod( "PwsFile.add" );

		// TODO validate the record before adding it
		rec.setOwner( this );
		RecordSet.add( rec );
		setModified();

		LOG.leaveMethod( "PwsFile.add" );
	}

	/**
	 * Allocates a byte array at least <code>length</code> bytes in length and which is an integer multiple
	 * of <code>BLOCK_LENGTH</code>.
	 * 
	 * @param length the number of bytes the array must hold.
	 * 
	 * @return A byte array of the correct length.
	 */
	static final byte [] allocateBuffer( int length )
	{
		LOG.enterMethod( "PwsFile.allocateBuffer" );

		int	bLen;

		bLen	= calcBlockLength( length );

		LOG.leaveMethod( "PwsFile.allocateBuffer" );

		return new byte[ bLen ];
	}

	/**
	 * Calculates the next integer multiple of <code>BLOCK_LENGTH</code> &gt;= <code>length</code>.
	 * If <code>length</code> is zero, however, then <code>BLOCK_LENGTH</code> is returned as the
	 * calculated block length.
	 * 
	 * @param length the minimum block length 
	 * 
	 * @return <code>length</code> rounded up to the next multiple of <code>BLOCK_LENGTH</code>.
	 * 
	 * @throws IllegalArgumentException if length &lt; zero.
	 */
	static final int calcBlockLength( int length )
	{
		LOG.enterMethod( "PwsFile.calcBlockLength" );

		int result;

		if ( length < 0 )
		{
			LOG.error( I18nHelper.getInstance().formatMessage("E00004") );
			LOG.leaveMethod( "PwsFile.calcBlockLength" );
			throw new IllegalArgumentException( I18nHelper.getInstance().formatMessage("E00004") );
		}
		result = ( length == 0 ) ? BLOCK_LENGTH : ( (length + (BLOCK_LENGTH - 1)) / BLOCK_LENGTH ) * BLOCK_LENGTH;
		
		LOG.debug1( "Length = " + length + ", BlockLength = " + result );

		LOG.leaveMethod( "PwsFile.calcBlockLength" );

		return result;
	}

	/**
	 * Attempts to close the file.
	 * 
	 * @throws IOException If the attempt fails.
	 */
	void close()
	throws IOException
	{
		LOG.enterMethod( "PwsFile.close" );

		if ( InStream != null )
		{	
			InStream.close();
	
			InStream	= null;
			Algorithm	= null;
		}

		LOG.leaveMethod( "PwsFile.close" );
	}

	/**
	 * Returns the fully qualified path and filename.
	 * 
	 * @return The fully qualified filename, or null if no filename has been set.
	 */
	public String getFilename()
	{
		if (FilePath != null && FileName != null) {
			return FilePath + FileName;
		} else {
			return null;
		}
		
	}

	/**
	 * Returns the major version number for the file.
	 * 
	 * @return The major version number for the file.
	 */
	public abstract int getFileVersionMajor();

	/**
	 * Returns the file header.
	 * 
	 * @return The file header.
	 */
	PwsFileHeader getHeader()
	{
		return Header;
	}

	/**
	 * Returns the passphrase used to open the file.
	 * 
	 * @return The file's passphrase.
	 */
	public String getPassphrase()
	{
		return Passphrase;
	}

	/**
	 * Returns the number of records in the file.
	 * 
	 * @return The number of records in the file.
	 */
	public int getRecordCount()
	{
		LOG.enterMethod( "PwsFile.getRecordCount" );
		
		int size = RecordSet.size();

		LOG.leaveMethod( "PwsFile.getRecordCount" );

		return size;
	}

	/**
	 * Returns an iterator over the records.  Records may be deleted from the file by
	 * calling the <code>remove()</code> method on the iterator.
	 * 
	 * @return An <code>Iterator</code> over the records.
	 */
	public Iterator getRecords()
	{
		return new FileIterator( this, RecordSet.iterator() );
	}

	/**
	 * Returns an flag as to whether this file or any of its records have been modified.
	 * 
	 * @return <code>true</code> if the file has been modified, <code>false</code> if it hasn't.
	 */
	public boolean isModified()
	{
		return Modified;
	}

	/**
	 * Constructs and initialises the blowfish encryption routines ready to decrypt or
	 * encrypt data.
	 * 
	 * @param passphrase
	 * 
	 * @return A properly initialised {@link BlowfishPws} object.
	 */
	private BlowfishPws makeBlowfish( byte [] passphrase )
	{
		SHA1	sha1;
		byte	salt[];
		
		sha1 = new SHA1();
		salt = Header.getSalt();

		sha1.update( passphrase, 0, passphrase.length );
		sha1.update( salt, 0, salt.length );
		sha1.finalize();

		return new BlowfishPws( sha1.getDigest(), Header.getIpThing() );
	}

	/**
	 * Allocates a new, empty record unowned by any file.
	 * 
	 * @return A new empty record
	 */
	public abstract PwsRecord newRecord();

	/**
	 * Opens the database.
	 * 
	 * @param file       the <code>File</code> to read from
	 * @param passphrase the passphrase for the file.
	 * 
	 * @throws EndOfFileException
	 * @throws IOException
	 * @throws UnsupportedFileVersionException
	 */
	protected void open( File file, String passphrase )
	throws EndOfFileException, IOException, UnsupportedFileVersionException
	{
		LOG.enterMethod( "PwsFile.init" );

		setFilename( file );

		Passphrase		= passphrase;

		InStream		= new FileInputStream( file );
		Header			= new PwsFileHeader( this );
		Algorithm		= makeBlowfish( passphrase.getBytes() );

		readExtraHeader( this );

		LOG.leaveMethod( "PwsFile.init" );
	}

	/**
	 * Reads all records from the file.
	 * 
	 * @throws IOException  If an error occurs reading from the file.
	 * @throws UnsupportedFileVersionException  If the file is an unsupported version
	 */
	void readAll()
	throws IOException, UnsupportedFileVersionException
	{
		try
		{
			for ( ;; )
			{
				readRecord();
			}
		}
		catch ( EndOfFileException e )
		{
			// OK
		}
	}

	/**
	 * Allocates a block of <code>BLOCK_LENGTH</code> bytes then reads and decrypts this many
	 * bytes from the file.
	 * 
	 * @return A byte array containing the decrypted data.
	 * 
	 * @throws EndOfFileException If end of file occurs whilst reading the data.
	 * @throws IOException        If an error occurs whilst reading the file.
	 */
	protected byte [] readBlock()
	throws EndOfFileException, IOException
	{
		byte []	block;

		block = new byte[ getBlockSize() ];
		readDecryptedBytes( block );
		
		return block;
	}

	/**
	 * Reads raw (undecrypted) bytes from the file.  The method attepts to read
	 * <code>bytes.length</code> bytes from the file.
	 * 
	 * @param bytes the array to be filled from the file.
	 * 
	 * @throws EndOfFileException If end of file occurs whilst reading the data.
	 * @throws IOException        If an error occurs whilst reading the file.
	 */
	protected void readBytes( byte [] bytes )
	throws IOException, EndOfFileException
	{
		int count;

		count = InStream.read( bytes );

		if ( count == -1 )
		{
			LOG.debug1( "END OF FILE" );
			throw new EndOfFileException();
		}
		else if ( count < bytes.length )
		{
			LOG.info( I18nHelper.getInstance().formatMessage("I00003", new Object [] { new Integer(bytes.length), new Integer(count) } ) );
			throw new IOException( I18nHelper.getInstance().formatMessage("E00006") );
		}
		LOG.debug1( "Read " + count + " bytes" );
	}

	/**
	 * Reads bytes from the file and decryps them.  <code>buff</code> may be any length provided
	 * that is a multiple of <code>BLOCK_LENGTH</code> bytes in length.
	 * 
	 * @param buff the buffer to read the bytes into.
	 * 
	 * @throws EndOfFileException If end of file has been reached.
	 * @throws IOException If a read error occurs.
	 * @throws IllegalArgumentException If <code>buff.length</code> is not an integral multiple of <code>BLOCK_LENGTH</code>.
	 */
	protected void readDecryptedBytes( byte [] buff )
	throws EndOfFileException, IOException
	{
		if ( (buff.length == 0) || ((buff.length % BLOCK_LENGTH) != 0) )
		{
			throw new IllegalArgumentException( I18nHelper.getInstance().formatMessage("E00001") );
		}
		readBytes( buff );
		Algorithm.decrypt( buff );
	}

	/**
	 * Reads any additional header from the file.  Subclasses should override this a necessary
	 * as the default implementation does nothing.
	 * 
	 * @param file the {@link PwsFile} instance to read the header from.
	 * 
	 * @throws EndOfFileException              If end of file is reached.
	 * @throws IOException                     If an error occurs while reading the file.
	 * @throws UnsupportedFileVersionException If the file's version is unsupported.
	 */
	protected void readExtraHeader( PwsFile file )
	throws EndOfFileException, IOException, UnsupportedFileVersionException
	{
	}

	/**
	 * Reads a single record from the file.  The correct subclass of PwsRecord is
	 * returned depending on the version of the file.
	 * 
	 * @return The record read from the file.
	 * 
	 * @throws EndOfFileException When end-of-file is reached.
	 * @throws IOException
	 * @throws UnsupportedFileVersionException If this version of the file cannot be handled.
	 */
	protected PwsRecord readRecord()
	throws EndOfFileException, IOException, UnsupportedFileVersionException
	{
		PwsRecord	rec;

		rec = PwsRecord.read( this );

		if ( rec.isValid() )
		{	
			RecordSet.add( rec );
		}

		return rec;
	}

	/**
	 * Deletes the given record from the file.
	 * 
	 * @param rec the record to be deleted.
	 */
	void removeRecord( PwsRecord rec )
	{
		LOG.enterMethod( "PwsFile.removeRecord" );

		if ( RecordSet.contains(rec) )
		{
			LOG.debug1( "Record removed" );
			RecordSet.remove( rec );
			setModified();
		}
		else
		{
			LOG.debug1( "Record not removed - it is not part of this file" );
		}

		LOG.leaveMethod( "PwsFile.removeRecord" );
	}

	/**
	 * Writes this file back to the filesystem.  If successful the modified flag is also 
	 * reset on the file and all records.
	 * 
	 * @throws IOException if the attempt fails.
	 */
	public void save()
	throws IOException
	{
		PwsRecord	rec;
		File		tempFile;
		File		oldFile;
		File		bakFile;

		// For safety we'll write to a temporary file which will be renamed to the
		// real name if we manage to write it successfully.

		tempFile	= File.createTempFile( "pwsafe", null, new File(FilePath) );
		OutStream	= new FileOutputStream( tempFile );

		try
		{
			Header.save( this );

			// Can only be created once the V1 header's been written.

			Algorithm	= makeBlowfish( Passphrase.getBytes() );

			writeExtraHeader( this );

			for ( Iterator iter = getRecords(); iter.hasNext(); )
			{
				rec = (PwsRecord) iter.next();
	
				rec.saveRecord( this );
			}
	
			OutStream.close();
	
			oldFile		= new File( FilePath, FileName );
			bakFile		= new File( FilePath, FileName + "~" );

			if ( bakFile.exists() )
			{	
				if ( !bakFile.delete() )
				{
					LOG.error( I18nHelper.getInstance().formatMessage("E00012", new Object [] { bakFile.getCanonicalPath() } ) );
					// TODO Throw an exception here
					return;
				}
			}

			if ( oldFile.exists() )
			{
				if ( !oldFile.renameTo( bakFile ) )
				{
					LOG.error( I18nHelper.getInstance().formatMessage("E00011", new Object [] { tempFile.getCanonicalPath() } ) );
					// TODO Throw an exception here?
					return;
				}
				LOG.debug1( "Old file successfully renamed to " + bakFile.getCanonicalPath() );
			}

			if ( tempFile.renameTo( oldFile ) )
			{
				LOG.debug1( "Temp file successfully renamed to " + oldFile.getCanonicalPath() );

				for ( Iterator iter = getRecords(); iter.hasNext(); )
				{
					rec = (PwsRecord) iter.next();

					rec.resetModified();
				}
				Modified = false;
			}
			else
			{
				LOG.error( I18nHelper.getInstance().formatMessage("E00010", new Object [] { tempFile.getCanonicalPath() } ) );
				// TODO Throw an exception here?
				return;
			}
		}
		catch ( IOException e )
		{
			try
			{
				OutStream.close();
			}
			catch ( Exception e2 )
			{
				// do nothing we're going to throw the original exception
			}
			throw e;
		}
		finally
		{
			OutStream	= null;
			Algorithm	= null;
		}
	}

	/**
	 * Sets the name of the file that this file will be saved to.
	 * 
	 * @param newname the new name for the file.
	 * 
	 * @throws IOException
	 */
	public void setFilename( String newname )
	throws IOException
	{
		File file;

		file = new File( newname );
		setFilename( file );
		file = null;
	}

	/**
	 * Sets the name of the file that this file will be saved to.
	 * 
	 * @param file the <code>File</code> object representing the new file name.
	 * 
	 * @throws IOException
	 */
	protected void setFilename( File file )
	throws IOException
	{
		String	fname;
		File	file2;

		fname		= file.getCanonicalPath();
		file2		= new File( fname );
		FilePath	= file2.getParent();
		FileName	= file2.getName();

		if ( !FilePath.endsWith(File.separator) )
		{
			FilePath += File.separator;
		}

		LOG.debug2( "FileName = \"" + FilePath + FileName + "\"" );
	}

	/**
	 * Set the flag to indicate that the file has been modified.  There should not normally
	 * be any reason to call this method as it should be called indirectly when a record is
	 * added, changed or removed.
	 */
	protected void setModified()
	{
		Modified = true;
	}

	/**
	 * Sets the passphrase that will be used to encrypt the file when it is saved.
	 * 
	 * @param pass
	 */
	public void setPassphrase( String pass )
	{
		Passphrase	= pass;
	}

	/**
	 * Writes unencrypted bytes to the file.
	 * 
	 * @param buffer the data to be written.
	 * 
	 * @throws IOException
	 */
	void writeBytes( byte [] buffer )
	throws IOException
	{
		OutStream.write( buffer );
		LOG.debug1( "Wrote " + buffer.length + " bytes" );
	}

	/**
	 * Encrypts then writes the contents of <code>buff</code> to the file.
	 * 
	 * @param buff the data to be written.
	 * 
	 * @throws IOException
	 */
	protected void writeEncryptedBytes( byte [] buff )
	throws IOException
	{
		if ( (buff.length == 0) || ((buff.length % BLOCK_LENGTH) != 0) )
		{
			throw new IllegalArgumentException( I18nHelper.getInstance().formatMessage("E00001") );
		}
		
		byte [] temp = Util.cloneByteArray( buff );
		Algorithm.encrypt( temp );
		writeBytes( temp );
	}

	/**
	 * Writes any additional header.  This default implementation does nothing.  Subclasses 
	 * should override this as necessary. 
	 * 
	 * @param file
	 * 
	 * @throws IOException
	 */
	protected void writeExtraHeader( PwsFile file )
	throws IOException
	{
	}
	
	/** 
	 * Returns the size of blocks in this file type.
	 * 
	 * @return the size of blocks in this file type as an int
	 */
	abstract int getBlockSize();
}
