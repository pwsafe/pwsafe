/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.passwordsafej;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;

import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

/**
 * Creates a container suitable for holding buttons.  The buttons are added using the {@link #add(JButton)}
 * method and are arranged as a horizontal strip with each button the same size as the widest button.  If
 * the button bar is put into a container wider than its natural size, the buttons are not resized, but
 * horizontal spacing is added to centre the buttons within the area.
 * 
 * @author Kevin Preece
 */
public class ButtonBar extends JPanel
{
	public static final int		LEFT		= 0;
	public static final int		TOP			= 0;
	public static final int		RIGHT		= 1;
	public static final int		BOTTOM		= 1;
	public static final int		CENTRE		= 2;
	public static final int		CENTER		= 2;
	public static final int		HORIZONTAL	= 3;
	public static final int		VERTICAL	= 4;

	private JPanel		ButtonPanel;

	/**
	 * Creates a button bar that is initially empty with the default orientation of
	 * <code>HORIZONTAL</code> and alignment of <code>CENTRE</code>.
	 */
	public ButtonBar()
	{
		this( HORIZONTAL, CENTRE );
	}

	/**
	 * Creates a bar that JButton buttons are added to.  All buttons will be resized
	 * so that they are equal.
	 * 
	 * @param orientation  the orientation of the button bar <code>HORIZONTAL</code> or <code>VERTICAL</code>.
	 * @param alignment    the button bar alignment - <code>LEFT</code>, <code>CENTRE</code> or <code>RIGHT</code>. 
	 */
	public ButtonBar( int orientation, int alignment )
	{
		super();

		JPanel				layout;
		GridBagConstraints	c;
		GridLayout			buttonLayout;

		if ( orientation == HORIZONTAL )
		{
			// rows, cols, hgap, vgap
			buttonLayout = new GridLayout( 1, 0, 8, 0 );
		}
		else if ( orientation == VERTICAL )
		{
			// rows, cols, hgap, vgap
			buttonLayout = new GridLayout( 0, 1, 0, 4 );
		}
		else
		{
			throw new IllegalArgumentException();
		}

		if ( (alignment != LEFT) && (alignment != CENTRE) && (alignment != RIGHT) )
		{
			throw new IllegalArgumentException();
		}

		layout			= new JPanel( new GridBagLayout() );
		ButtonPanel		= new JPanel( buttonLayout );
		c				= new GridBagConstraints();

		if ( (alignment == CENTRE) || (alignment == RIGHT) )
		{
			c.fill			= GridBagConstraints.HORIZONTAL;
			layout.add( new SpacerPanel( 10, 10 ), c );
		}
		
		c.fill			= GridBagConstraints.NONE;
		layout.add( ButtonPanel, c );

		if ( (alignment == CENTRE) || (alignment == LEFT) )
		{
			c.fill			= GridBagConstraints.HORIZONTAL;
			layout.add( new SpacerPanel( 10, 10 ), c );
		}

		this.setBorder( new EmptyBorder( 24, 0, 6, 0 ) );
		this.add( layout );
	}

	/**
	 * Adds a button to this button bar.
	 * 
	 * @param button the button to add.
	 */
	public void add( JButton button )
	{
		ButtonPanel.add( button );
	}
}
