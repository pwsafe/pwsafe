/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.passwordsafej;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.ButtonGroup;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JRadioButtonMenuItem;

import org.pwsafe.lib.Log;

/**
 *
 * @author Kevin Preece
 */
public class MenuHandler
{
	/**
	 * Log4j logger
	 */
	private static final Log	LOG	= Log.getInstance( MenuHandler.class.getPackage().getName() );

	private JMenuBar		MenuBar;
	private PasswordSafeJ	Owner;
	private	JMenuItem		EditAdd;
	private	JMenuItem		EditEdit;
	private	JMenuItem		EditDelete;
	private JMenuItem		EditFind;
	private JMenuItem		EditCopyUser;
	private JMenuItem		EditCopyPass;
	private JMenuItem		ManageChangePw;
	private JMenuItem		ManageBackup;
	private JMenuItem		ManageRestore;

	/**
	 * 
	 * @param owner  
	 */
	public MenuHandler( PasswordSafeJ owner )
	{
		LOG.enterMethod( "MenuHandler" );

		MenuBar	= new JMenuBar();
		Owner	= owner;

		MenuBar.add( buildFileMenu() );
		MenuBar.add( buildEditMenu() );
		MenuBar.add( buildViewMenu() );
		MenuBar.add( buildManageMenu() );
		MenuBar.add( buildHelpMenu() );

		LOG.leaveMethod( "MenuHandler" );
	}

	private JMenu buildEditMenu()
	{
		LOG.enterMethod( "buildEditMenu" );

		JMenu		menu;
		JMenuItem	item;
	
		menu			= new JMenu( I18nHelper.getInstance().formatMessage("menu.edit") );
	
		EditAdd			= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.edit.add") );
		EditAdd.setEnabled( false );
		EditAdd.addActionListener( new ActionListener() {
			public void actionPerformed( ActionEvent event )
			{
				LOG.debug1( "Edit/Add menu option selected" );
				Owner.addEntry();
			}
		} );
		menu.add( EditAdd );
	
		EditEdit		= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.edit.edit") );
		EditEdit.setEnabled( false );
		menu.add( EditEdit );
	
		EditDelete		= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.edit.delete") );
		EditDelete.setEnabled( false );
		menu.add( EditDelete );
	
		EditFind		= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.edit.find") );
		EditFind.setEnabled( false );
		menu.add( EditFind );
	
		menu.addSeparator();
	
		EditCopyPass	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.edit.copypass") );
		EditCopyPass.setEnabled( false );
		menu.add( EditCopyPass );
	
		EditCopyUser	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.edit.copyuser") );
		EditCopyUser.setEnabled( false );
		menu.add( EditCopyUser );
	
		item		= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.edit.clearclip") );
		item.setEnabled( false );
		menu.add( item );

		LOG.leaveMethod( "buildEditMenu" );

		return menu;
	}

	private JMenuItem buildExportMenu()
	{
		LOG.enterMethod( "buildExportMenu" );

		JMenu		menu;
		JMenuItem	item;
		
		menu	= new JMenu( I18nHelper.getInstance().formatMessage("menu.file.export") );
		menu.setEnabled( false );
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.file.export.v1") );
		item.setEnabled( false );
		menu.add( item );
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.file.export.text") );
		item.setEnabled( false );
		menu.add( item );

		LOG.leaveMethod( "buildExportMenu" );

		return menu;
	}

