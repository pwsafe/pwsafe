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

import BlowfishJ.SHA1;

/**
 * This is the base class for all variations of the PasswordSafe file format.
 * <p />
 * <tt>
 * <pre>+--------------+-----------+----------------------------------------------------+
 * |       Length | Name      | Description                                        |
 * +--------------+-----------+----------------------------------------------------+
 * |            8 | RandStuff | Random bytes                                       |
 * |           20 | RandHash  | Random hash                                        |
 * |           20 | Salt      | Salt                                               |
 * |            8 | IpThing   | Initial vector for decryption                      |
 * +--------------+-----------+----------------------------------------------------+</pre>
 * </tt>
 * <p />
 * The records follow immediately after the header.  Each record has the same
 * layout:
 * <tt>
 * <pre>+--------------+-----------+----------------------------------------------------+
 * |  BLOCK_LENGTH| RecLen    | Actual length of the data that follows (encrypted) |
 * |n*BLOCK_LENGTH| RecData   | The encrypted data.  The length is RecLen rounded  |
 * |              |           | up to be a multiple of BLOCK_LENGTH                |
 * +--------------+-----------+----------------------------------------------------+</pre>
 * </tt><p />
 */
public abstract class PwsFile
{
	private static final Log LOG = Log.getInstance(PwsFile.class.getPackage().getName());

	public static final int	STUFF_LENGTH	= 8;
	public static final int	HASH_LENGTH		= 20;
	public static final int BLOCK_LENGTH	= 8;

	/**
	 * This provides a wrapper around the <code>Iterator</code> that is returned by the
	 * <code>iterator()</code> method on the Collections class used to store the PasswordSafe
	 * records.  It allows us to mark the file as modified when records are deleted file
	 * using the iterator's <code>remove()</code> method.
	 */
	private class FileIterator implements Iterator
	{
		private PwsFile		TheFile;
		private Iterator	TheIterator;

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
		public boolean hasNext()
		{
			return TheIterator.hasNext();
		}

		/**
		 * Returns the next element in the iteration.
		 * 
		 * @return the next element in the iteration.
		 * 
		 * @see java.util.Iterator#next()
		 */
		public Object next()
		{
			return TheIterator.next();
		}

		/* (non-Javadoc)
		 * @see java.util.Iterator#remove()
		 */
		public void remove()
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
	private String			FilePath		= null;

	/**
	 * The file name.
	 */
	private String			FileName		= null;

	/**
	 * The password for the file.
	 */
	private String			Password		= null;

	/**
	 * The stream used to read data from the file.  It is non-null only whilst data
	 * are being read from the file.
	 */
	private InputStream		InStream		= null;

	/**
	 * The stream used to write data to the file.  It is non-null only whilst data are
	 * being written to the file. 
	 */
	private OutputStream	OutStream		= null;

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
	private ArrayList		RecordSet		= null;

	/**
	 * Flag indicating whether (<code>true</code>) or not (<code>false</code>) the file
	 * has been modified in memory and not yet written back to the filesystem.
	 */
	private boolean			Modified		= false;

	/**
	 * Construct the PasswordSafe file by reading it from the file.
	 * <p />
	 * @param filename 
	 * @param password
	 * <p />
	 * @throws EndOfFileException
	 * @throws IOException
	 */
	public PwsFile( String filename, String password )
	throws EndOfFileException, IOException
	{
		LOG.enterMethod( "PwsFile.PwsFile( String )" );

		open( new File(filename), password );

		LOG.leaveMethod( "PwsFile.PwsFile( String )" );
	}

	/**
	 * Adds a record to the file.
	 * <p />
	 * @param rec the record to be added.
	 * <p />
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
	 * <p />
	 * @param length the number of bytes the array must hold.
	 * <p />
	 * @return A byte array of the correct length.
	 */
	static byte [] allocateBuffer( int length )
	{
		LOG.enterMethod( "PwsFile.allocateBuffer" );

		int	bLen;

		bLen	= calcBlockLength( length );

		LOG.leaveMethod( "PwsFile.allocateBuffer" );

		return new byte[ bLen ];
	}

