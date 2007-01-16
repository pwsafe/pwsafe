/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.passwordsafej;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.HeadlessException;
import java.awt.Rectangle;
import java.awt.Window;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextField;

import org.pwsafe.lib.Log;
import org.pwsafe.lib.exception.EndOfFileException;
import org.pwsafe.lib.exception.InvalidPassphraseException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;
import org.pwsafe.lib.file.PwsFile;
import org.pwsafe.lib.file.PwsFileFactory;

/**
 * 
 * @author Kevin Preece
 */
public class PasswordSafeJ extends JFrame
{
	/**
	 * Log4j logger
	 */
	private static final Log	LOG	= Log.getInstance( PasswordSafeJ.class.getPackage().getName() );

	private MenuHandler		menuHandler;
	private TreeHandler		treeHandler;
	private ToolbarHandler	toolbarHandler;
	private JTextField		statusBar;
	private PwsFile			theFile;

	/**
	 * 
	 * @param args
	 */
	public static void main( String [] args )
	{
		LOG.enterMethod( "main" );

		JFrame.setDefaultLookAndFeelDecorated( true );
		JDialog.setDefaultLookAndFeelDecorated( true );

		new PasswordSafeJ();

		LOG.leaveMethod( "main" );
	}

	/**
	 * @throws java.awt.HeadlessException
	 */
	public PasswordSafeJ() throws HeadlessException
	{
		super( I18nHelper.getInstance().formatMessage("ui.title") );

		LOG.enterMethod( "PasswordSafeJ" );

		JScrollPane	sp;

		menuHandler		= new MenuHandler( this );
		treeHandler 	= new TreeHandler( this );
		sp				= treeHandler.getScrollPane();
		toolbarHandler	= new ToolbarHandler( this );
		statusBar		= new JTextField( "http://passwordsafe.sourceforge.net" );

		statusBar.setEditable( false );

//		setIconImage( new ImageIcon(imgURL).getImage() );
		setJMenuBar( menuHandler.getMenuBar() );

		sp.setPreferredSize( new Dimension(400, 400) );

		getContentPane().add( toolbarHandler.getToolBar(), BorderLayout.NORTH );
		getContentPane().add( sp, BorderLayout.CENTER );
		getContentPane().add( statusBar, BorderLayout.SOUTH );

		addWindowEvents();

		pack();
		setLocation( Util.centreWithin( new Rectangle(getToolkit().getScreenSize()), this.getSize()) );
		setVisible( true );

		LOG.leaveMethod( "PasswordSafeJ" );
	}

	public void addEntry()
	{
		AddDialog	addDialog;

		addDialog	= new AddDialog( this );
	}

	private void addWindowEvents()
	{
		LOG.enterMethod( "addWindowEvents" );

		this.addWindowListener( new WindowAdapter()
		{
			public void windowClosing(WindowEvent arg0)
			{
				LOG.debug1( "Window close button pressed" );
				exit();
			}
		} );

		LOG.leaveMethod( "addWindowEvents" );
	}

	void exit()
	{
		exit( this );
	}

	void exit( Window arg0 )
	{
		arg0.setVisible( false );
		arg0.dispose();
		System.exit( 0 );
	}

	void leafSelected( boolean isLeafNode )
	{
		menuHandler.leafSelected( isLeafNode );
		toolbarHandler.leafSelected( isLeafNode );
	}

	void openFile()
	{
		LOG.enterMethod( "openFile" );

		JFileChooser	fc;

		fc	= new FileOpenDialogue();

		if ( fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION )
		{
			try
			{
				File			file;
				TreeHandler		oldTreeHandler;
				JScrollPane		sp;
				PasswordDialog	pwd;

				pwd				= new PasswordDialog( this );

				if ( pwd.isOkPressed() )
				{
					file			= fc.getSelectedFile();
					theFile			= PwsFileFactory.loadFile( file.getCanonicalPath(), pwd.getEnteredPassword() );
					oldTreeHandler	= treeHandler;
					treeHandler		= new TreeHandler( this, theFile );
					sp				= treeHandler.getScrollPane();

					sp.setPreferredSize( oldTreeHandler.getScrollPane().getSize() );

					getContentPane().remove( oldTreeHandler.getScrollPane() );
					getContentPane().add( sp, BorderLayout.CENTER );

					pack();

					menuHandler.fileOpened();
					toolbarHandler.fileOpened();
				}
			}
			catch ( InvalidPassphraseException e )
			{
				LOG.debug1( "InvalidPasswordException" );
				// TODO show a dialog and try again
			}
			catch ( EndOfFileException e )
			{
				LOG.debug1( "EndOfFileException" );
				// TODO handle this exception
			}
			catch ( FileNotFoundException e )
			{
				LOG.debug1( "FileNotFoundException" );
				// TODO handle this exception
			}
			catch ( UnsupportedFileVersionException e )
			{
				LOG.debug1( "UnsupportedFileVersionException" );
				// TODO handle this exception
			}
			catch ( IOException e )
			{
				LOG.debug1( "IOException" );
				// TODO handle this exception
			}
		}

		LOG.leaveMethod( "openFile" );
	}
}
