/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.passwordsafej;

import java.awt.BorderLayout;
import java.awt.HeadlessException;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SpringLayout;
import javax.swing.border.EmptyBorder;

/**
 *
 * @author Kevin Preece
 */
public class AddDialog extends JDialog
{
	private JTextField	Group;
	private JTextField	Title;
	private JTextField	Username;
	private JTextField	Password;
	private JTextArea	Notes;

	/**
	 * @param owner
	 * @throws java.awt.HeadlessException
	 */
	public AddDialog( PasswordSafeJ owner ) throws HeadlessException
	{
		super( owner );
		setModal( true );
		setTitle( I18nHelper.getInstance().formatMessage("adddlg.title") );
		setResizable( false );

		buildTopPanel();
		buildCentrePanel();
		buildRightPanel();

		pack();
		Util.centreWithin( owner, this );
		show();
	}

	private void buildCentrePanel()
	{
		JPanel		innerPanel;
		JLabel		label;
		JScrollPane	scrollPane;

		innerPanel	= new JPanel( new SpringLayout() );

		label		= new JLabel( I18nHelper.getInstance().formatMessage("adddlg.label.group") );
		Group		= new JTextField( 20 );
		label.setLabelFor( Group );

		innerPanel.add( label );
		innerPanel.add( Group );

		label		= new JLabel( I18nHelper.getInstance().formatMessage("adddlg.label.title") );
		Title		= new JTextField( 20 );
		label.setLabelFor( Title );

		innerPanel.add( label );
		innerPanel.add( Title );

		label		= new JLabel( I18nHelper.getInstance().formatMessage("adddlg.label.username") );
		Username	= new JTextField( 20 );
		label.setLabelFor( Username );

		innerPanel.add( label );
		innerPanel.add( Username );

		label		= new JLabel( I18nHelper.getInstance().formatMessage("adddlg.label.password") );
		Password	= new JTextField( 20 );
		label.setLabelFor( Password );

		innerPanel.add( label );
		innerPanel.add( Password );

		label		= new JLabel( I18nHelper.getInstance().formatMessage("adddlg.label.notes") );
		Notes		= new JTextArea( 5, 20 );
		scrollPane	= new JScrollPane( Notes );
		label.setLabelFor( scrollPane );

		innerPanel.add( label );
		innerPanel.add( scrollPane );

		innerPanel.setBorder( new EmptyBorder( 20, 8, 8, 8 ) );

		SpringUtilities.makeCompactGrid( innerPanel, 5, 2, 6, 6, 6, 6 );

		getContentPane().add( innerPanel, BorderLayout.CENTER );
	}

	private void buildRightPanel()
	{
		ButtonBar	bar;
		JButton		button;

		bar		= new ButtonBar( ButtonBar.VERTICAL, ButtonBar.TOP );

		button		= new JButton( I18nHelper.getInstance().formatMessage("button.ok") );
		bar.add( button );

		button.addActionListener( new ActionListener() {
			public void actionPerformed( ActionEvent e ) {
				dispose();
			}} );

		// Set the OK button as the default
		getRootPane().setDefaultButton( button );

		button		= new JButton( I18nHelper.getInstance().formatMessage("button.cancel") );
		bar.add( button );

		button.addActionListener( new ActionListener() {
			public void actionPerformed( ActionEvent e ) {
				dispose();
			}} );

		button		= new JButton( I18nHelper.getInstance().formatMessage("button.help") );
		bar.add( button );

//		button.addActionListener( new ActionListener() {
//			public void actionPerformed( ActionEvent e ) {
//				dispose();
//			}} );

		button		= new JButton( I18nHelper.getInstance().formatMessage("button.random.password") );
		bar.add( button );

//		button.addActionListener( new ActionListener() {
//			public void actionPerformed( ActionEvent e ) {
//				dispose();
//			}} );

		getContentPane().add( bar, BorderLayout.EAST );
	}

	private void buildTopPanel()
	{
		JTextArea	text1;

		text1	= new JTextArea( I18nHelper.getInstance().formatMessage("adddlg.text") );

		text1.setEditable( false );
		text1.setBackground( getBackground() );
		text1.setBorder( new EmptyBorder( 8, 8, 16, 8 ) );

		getContentPane().add( text1, BorderLayout.NORTH );
	}
}