	/**
	 * Calculates the next integer multiple of <code>BLOCK_LENGTH</code> &gt;= <code>length</code>.
	 * <p />
	 * @param length 
	 * <p />
	 * @return <code>length</code> rounded up to the next multiple of <code>BLOCK_LENGTH</code>. 
	 */
	static int calcBlockLength( int length )
	{
		LOG.enterMethod( "PwsFile.calcBlockLength" );

		int result;

		if ( length < 0 )
		{
			LOG.error( I18nHelper.formatMessage("E00004") );
			LOG.leaveMethod( "PwsFile.calcBlockLength" );
			throw new IllegalArgumentException( I18nHelper.formatMessage("E00004") );
		}
		result = ( length == 0 ) ? BLOCK_LENGTH : ( (length + (BLOCK_LENGTH - 1)) / BLOCK_LENGTH ) * BLOCK_LENGTH;
		
		LOG.debug1( "Length = " + length + ", BlockLength = " + result );

		LOG.leaveMethod( "PwsFile.calcBlockLength" );

		return result;
	}

	/**
	 * Closes the file.
	 * <p />
	 * @throws IOException
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
	 * Returns the major version number for the file.
	 * <p />
	 * @return
	 */
	public abstract int getFileVersionMajor();

	/**
	 * Returns the file header.
	 * <p />
	 * @return The file header.
	 */
	PwsFileHeader getHeader()
	{
		return Header;
	}

	/**
	 * Returns the password used to open the file.
	 * <p />
	 * @return The file's password.
	 */
	String getPassword()
	{
		return Password;
	}

	/**
	 * Returns the number of records in the file.
	 * <p />
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
	 * <p />
	 * @return
	 */
	public Iterator getRecords()
	{
		return new FileIterator( this, RecordSet.iterator() );
	}

	/**
	 * Returns an flag as to whether this file or any of its records have been modified.
	 * <p />
	 * @return
	 */
	public boolean isModified()
	{
		return Modified;
	}

	/**
	 * Constructs and initialises the blowfish encryption routines ready to decrypt or
	 * encrypt data.
	 * <p />
	 * @param password
	 * <p />
	 * @return A properly initialised {@link BlowfishPws} object.
	 */
	private BlowfishPws makeBlowfish( byte [] password )
	{
		SHA1	sha1;
		byte	salt[];
		
		sha1 = new SHA1();
		salt = Header.getSalt();

		sha1.update( password, 0, password.length );
		sha1.update( salt, 0, salt.length );
		sha1.finalize();

		return new BlowfishPws( sha1.getDigest(), Header.getIpThing() );
	}

	/**
	 * Opens the file.
	 * <p />
	 * @param file
	 * @param password
	 * <p />
	 * @throws EndOfFileException
	 * @throws IOException
	 */
	private void open( File file, String password )
	throws EndOfFileException, IOException
	{
		LOG.enterMethod( "PwsFile.init" );

		setFilename( file );

		Password		= password;

		InStream		= new FileInputStream( file );
		Header			= new PwsFileHeader( this );
		Algorithm		= makeBlowfish( password.getBytes() );
		RecordSet		= new ArrayList();

		readExtraHeader( this );

		LOG.leaveMethod( "PwsFile.init" );
	}

	/**
	 * Reads all records from the file.
	 * <p />
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
	 * <p />
	 * @throws IOException              if an error occurs whilst reading the file.
	 */
	protected byte [] readBlock()
	throws EndOfFileException, IOException
	{
		byte []	block;

		block = new byte[ BLOCK_LENGTH ];
		readDecryptedBytes( block );
		
		return block;
	}

	/**
	 * Reads raw (undecrypted) bytes from the file.  The method attepts to read
	 * <code>bytes.length</code> bytes from the file.
	 * <p />
	 * @param bytes the array to be filled from the file.
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
			LOG.info( I18nHelper.formatMessage("I00003", new Object [] { new Integer(bytes.length), new Integer(count) } ) );
			throw new IOException( I18nHelper.formatMessage("E00006") );
		}
		LOG.debug1( "Read " + count + " bytes" );
	}

	/**
	 * Reads bytes from the file and decryps them.  <code>buff</code> may be any length provided
	 * that is a multiple of <code>BLOCK_LENGTH</code> bytes in length.
	 * <p />
	 * @param buff the buffer to read the bytes into.
	 * <p />
	 * @throws EndOfFileException If end of file has been reached.
	 * @throws IOException If a read error occurs.
	 * @throws IllegalArgumentException If <code>buff.length</code> is not an integral multiple of <code>BLOCK_LENGTH</code>.
	 */
	protected void readDecryptedBytes( byte [] buff )
	throws EndOfFileException, IOException
	{
		if ( (buff.length == 0) || ((buff.length % BLOCK_LENGTH) != 0) )
		{
			throw new IllegalArgumentException( I18nHelper.formatMessage("E00001") );
		}
		readBytes( buff );
		Algorithm.decrypt( buff );
	}

