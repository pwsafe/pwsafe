/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.passwordsafej;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Creats a {@link ButtonBar} containing an "Ok" and a "Cancel" button.
 * 
 * @author Kevin Preece
 */
public class OkCancelButtonBar extends ButtonBar
{
	private	OkButton		okButton;
	private	CancelButton	cancelButton;
	private boolean			OkPressed;

	/**
	 * 
	 */
	public OkCancelButtonBar()
	{
		super();

		okButton		= new OkButton();
		cancelButton	= new CancelButton();

		okButton.addActionListener( new ActionListener() {
			public void actionPerformed( ActionEvent e )
			{
				OkPressed	= true;
			}
			} );

		cancelButton.addActionListener( new ActionListener() {
			public void actionPerformed( ActionEvent e )
			{
				OkPressed	= false;
			}
			} );
		
		super.add( okButton );
		super.add( cancelButton );
	}

	/**
	 * Adds an <code>ActionListener</code> that will be called when the "Cancel" button is pressed.
	 * 
	 * @param listener  the <code>ActionListener</code> to add.
	 */
	public void addCancelActionListener( ActionListener listener )
	{
		cancelButton.addActionListener( listener );
	}

	/**
	 * Adds an <code>ActionListener</code> that will be called when the "Ok" button is pressed.
	 * 
	 * @param listener  the <code>ActionListener</code> to add.
	 */
	public void addOkActionListener( ActionListener listener )
	{
		okButton.addActionListener( listener );
	}

	/**
	 * @return Returns the cancelButton.
	 */
	public CancelButton getCancelButton()
	{
		return cancelButton;
	}

	/**
	 * @return Returns the okButton.
	 */
	public OkButton getOkButton()
	{
		return okButton;
	}

	/**
	 * Returns <code>true</code> if the "Cancel" button was pressed.
	 * 
	 * @return
	 */
	public boolean isCancelPressed()
	{
		return !OkPressed;
	}

	/**
	 * Returns <code>true</code> if the "Ok" button was pressed.
	 * 
	 * @return
	 */
	public boolean isOkPressed()
	{
		return OkPressed;
	}
}