	private JMenu buildFileMenu()
	{
		LOG.enterMethod( "buildFileMenu" );

		JMenu		menu;
		JMenuItem	item;
	
		menu	= new JMenu( I18nHelper.getInstance().formatMessage("menu.file") );
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.file.new") );
		item.setEnabled( true );
		menu.add( item );
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.file.open") );
		item.setEnabled( true );
		item.addActionListener( new ActionListener() {
			public void actionPerformed( ActionEvent event )
			{
				LOG.debug1( "Open file menu option selected" );
				Owner.openFile();
			}
		} );
		menu.add( item );
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.file.openrecent") );
		item.setEnabled( false );
		menu.add( item );
	
		menu.addSeparator();
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.file.save") );
		item.setEnabled( false );
		menu.add( item );
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.file.saveas") );
		item.setEnabled( false );
		menu.add( item );
	
		menu.addSeparator();
	
		menu.add( buildExportMenu() );
	
		menu.addSeparator();
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.file.exit") );
		item.setEnabled( true );
		item.addActionListener( new ActionListener() {
				public void actionPerformed( ActionEvent event )
				{
					LOG.debug1( "Exit menu option selected" );
					Owner.exit();
				}
			} );
		menu.add( item );

		LOG.leaveMethod( "buildFileMenu" );
	
		return menu;
	}

	private JMenu buildHelpMenu()
	{
		LOG.enterMethod( "buildHelpMenu" );

		JMenu		menu;
		JMenuItem	item;
	
		menu	= new JMenu( I18nHelper.getInstance().formatMessage("menu.help") );
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.help.help") );
		item.setEnabled( true );
		menu.add( item );
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.help.online") );
		item.setEnabled( false );
		menu.add( item );
	
		item	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.help.about") );
		item.setEnabled( true );
		item.addActionListener( new ActionListener() {
			public void actionPerformed( ActionEvent event )
			{
				LOG.debug1( "\"Help/About\" option selected" );
				new AboutDialog(Owner);
			}
		} );
		menu.add( item );

		LOG.leaveMethod( "buildHelpMenu" );

		return menu;
	}

	private JMenu buildManageMenu()
	{
		LOG.enterMethod( "buildManageMenu" );

		JMenu		menu;
		JMenuItem	item;
	
		menu			= new JMenu( I18nHelper.getInstance().formatMessage("menu.manage") );
	
		ManageChangePw	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.manage.changepass") );
		ManageChangePw.setEnabled( false );
		menu.add( ManageChangePw );
	
		menu.addSeparator();
	
		ManageBackup	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.manage.backup") );
		ManageBackup.setEnabled( false );
		menu.add( ManageBackup );
	
		ManageRestore	= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.manage.restore") );
		ManageRestore.setEnabled( false );
		menu.add( ManageRestore );
	
		menu.addSeparator();
	
		item			= new JMenuItem( I18nHelper.getInstance().formatMessage("menu.manage.options") );
		item.setEnabled( true );
		menu.add( item );

		LOG.leaveMethod( "buildManageMenu" );

		return menu;
	}

	private JMenu buildViewMenu()
	{
		LOG.enterMethod( "buildViewMenu" );

		JMenu		menu;
		JMenuItem	item;
		ButtonGroup	group;
	
		group	= new ButtonGroup();
		menu	= new JMenu( I18nHelper.getInstance().formatMessage("menu.view") );
	
		item	= new JRadioButtonMenuItem( I18nHelper.getInstance().formatMessage("menu.view.list") );
		item.setEnabled( false );
		item.setSelected( false );
		group.add( item );
		menu.add( item );
	
		item	= new JRadioButtonMenuItem( I18nHelper.getInstance().formatMessage("menu.view.tree") );
		item.setEnabled( true );
		item.setSelected( true );
		group.add( item );
		menu.add( item );

		LOG.leaveMethod( "buildViewMenu" );

		return menu;
	}

	void fileOpened()
	{
		EditAdd.setEnabled( true );
		EditFind.setEnabled( true );
		ManageChangePw.setEnabled( true );
		ManageBackup.setEnabled( true );
		ManageRestore.setEnabled( true );
	}

	void leafSelected( boolean isLeafNode )
	{
		EditEdit.setEnabled( isLeafNode );
		EditDelete.setEnabled( isLeafNode );
		EditCopyUser.setEnabled( isLeafNode );
		EditCopyPass.setEnabled( isLeafNode );
	}

	/**
	 * @return Returns the menuBar.
	 */
	public JMenuBar getMenuBar()
	{
		return MenuBar;
	}
}
