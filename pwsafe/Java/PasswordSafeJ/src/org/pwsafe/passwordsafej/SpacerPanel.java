/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.passwordsafej;

import java.awt.Dimension;

import javax.swing.JPanel;

/**
 * Creates a blank panel to be used for spacing between components or borders around a dialogue.
 * 
 * @author Kevin Preece
 */
public class SpacerPanel extends JPanel
{
	/**
	 * Creates a JPanel with its preferred size set to the given dimensions.  The resulting
	 * JPanel is suitable for use as whitespace filler in layouts.
	 * 
	 * @param width   the preferred width of this spacer.
	 * @param height  the preferred height of this spacer.
	 */
	public SpacerPanel( int width, int height )
	{
		super();
		setPreferredSize( new Dimension( width, height ) );
	}
}
