/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.passwordsafej;

import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreeSelectionModel;

import org.pwsafe.lib.Log;
import org.pwsafe.lib.exception.PasswordSafeException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;
import org.pwsafe.lib.file.PwsFile;
import org.pwsafe.util.DefaultRecordWrapper;
import org.pwsafe.util.RecordHelper;

/**
 *
 * @author Kevin Preece
 */
public class TreeHandler implements TreeSelectionListener
{
	/**
	 * Log4j logger
	 */
	private static final Log	LOG	= Log.getInstance( TreeHandler.class.getPackage().getName() );

	private	JTree					Tree		= null;
	private DefaultMutableTreeNode	RootNode	= null;
	private JScrollPane				ScrollPane	= null;
	private PasswordSafeJ 			Owner		= null;

	/**
	 * 
	 * @param owner
	 */
	public TreeHandler( PasswordSafeJ owner )
	{
		super();

		LOG.enterMethod( "TreeHandler" );

//		try
		{
			RootNode	= new DefaultMutableTreeNode();

			setOptions( owner );
		}
//		catch ( UnsupportedFileVersionException e )
//		{
//		}
//		catch ( PasswordSafeException e )
//		{
//		}
		
		LOG.leaveMethod( "TreeHandler" );
	}

	/**
	 * @param owner
	 * @param file
	 * 
	 * @throws UnsupportedFileVersionException
	 */
	public TreeHandler( PasswordSafeJ owner, PwsFile file )
	throws UnsupportedFileVersionException
	{
		LOG.enterMethod( "TreeHandler(PwsFile)" );

		RootNode	= RecordHelper.recordsAsSwingNodes( file );

		setOptions( owner );

		LOG.leaveMethod( "TreeHandler(PwsFile)" );
	}

	/**
	 * @return Returns the rootNode.
	 */
	public DefaultMutableTreeNode getRootNode()
	{
		return RootNode;
	}

	/**
	 * Returns the tree in a ScrollPane
	 * 
	 * @return
	 */
	public JScrollPane getScrollPane()
	{
		if ( ScrollPane == null )
		{
			ScrollPane	= new JScrollPane( Tree );
		}
		return ScrollPane;
	}

	/**
	 * @return Returns the tree.
	 */
	public JTree getTree()
	{
		return Tree;
	}

	private void setOptions( PasswordSafeJ owner )
	{
		Tree	= new JTree( RootNode );
		Owner	= owner;

		Tree.setRootVisible( false );
		Tree.setEditable( false );
		Tree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
		Tree.addTreeSelectionListener( this );
	}

	/**
	 * Handles selection of a tree node.
	 * 
	 * @param e
	 * 
	 * @see javax.swing.event.TreeSelectionListener#valueChanged(javax.swing.event.TreeSelectionEvent)
	 */
	public void valueChanged( TreeSelectionEvent e )
	{
		LOG.enterMethod( "valueChanged" );

		DefaultMutableTreeNode	node;

		node	= (DefaultMutableTreeNode) Tree.getLastSelectedPathComponent();

		if ( node != null )
		{
			Object	userObject;

			userObject	= node.getUserObject();

			Owner.leafSelected( ( userObject instanceof DefaultRecordWrapper ) ? true : false );
		}

		LOG.leaveMethod( "valueChanged" );
	}
}
