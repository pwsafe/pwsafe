package org.pwsafe.passwordsafeswt.preference;

import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.preference.PreferencePage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

/**
 * Miscellaneous preference items.
 *
 * @author Glen Smith
 */
public class MiscPreferences extends PreferencePage {

	  // Names for preferences
	  public static final String CONFIRM_ITEM_DELETION = "confirm.item.deletion";
	  public static final String SAVE_IMMEDIATELY_ON_EDIT = "save.immediately.on.edit";
	  public static final String ESCAPE_KEY_EXITS_APP = "escape.key.exits.app";
	  public static final String HOT_KEY_ACTIVE = "hot.key.active";
	  public static final String HOT_KEY = "hot.key.value";
	  public static final String DOUBLE_CLICK_COPIES_TO_CLIPBOARD = "double.click.copies.to.clipboard";
	  
	  // Text fields for user to enter preferences
	  private Button btnConfirmDeletion;
	  private Button btnSaveImmediately;
	  private Button btnEscapeExitsApp;
	  private Button btnHotKey;
	  private Text txtHotKey;
	  private Button btnCopiesPasswordToClipboard;
	  private Button btnViewsEntry;


	  /**
	   * Creates the controls for this page
	   */
	  protected Control createContents(Composite parent) {
	    Composite composite = new Composite(parent, SWT.NONE);
	    composite.setLayout(new GridLayout());
	    
		// Get the preference store and setup defaults
	    IPreferenceStore preferenceStore = getPreferenceStore();

	    btnConfirmDeletion = new Button(composite, SWT.CHECK);
	    btnConfirmDeletion.setText("Confirm deletion of items");
	    btnConfirmDeletion.setSelection(preferenceStore.getBoolean(CONFIRM_ITEM_DELETION));
	    btnConfirmDeletion.setEnabled(false);

	    btnSaveImmediately = new Button(composite, SWT.CHECK);
	    btnSaveImmediately.setText("Save database immediately after Edit or Add");
	    btnSaveImmediately.setSelection(preferenceStore.getBoolean(SAVE_IMMEDIATELY_ON_EDIT));

	    btnEscapeExitsApp = new Button(composite, SWT.CHECK);
	    btnEscapeExitsApp.setText("Escape key exits application");
	    btnEscapeExitsApp.setSelection(preferenceStore.getBoolean(ESCAPE_KEY_EXITS_APP));
	    btnEscapeExitsApp.setEnabled(false);

	    final Composite compositeHotKey = new Composite(composite, SWT.NONE);
	    final GridLayout gridLayout = new GridLayout();
	    gridLayout.marginWidth = 0;
	    gridLayout.numColumns = 2;
	    gridLayout.marginHeight = 0;
	    compositeHotKey.setLayout(gridLayout);

	    btnHotKey = new Button(compositeHotKey, SWT.CHECK);
	    btnHotKey.setText("Hot key");
	    btnHotKey.setSelection(preferenceStore.getBoolean(HOT_KEY_ACTIVE));
	    btnHotKey.setEnabled(false);

	    txtHotKey = new Text(compositeHotKey, SWT.BORDER);
	    txtHotKey.setText(preferenceStore.getString(HOT_KEY));
	    txtHotKey.setEnabled(false);

	    final Composite compositeDoubleClick = new Composite(composite, SWT.NONE);
	    final GridLayout gridLayout_1 = new GridLayout();
	    gridLayout_1.numColumns = 2;
	    compositeDoubleClick.setLayout(gridLayout_1);

	    final Label lblDoubleClick = new Label(compositeDoubleClick, SWT.NONE);
	    lblDoubleClick.setLayoutData(new GridData(GridData.VERTICAL_ALIGN_BEGINNING));
	    lblDoubleClick.setText("Double-click on entry:");

	    final Composite compositeRadios = new Composite(compositeDoubleClick, SWT.NONE);
	    final GridLayout gridLayout_2 = new GridLayout();
	    gridLayout_2.marginWidth = 0;
	    gridLayout_2.marginHeight = 0;
	    compositeRadios.setLayout(gridLayout_2);

	    btnCopiesPasswordToClipboard = new Button(compositeRadios, SWT.RADIO);
	    btnCopiesPasswordToClipboard.setText("Copies password to clipboard");

	    btnViewsEntry = new Button(compositeRadios, SWT.RADIO);
	    btnViewsEntry.setText("View/Edit entry");
	    
	    if (preferenceStore.getBoolean(DOUBLE_CLICK_COPIES_TO_CLIPBOARD)) {
	    	btnCopiesPasswordToClipboard.setSelection(true);
	    } else {
	    	btnViewsEntry.setSelection(true);
	    }
	    

	    return composite;
	  }

	  /**
	   * Called when user clicks Restore Defaults
	   */
	  protected void performDefaults() {
	    // Get the preference store
	    IPreferenceStore preferenceStore = getPreferenceStore();

	    // Reset the fields to the defaults
	    btnConfirmDeletion.setSelection(preferenceStore.getDefaultBoolean(CONFIRM_ITEM_DELETION));
	    btnSaveImmediately.setSelection(preferenceStore.getDefaultBoolean(SAVE_IMMEDIATELY_ON_EDIT));
	    btnEscapeExitsApp.setSelection(preferenceStore.getDefaultBoolean(ESCAPE_KEY_EXITS_APP));
	    btnHotKey.setSelection(preferenceStore.getDefaultBoolean(HOT_KEY_ACTIVE));
	    txtHotKey.setText(preferenceStore.getDefaultString(HOT_KEY));
	    if (preferenceStore.getDefaultBoolean(DOUBLE_CLICK_COPIES_TO_CLIPBOARD)) {
	    	btnCopiesPasswordToClipboard.setSelection(true);
	    } else {
	    	btnViewsEntry.setSelection(true);
	    }

	  }

	  /**
	   * Called when user clicks Apply or OK
	   * 
	   * @return boolean
	   */
	  public boolean performOk() {
	    // Get the preference store
	    IPreferenceStore preferenceStore = getPreferenceStore();

	    // Set the values from the fields
	    preferenceStore.setValue(CONFIRM_ITEM_DELETION, btnConfirmDeletion.getSelection());
   		preferenceStore.setValue(SAVE_IMMEDIATELY_ON_EDIT,btnSaveImmediately.getSelection());
		preferenceStore.setValue(ESCAPE_KEY_EXITS_APP,btnEscapeExitsApp.getSelection());
		preferenceStore.setValue(HOT_KEY_ACTIVE,btnHotKey.getSelection());
		preferenceStore.setValue(HOT_KEY,txtHotKey.getText());
		preferenceStore.setValue(DOUBLE_CLICK_COPIES_TO_CLIPBOARD, btnCopiesPasswordToClipboard.getSelection());
	    
	    // Return true to allow dialog to close
	    return true;
	  }
}
