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
 * Preferences related to default username.
 *
 * @author Glen Smith
 */
public class UsernamePreferences extends PreferencePage {

	  // Names for preferences
	  public static final String USE_DEFAULT_USERNAME = "use.default.username";
	  public static final String DEFAULT_USERNAME = "default.username";
	  public static final String QUERY_FOR_DEFAULT_USERNAME = "query.for.default.username";

	  // Text fields for user to enter preferences
	  private Button btnUseDefaultUsername;
	  private Text txtUsername;
	  private Button btnQuerySetUsername;

	  /**
	   * Creates the controls for this page
	   */
	  protected Control createContents(Composite parent) {
	    Composite composite = new Composite(parent, SWT.NONE);
	    composite.setLayout(new GridLayout());
	    
	    // Get the preference store
	    IPreferenceStore preferenceStore = getPreferenceStore();

	    btnUseDefaultUsername = new Button(composite, SWT.CHECK);
	    btnUseDefaultUsername.setText("Use a default username");
	    btnUseDefaultUsername.setSelection(preferenceStore.getBoolean(USE_DEFAULT_USERNAME));


	    final Composite group = new Composite(composite, SWT.NONE);
	    final GridData gridData = new GridData();
	    gridData.widthHint = 284;
	    group.setLayoutData(gridData);
	    final GridLayout gridLayout = new GridLayout();
	    gridLayout.marginWidth = 40;
	    gridLayout.numColumns = 2;
	    group.setLayout(gridLayout);

	    final Label lblUsername = new Label(group, SWT.NONE);
	    lblUsername.setText("Username:");

	    txtUsername = new Text(group, SWT.BORDER);
	    txtUsername.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
	    txtUsername.setText(preferenceStore.getString(DEFAULT_USERNAME));

	    btnQuerySetUsername = new Button(composite, SWT.CHECK);
	    btnQuerySetUsername.setText("Query user to set default username");
	    btnQuerySetUsername.setSelection(preferenceStore.getBoolean(QUERY_FOR_DEFAULT_USERNAME));
	    btnQuerySetUsername.setEnabled(false);

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
	    btnUseDefaultUsername.setSelection(preferenceStore.getDefaultBoolean(USE_DEFAULT_USERNAME));
	    txtUsername.setText(preferenceStore.getDefaultString(DEFAULT_USERNAME));
	    btnQuerySetUsername.setSelection(preferenceStore.getDefaultBoolean(QUERY_FOR_DEFAULT_USERNAME));

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
	    preferenceStore.setValue(USE_DEFAULT_USERNAME, btnUseDefaultUsername.getSelection());
	    preferenceStore.setValue(DEFAULT_USERNAME, txtUsername.getText());
	    preferenceStore.setValue(QUERY_FOR_DEFAULT_USERNAME, btnQuerySetUsername.getSelection());

	    // Return true to allow dialog to close
	    return true;
	  }
}
