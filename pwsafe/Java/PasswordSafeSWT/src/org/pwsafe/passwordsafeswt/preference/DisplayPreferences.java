package org.pwsafe.passwordsafeswt.preference;

import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.preference.PreferencePage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;

/**
 * GUI related preferences.
 * 
 * @author Glen Smith
 */
public class DisplayPreferences extends PreferencePage {

	// Names for preferences
	public static final String ALWAYS_ON_TOP = "display.always.on.top";
	public static final String SHOW_PASSWORD_IN_LIST = "show.password.in.list";
	public static final String SHOW_PASSWORD_IN_EDIT_MODE = "show.password.in.edit.mode";
	public static final String SHOW_ICON_IN_SYSTEM_TRAY = "show.icon.in.system.tray";
	public static final String TREE_COLUMN_SIZE = "tree.width.column";
	public static final String TABLE_COLUMN_SIZE = "table.width.column";

	Button btnAlwaysOnTop;
	Button btnShowPasswordInList;
	Button btnShowPasswordInEdit;
	Button btnSystemTray;

	// Text fields for user to enter preferences

	public DisplayPreferences() {
		super();
	}

	/**
	 * Creates the controls for this page
	 */
	protected Control createContents(Composite parent) {
		Composite composite = new Composite(parent, SWT.NONE);
		composite.setLayout(new GridLayout());
		
		// Get the preference store and setup defaults
	    IPreferenceStore preferenceStore = getPreferenceStore();	
		preferenceStore.setDefault(SHOW_ICON_IN_SYSTEM_TRAY, true);

		btnAlwaysOnTop = new Button(composite, SWT.CHECK);
		btnAlwaysOnTop.setText("Always keep Password Safe on top");
		btnAlwaysOnTop.setSelection(preferenceStore.getBoolean(ALWAYS_ON_TOP));
		btnAlwaysOnTop.setEnabled(false);

		btnShowPasswordInList = new Button(composite, SWT.CHECK);
		btnShowPasswordInList.setText("Show password in display list");
		btnShowPasswordInList.setSelection(preferenceStore.getBoolean(SHOW_PASSWORD_IN_LIST));

		btnShowPasswordInEdit = new Button(composite, SWT.CHECK);
		btnShowPasswordInEdit.setText("Show password by default in edit mode");
		btnShowPasswordInEdit.setSelection(preferenceStore.getBoolean(SHOW_PASSWORD_IN_EDIT_MODE));

		btnSystemTray = new Button(composite, SWT.CHECK);
		btnSystemTray.setText("Put icon in System Tray");
		btnSystemTray.setSelection(preferenceStore.getBoolean(SHOW_ICON_IN_SYSTEM_TRAY));

		// Create three text fields.
		// Set the text in each from the preference store

		return composite;
	}
	

	/**
	 * Called when user clicks Restore Defaults
	 */
	protected void performDefaults() {
		// Get the preference store
		IPreferenceStore preferenceStore = getPreferenceStore();

		// Reset the fields to the defaults
		btnAlwaysOnTop.setSelection(preferenceStore.getDefaultBoolean(ALWAYS_ON_TOP));
		btnShowPasswordInList.setSelection(preferenceStore.getDefaultBoolean(SHOW_PASSWORD_IN_LIST));
		btnShowPasswordInEdit.setSelection(preferenceStore.getDefaultBoolean(SHOW_PASSWORD_IN_EDIT_MODE));
		btnSystemTray.setSelection(preferenceStore.getDefaultBoolean(SHOW_ICON_IN_SYSTEM_TRAY));
	}

	/**
	 * Called when user clicks Apply or OK
	 * 
	 * @return boolean true if the dialog is allowed to close, false otherwise
	 */
	public boolean performOk() {
		// Get the preference store
		IPreferenceStore preferenceStore = getPreferenceStore();

		// Set the values from the fields
		if (btnAlwaysOnTop != null) preferenceStore.setValue(ALWAYS_ON_TOP, btnAlwaysOnTop.getSelection());
	    if (btnShowPasswordInList != null) preferenceStore.setValue(SHOW_PASSWORD_IN_LIST, btnShowPasswordInList.getSelection());
	    if (btnShowPasswordInEdit != null)
	        preferenceStore.setValue(SHOW_PASSWORD_IN_EDIT_MODE, btnShowPasswordInEdit.getSelection());
	    if (btnSystemTray != null)
	        preferenceStore.setValue(SHOW_ICON_IN_SYSTEM_TRAY, btnSystemTray.getSelection());

		// Return true to allow dialog to close
		return true;
	}
}