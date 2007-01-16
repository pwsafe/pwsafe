/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.passwordsafej;

import java.awt.BorderLayout;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.HeadlessException;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.border.EmptyBorder;

/**
 * Displays a password dialogue box and handles user interation with it.
 * 
 * @author Kevin Preece
 */
public class PasswordDialog extends JDialog
{
	private JPasswordField	EnteredPassword;
	private boolean			OkPressed;

	/**
	 * @param owner  the parent <code>Frame</code>.
	 * 
	 * @throws java.awt.HeadlessException
	 */
	public PasswordDialog( Frame owner ) throws HeadlessException
	{
		super( owner );

		setModal( true );
		setTitle( I18nHelper.getInstance().formatMessage("text.enter.safe.combo") );
		setResizable( false );

		buildCentrePanel();
		buildButtonPanel();

		addWindowEvents();

		pack();
		Util.centreWithin( owner, this );
		setVisible( true );
	}

	private void addWindowEvents()
	{
		this.addWindowListener( new WindowAdapter() {
			public void windowClosing( WindowEvent arg0 )
			{
				dispose();
				OkPressed	= false;
			}
			} );
	}

	private void buildButtonPanel()
	{
		OkCancelButtonBar	buttonbar;

		buttonbar		= new OkCancelButtonBar();

		buttonbar.addOkActionListener( new ActionListener() {
			public void actionPerformed( ActionEvent e )
			{
				dispose();
				OkPressed	= true;
			}
			} );

		buttonbar.addCancelActionListener( new ActionListener() {
			public void actionPerformed( ActionEvent e )
			{
				dispose();
				OkPressed	= false;
			}
			} );

		getContentPane().add( buttonbar, BorderLayout.SOUTH );
		getRootPane().setDefaultButton(buttonbar.getOkButton());
	}

	private void buildCentrePanel()
	{
		JPanel	panel;

		panel			= new JPanel( new GridLayout(0, 1) );
		EnteredPassword	= new JPasswordField(20);

		panel.setBorder( new EmptyBorder( 20, 20, 0, 20 ) );
		panel.add( new JLabel( I18nHelper.getInstance().formatMessage("text.safe.combo") ) );
		panel.add( EnteredPassword );

		getContentPane().add( panel, BorderLayout.CENTER );
	}

	/**
	 * Returns the password entered in the dialogue.
	 * 
	 * @return The password entered in the dialogue.
	 */
	public String getEnteredPassword()
	{
		// TODO Fix - Using a String is insecure.
		return new String( EnteredPassword.getPassword() );
	}

	/**
	 * 
	 * @return
	 */
	public boolean isOkPressed()
	{
		return OkPressed;
	}
}
