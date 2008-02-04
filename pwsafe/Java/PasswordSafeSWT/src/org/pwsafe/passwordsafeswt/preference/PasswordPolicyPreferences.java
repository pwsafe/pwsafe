package org.pwsafe.passwordsafeswt.preference;

import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.preference.PreferencePage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Spinner;

/**
 * Preferences related to password generation.
 *
 * @author Glen Smith
 */
public class PasswordPolicyPreferences extends PreferencePage {

	  // Names for preferences
	  public static final String DEFAULT_PASSWORD_LENGTH = "default.password.length";
	  public static final String USE_LOWERCASE_LETTERS = "use.lowercase.letters";
	  public static final String USE_UPPERCASE_LETTERS = "use.uppercase.letters";
	  public static final String USE_DIGITS = "use.digits";
	  public static final String USE_SYMBOLS = "use.symbols";
	  public static final String USE_EASY_TO_READ = "use.easy.to.read";
	  public static final String USE_HEX_ONLY = "use.hex.only";

	  // Text fields for user to enter preferences
	  private Spinner spiLength;
	  private Button btnUseLowercase;
	  private Button btnUserUppercase;
	  private Button btnUseDigits;
	  private Button btnUseSymbols;
	  private Button btnUseEaseToRead;
	  private Button btnUseHexOnly;
	  

	  /**
	   * Creates the controls for this page
	   */
	  protected Control createContents(Composite parent) {
	    Composite composite = new Composite(parent, SWT.NONE);
	    composite.setLayout(new GridLayout());
	    
	    // Get the preference store
	    IPreferenceStore preferenceStore = getPreferenceStore();


	    final Label lblRandomRules = new Label(composite, SWT.NONE);
	    lblRandomRules.setText("Random password generation rules:");

	    final Composite composite_1 = new Composite(composite, SWT.NONE);
	    final GridLayout gridLayout = new GridLayout();
	    gridLayout.marginHeight = 0;
	    gridLayout.marginWidth = 0;
	    gridLayout.numColumns = 2;
	    composite_1.setLayout(gridLayout);

	    final Label lblDefaultLength = new Label(composite_1, SWT.NONE);
	    lblDefaultLength.setText("Default password length:");

	    spiLength = new Spinner(composite_1, SWT.BORDER);
	    spiLength.setSelection(preferenceStore.getInt(DEFAULT_PASSWORD_LENGTH));

	    btnUseLowercase = new Button(composite, SWT.CHECK);
	    btnUseLowercase.setText("Use lowercase letters");
	    btnUseLowercase.setSelection(preferenceStore.getBoolean(USE_LOWERCASE_LETTERS));

	    btnUserUppercase = new Button(composite, SWT.CHECK);
	    btnUserUppercase.setText("Use UPPERCASE letters");
	    btnUserUppercase.setSelection(preferenceStore.getBoolean(USE_UPPERCASE_LETTERS));

	    btnUseDigits = new Button(composite, SWT.CHECK);
	    btnUseDigits.setText("Use digits");
	    btnUseDigits.setSelection(preferenceStore.getBoolean(USE_DIGITS));

	    btnUseSymbols = new Button(composite, SWT.CHECK);
	    btnUseSymbols.setText("Use symbols (i.e., , %, $, etc.)");
	    btnUseSymbols.setSelection(preferenceStore.getBoolean(USE_SYMBOLS));

	    btnUseEaseToRead = new Button(composite, SWT.CHECK);
	    btnUseEaseToRead.setText("Use only easy-to-read characters (i.e., without \"0\" and \"O\")");
	    btnUseEaseToRead.setSelection(preferenceStore.getBoolean(USE_EASY_TO_READ));
	    
	    btnUseHexOnly = new Button(composite, SWT.CHECK);
	    btnUseHexOnly.setEnabled(false);
	    btnUseHexOnly.setText("Use hexadecimal digits only (0-9,a-f)");
	    btnUseHexOnly.setSelection(preferenceStore.getBoolean(USE_HEX_ONLY));

	    return composite;
	  }

	  /**
	   * Called when user clicks Restore Defaults
	   */
	  protected void performDefaults() {
	    // Get the preference store
	    IPreferenceStore preferenceStore = getPreferenceStore();

	    spiLength.setData(preferenceStore.getDefaultInt(DEFAULT_PASSWORD_LENGTH));
	    btnUseLowercase.setSelection(preferenceStore.getDefaultBoolean(USE_LOWERCASE_LETTERS));
	    btnUserUppercase.setSelection(preferenceStore.getDefaultBoolean(USE_UPPERCASE_LETTERS));  
	    btnUseDigits.setSelection(preferenceStore.getDefaultBoolean(USE_DIGITS));
	    btnUseSymbols.setSelection(preferenceStore.getDefaultBoolean(USE_SYMBOLS));
	    btnUseEaseToRead.setSelection(preferenceStore.getDefaultBoolean(USE_EASY_TO_READ));
	    btnUseHexOnly.setSelection(preferenceStore.getDefaultBoolean(USE_HEX_ONLY));
	    
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
	    preferenceStore.setValue(DEFAULT_PASSWORD_LENGTH, spiLength.getSelection());
	    preferenceStore.setValue(USE_LOWERCASE_LETTERS, btnUseLowercase.getSelection());
	    preferenceStore.setValue(USE_UPPERCASE_LETTERS, btnUserUppercase.getSelection());
	    preferenceStore.setValue(USE_DIGITS, btnUseDigits.getSelection());
	    preferenceStore.setValue(USE_SYMBOLS, btnUseSymbols.getSelection());
	    preferenceStore.setValue(USE_EASY_TO_READ, btnUseEaseToRead.getSelection());
	    preferenceStore.setValue(USE_HEX_ONLY, btnUseHexOnly.getSelection());

	    // Return true to allow dialog to close
	    return true;
	  }
}
