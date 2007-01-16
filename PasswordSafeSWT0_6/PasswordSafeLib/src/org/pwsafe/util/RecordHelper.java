/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.util;

import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;

import javax.swing.tree.DefaultMutableTreeNode;

import org.pwsafe.lib.Log;
import org.pwsafe.lib.UUID;
import org.pwsafe.lib.exception.PasswordSafeException;
import org.pwsafe.lib.exception.UnsupportedFileVersionException;
import org.pwsafe.lib.file.PwsFile;
import org.pwsafe.lib.file.PwsFileFactory;
import org.pwsafe.lib.file.PwsFileV1;
import org.pwsafe.lib.file.PwsFileV2;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;

/**
 * Provides utility methods for converting a collection of {@link PwsRecord}s taken from a
 * {@link PwsFile) into other collection types suitable for processing as a heirarchical
 * structure or adding to a swing component.
 */
public class RecordHelper
{
	/**
	 * Log4j logger
	 */
	private static final Log	LOG	= Log.getInstance( RecordHelper.class.getPackage().getName() );

	/**
	 * Private for singleton pattern.
	 */
	private RecordHelper()
	{
		super();
	}

	/**
	 * 
	 * @param node
	 * @param map
	 * 
	 * @return
	 */
	private static DefaultMutableTreeNode addChildren( DefaultMutableTreeNode node, HashMap map )
	{
		for ( Iterator iter = map.keySet().iterator(); iter.hasNext(); )
		{
			String	key;
			Object	ob;

			key	= (String) iter.next(); 
			ob	= map.get( key );

			if ( ob instanceof HashMap )
			{
				node.add( addChildren( new DefaultMutableTreeNode(key), (HashMap) ob ) );
			}
			else if ( ob instanceof PwsRecord )
			{
				node.add( new DefaultMutableTreeNode( new DefaultRecordWrapper( (PwsRecord) ob ) ) );
			}
		}
		return node;
	}

	/**
	 * Creates a simple test tree.
	 * 
	 * @return
	 * 
	 * @throws PasswordSafeException
	 * @throws UnsupportedFileVersionException
	 */
	public static DefaultMutableTreeNode createTestTree()
	throws PasswordSafeException, UnsupportedFileVersionException
	{
		return recordsAsSwingNodes( PwsFileFactory.testFile() );
	}

	/**
	 * Finds the node named <code>name</code>.  Name is an empty string or a list of node
	 * names, separated by 'dots', e.g. "bank.online".  Nodes are created as necessary if
	 * they don't exist. 
	 * 
	 * @param root  the root node where the search starts.
	 * @param name  the node to look for.
	 * 
	 * @return The node named <code>name</code>.
	 */
	private static HashMap findNode( HashMap root, String name )
	{
		LOG.enterMethod( "findNode" );

		StringTokenizer	st;
		HashMap			node;
		HashMap			temp;
		String			token;

		st		= new StringTokenizer( name, "." );
		node	= root;

		while ( st.hasMoreTokens() )
		{
			token	= st.nextToken();
			temp	= (HashMap) node.get( token );

			if ( temp == null )
			{
				temp	= new HashMap();
				node.put( token, temp );
			}
			node	= temp;
		}
		LOG.leaveMethod( "findNode" );

		return node;
	}

	/**
	 * Returns the records from file as a heirarchical tree structure of <code>DefaultMutableTreeNode</code>s.
	 * 
	 * @param file  the file to getthe records from.
	 * 
	 * @return The root node of the tree.
	 * 
	 * @throws UnsupportedFileVersionException
	 */
	public static DefaultMutableTreeNode recordsAsSwingNodes( PwsFile file )
	throws UnsupportedFileVersionException
	{
		LOG.enterMethod( "recordsAsSwingNodes" );

		DefaultMutableTreeNode	root;

		root = addChildren( new DefaultMutableTreeNode(), recordsAsTree(file) );

		LOG.leaveMethod( "recordsAsSwingNodes" );

		return root;
	}

	/**
	 * Returns the records from file in a heirarchical tree-like structure.  The root node
	 * is a <code>HasMap</code> which can have zero or more nodes as entries.  Nodes which themselves
	 * have further sub-entries are stored as a <code>HashMap</code>.  Leaf nodes are stored as the
	 * {@link PwsRecord}.  E.g.
	 * </p><p>
	 * <tt><pre>HashMap                    (root node)
	 * +-- HashMap                "bank"
	 * |   +-- HashMap            "online"
	 * |   |   +-- PwsRecord      "Online Bank 1"
	 * |   |   +-- PwsRecord      "Online Bank 2"
	 * |   +-- PwsRecord          "Lone Entry"
	 * |   +-- HashMap            "telephone"
	 * |       +-- PwsRecord      "Telephone Bank 1"
	 * +-- HashMap                "websites"
	 *     +-- PwsRecord          "Some Online Store"</pre>
	 * </tt>
	 * </p><p>
	 * Which corresponds to records:
	 * <table>
	 *   <tr>
	 *     <th>Group</th>
	 *     <th>Title</th>
	 *   </tr>
	 *   <tr>
	 *     <td>bank.online</td>
	 *     <td>Online Bank 1</td>
	 *   </tr>
	 *   <tr>
	 *     <td>bank.online</td>
	 *     <td>Online Bank 2</td>
	 *   </tr>
	 *   <tr>
	 *     <td>bank</td>
	 *     <td>Lone Entry</td>
	 *   </tr>
	 *   <tr>
	 *     <td>bank.telephone</td>
	 *     <td>Telephone Bank 1</td>
	 *   </tr>
	 *   <tr>
	 *     <td>websites</td>
	 *     <td>Some Online Store</td>
	 *   </tr>
	 * </table>
	 * </p><p>
	 * Note, however, that PasswordSafe files in V1.x format don't have groups.  Therefore this
	 * method will simply return a flat heirarchy.
	 * 
	 * @param file  the file to get the records from.
	 * 
	 * @return
	 * 
	 * @throws UnsupportedFileVersionException if <code>file</code> is not a V1 or V2 file.
	 */
	public static HashMap recordsAsTree( PwsFile file )
	throws UnsupportedFileVersionException
	{
		if ( file instanceof PwsFileV1 )
		{
			return recordsAsTreeV1( (PwsFileV1) file );
		}
		else if ( file instanceof PwsFileV2 )
		{
			return recordsAsTreeV2( (PwsFileV2) file );
		}
		throw new UnsupportedFileVersionException();
	}

	/**
	 * @param file  the file to get the records from.
	 * 
	 * @return The records as a flat structure.
	 */
	private static HashMap recordsAsTreeV1( PwsFileV1 file )
	{
		HashMap	tree;

		tree	= new HashMap();

		for ( Iterator iter = file.getRecords(); iter.hasNext(); )
		{
			PwsRecordV1	rec;

			rec = (PwsRecordV1) iter.next();

			tree.put( new UUID().toString(), rec );
		}
		
		return tree;
	}

	/**
	 * @param file  the file to get the records from.
	 *
	 * @return The records as a heirarcical structure.
	 */
	private static HashMap recordsAsTreeV2( PwsFileV2 file )
	{
		HashMap	tree;

		tree	= new HashMap();

		for ( Iterator iter = file.getRecords(); iter.hasNext(); )
		{
			PwsRecordV2	rec;
			HashMap		node;

			rec		= (PwsRecordV2) iter.next();
			node	= findNode( tree, rec.getField(PwsRecordV2.GROUP).toString() );

			node.put( rec.getField(PwsRecordV2.UUID).toString(), rec );
		}
		
		return tree;
	}
}
