/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.passwordsafej;

import javax.swing.JButton;

/**
 *
 * @author Kevin Preece
 */
public class OkButton extends JButton
{
	/**
	 * 
	 */
	public OkButton()
	{
		super();
		setText( I18nHelper.getInstance().formatMessage("button.ok") );
	}
}