	/**
	 * Reads any additional header from the file.  Subclasses should override this a necessary
	 * as the default implementation does nothing.
	 * <p />
	 * @param file the {@link PwsFile} instance to read the header from.
	 * <p />
	 * @throws EndOfFileException If end of file is reached.
	 * @throws IOException        If an error occurs while reading the file.
	 */
	protected void readExtraHeader( PwsFile file )
	throws EndOfFileException, IOException
	{
	}

	/**
	 * Reads a single record from the file.  The correct subclass of PwsRecord is
	 * returned depending on the version of the file.
	 * <p />
	 * @return The record read from the file.
	 * <p />
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
	 * <p />
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
	 * Writes this file back to the filesystem.  The modified flag is also reset on the file 
	 * and all records.
	 */
	public void save()
	throws IOException
	{
		PwsRecord	rec;
		File		tempFile;
		File		oldFile;
		File		bakFile;

		// For safety we'll write to a temporary file which will be renamed to the
		// real name if we manage to wite it successfully.

		tempFile	= File.createTempFile( "pwsafe", null, new File(FilePath) );
		OutStream	= new FileOutputStream( tempFile );

		try
		{
			Header.save( this );

			// Can only be created once the V1 header's been written.

			Algorithm	= makeBlowfish( Password.getBytes() );

			writeExtraHeader( this );

			for ( Iterator iter = getRecords(); iter.hasNext(); )
			{
				rec = (PwsRecord) iter.next();
	
				rec.saveRecord( this );
				rec.resetModified();
			}
	
			OutStream.close();
	
			Modified	= false;
			oldFile		= new File( FilePath, FileName );
			bakFile		= new File( FilePath, FileName + "~" );

			// TODO improve error handling here
			// - what if we write the file OK but can't rename the original, e.g. because it's in use.

			bakFile.delete();
			if ( oldFile.exists() )
			{
				if ( !oldFile.renameTo( bakFile ) )
				{
					LOG.error( I18nHelper.formatMessage("E00011", new Object [] { tempFile.getCanonicalPath() } ) );
					// TODO Throw an exception here?
					return;
				}
				LOG.debug1( "Old file successfully renamed to " + bakFile.getCanonicalPath() );
			}
			if ( tempFile.renameTo( oldFile ) )
			{
				LOG.debug1( "Temp file successfully renamed to " + oldFile.getCanonicalPath() );
			}
			else
			{
				LOG.error( I18nHelper.formatMessage("E00010", new Object [] { tempFile.getCanonicalPath() } ) );
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
	 * <p />
	 * @param newname the new name for the file.
	 * <p />
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
	 * <p />
	 * @param file the <code>File</code> object representing the new file name.
	 * <p />
	 * @throws IOException
	 */
	private void setFilename( File file )
	throws IOException
	{
		String	fname;
		int		pos;
		File	file2;

		fname		= file.getCanonicalPath();
		file2		= new File( fname );
		FilePath	= file2.getParent();
		FileName	= file2.getName();

		if ( !FilePath.endsWith("\\") )
		{
			FilePath += "\\";
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
	 * Writes unencrypted bytes to the file.
	 * <p />
	 * @param buffer the data to be written.
	 * <p />
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
	 * <p />
	 * @param buff the data to be written.
	 * <p />
	 * @throws IOException
	 */
	protected void writeEncryptedBytes( byte [] buff )
	throws IOException
	{
		if ( (buff.length == 0) || ((buff.length % BLOCK_LENGTH) != 0) )
		{
			throw new IllegalArgumentException( I18nHelper.formatMessage("E00001") );
		}
		
		byte [] temp = Util.cloneByteArray( buff );
		Algorithm.encrypt( temp );
		writeBytes( temp );
	}

	/**
	 * Writes any additional header.  This default implementation does nothing.  Subclasses 
	 * should override this as necessary. 
	 * <p />
	 * @param file
	 */
	protected void writeExtraHeader( PwsFile file )
	throws IOException
	{
	}
}
