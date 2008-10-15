package org.pwsafe.passwordsafeswt.dialog;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CLabel;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;
import org.pwsafe.passwordsafeswt.preference.DisplayPreferences;
import org.pwsafe.passwordsafeswt.preference.PasswordPolicyPreferences;
import org.pwsafe.passwordsafeswt.util.ShellHelpers;
import org.pwsafe.passwordsafeswt.util.UserPreferences;

import java.text.DateFormat;
import java.text.ParseException;
import java.util.Calendar;
import java.util.Date;
import java.util.Random;

/**
 * The Dialog that allows a user to edit password entries.
 *
 * @author Glen Smith
 */
public class EditDialog extends Dialog {

	private static final Log log = LogFactory.getLog(EditDialog.class);
	
	private Text txtNotes;
	private Text txtPassword;
	private Text txtUsername;
	private Text txtTitle;
	private Text txtGroup;
	private Text txtUrl;
	private Text txtAutotype;
	private CLabel passwordChange;
	private CLabel changed;
	private CLabel lastAccess;
	private CLabel createTime;
	private Text txtPasswordExpire;
    private DateTime dtPasswordExpire;
    private boolean dirty;
	protected Object result;
	protected Shell shell;
    private PwsEntryDTO entryToEdit;
    
	public EditDialog(Shell parent, int style, PwsEntryDTO entryToEdit) {
		super(parent, style);
        this.entryToEdit = entryToEdit;
	}
	public EditDialog(Shell parent, PwsEntryDTO entryToEdit) {
		this(parent, SWT.NONE, entryToEdit);
	}
    
	public Object open() {
		createContents();
		ShellHelpers.centreShell(getParent(), shell);
		shell.pack();
		shell.open();
		shell.layout();
		Display display = getParent().getDisplay();
		while (!shell.isDisposed()) {
			if (!display.readAndDispatch())
				display.sleep();
		}
		return result;
	}
    
	/**
	 * Returns whether the data in the dialog has been updated by the user.
	 * 
	 * @return true if the data has been updated, false otherwise
	 */
    public boolean isDirty() {
    	return dirty;
    }
    
    /**
     * Marks the dialog as having data that needs to be updated.
     * 
     * @param dirty true if the dialog data needs saving, false otherwise.
     */
    public void setDirty(boolean dirty) {
    	this.dirty = dirty;
    }
    

	protected void createContents() {
		shell = new Shell(getParent(), SWT.RESIZE | SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		shell.setSize(590, 603);
		shell.setText("Edit/View Entry");
		final GridLayout gridLayout_2 = new GridLayout();
		gridLayout_2.marginWidth = 10;
		gridLayout_2.marginHeight = 10;
		shell.setLayout(gridLayout_2);
		
		// Setup adapter to catch any keypress and mark dialog dirty
		KeyAdapter dirtyKeypress = new KeyAdapter() {
			public void keyPressed(KeyEvent e) {
				setDirty(true);
			}
		};
		
		//use a modify listener as the password field drops letter key events on Linux
		ModifyListener entryEdited = new ModifyListener() {

			public void modifyText(ModifyEvent e) {
				setDirty(true);				
			}
			
		};

		final Composite compositeLabel = new Composite(shell, SWT.NONE);
		final GridData gridData = new GridData(GridData.HORIZONTAL_ALIGN_FILL);
		gridData.widthHint = 550;
		compositeLabel.setLayoutData(gridData);
		compositeLabel.setLayout(new GridLayout());

		final Label labelInfo = new Label(compositeLabel, SWT.WRAP);
		final GridData gridData_1 = new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.VERTICAL_ALIGN_FILL);
		gridData_1.widthHint = 550;

		labelInfo.setLayoutData(gridData_1);
		labelInfo.setText("To edit this entry from the current password file, simply make the desired changes in the fields below. Note that at least a title and a password are still required.");

		final Composite compositeFields = new Composite(shell, SWT.NONE);
		compositeFields.setLayout(new FormLayout());
		final GridData gridData_c = new GridData(SWT.FILL, SWT.FILL, true, true);
		gridData_c.widthHint = 550;
		compositeFields.setLayoutData(gridData_c);

		final Label lblGroup = new Label(compositeFields, SWT.NONE);
		final FormData formData = new FormData();
		formData.top = new FormAttachment(0, 10);
		formData.left = new FormAttachment(0, 17);
		lblGroup.setLayoutData(formData);
		lblGroup.setText("Group:");

		txtGroup = new Text(compositeFields, SWT.BORDER);
		txtGroup.addKeyListener(dirtyKeypress);
		final FormData formData_1 = new FormData();
		formData_1.left = new FormAttachment(lblGroup, 30);
		formData_1.top = new FormAttachment(lblGroup, 0, SWT.TOP);
		formData_1.right = new FormAttachment(43, 0);
		txtGroup.setLayoutData(formData_1);
        if (entryToEdit.getGroup() != null)
            txtGroup.setText(entryToEdit.getGroup());

		final Label lblTitle = new Label(compositeFields, SWT.NONE);
		final FormData formData_2 = new FormData();
		formData_2.top = new FormAttachment(txtGroup, 10, SWT.BOTTOM);
		formData_2.left = new FormAttachment(lblGroup, 0, SWT.LEFT);
		lblTitle.setLayoutData(formData_2);
		lblTitle.setText("Title:");

		txtTitle = new Text(compositeFields, SWT.BORDER);
		final FormData formData_3 = new FormData();
		formData_3.top = new FormAttachment(txtGroup, 10, SWT.BOTTOM);
		formData_3.left = new FormAttachment(txtGroup, 0, SWT.LEFT);
		formData_3.right = new FormAttachment(txtGroup, 0 , SWT.RIGHT);
		txtTitle.setLayoutData(formData_3);
		txtTitle.addKeyListener(dirtyKeypress);
        if (entryToEdit.getTitle() != null)
            txtTitle.setText(entryToEdit.getTitle());

		final Label lblUsername = new Label(compositeFields, SWT.NONE);
		final FormData formData_4 = new FormData();
		formData_4.top = new FormAttachment(txtTitle, 10, SWT.BOTTOM);
		formData_4.left = new FormAttachment(lblTitle, 0, SWT.LEFT);
		lblUsername.setLayoutData(formData_4);
		lblUsername.setText("Username:");

		txtUsername = new Text(compositeFields, SWT.BORDER);
		final FormData formData_5 = new FormData();
		formData_5.top = new FormAttachment(txtTitle, 10);
		formData_5.left = new FormAttachment(txtTitle, 0, SWT.LEFT);
		formData_5.right = new FormAttachment(txtTitle, 0 , SWT.RIGHT);
		txtUsername.setLayoutData(formData_5);
		txtUsername.addKeyListener(dirtyKeypress);
        if (entryToEdit.getUsername() != null)
            txtUsername.setText(entryToEdit.getUsername());

		final Label lblPassword = new Label(compositeFields, SWT.NONE);
		final FormData formData_6 = new FormData();
		formData_6.top = new FormAttachment(txtUsername, 10, SWT.BOTTOM);
		formData_6.left = new FormAttachment(lblUsername, 0, SWT.LEFT);
		lblPassword.setLayoutData(formData_6);
		lblPassword.setText("Password:");

		txtPassword = new Text(compositeFields, SWT.BORDER);
		final FormData formData_7 = new FormData();
		formData_7.top = new FormAttachment(txtUsername, 10, SWT.BOTTOM);
		formData_7.left = new FormAttachment(txtUsername, 0, SWT.LEFT);
		formData_7.right = new FormAttachment(txtUsername, 0 , SWT.RIGHT);
		txtPassword.setLayoutData(formData_7);
		txtPassword.addKeyListener(dirtyKeypress);
		if (!UserPreferences.getInstance().getBoolean(DisplayPreferences.SHOW_PASSWORD_IN_EDIT_MODE)) {
        txtPassword.setEchoChar('*');
		}
        if (entryToEdit.getPassword() != null)
            txtPassword.setText(entryToEdit.getPassword());
		txtPassword.addModifyListener(entryEdited);// important: add after setting content

		final Button btnShowPassword = new Button(compositeFields, SWT.NONE);
		btnShowPassword.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
                if (txtPassword.getEchoChar() != '\0') {
                	txtPassword.setEchoChar('\0');
                	btnShowPassword.setText("Hide Password");
                } else {
                	btnShowPassword.setText("Show Password");
                	txtPassword.setEchoChar('*');
                }
			}
		});
		final FormData formData_8 = new FormData();
		formData_8.left = new FormAttachment(txtPassword, 10);
		formData_8.top = new FormAttachment(txtUsername, 10, SWT.BOTTOM);
		formData_8.right = new FormAttachment(70, 0);
		btnShowPassword.setLayoutData(formData_8);
		if (UserPreferences.getInstance().getBoolean(DisplayPreferences.SHOW_PASSWORD_IN_EDIT_MODE)) {
			btnShowPassword.setText("Hide Password");
		} else {
			btnShowPassword.setText("Show Password");
		}
		
		final Label lblNotes = new Label(compositeFields, SWT.NONE);
		final FormData formData_9 = new FormData();
		formData_9.top = new FormAttachment(txtPassword, 5, SWT.BOTTOM);
		formData_9.left = new FormAttachment(lblPassword, 0, SWT.LEFT);
		lblNotes.setLayoutData(formData_9);
		lblNotes.setText("Notes:");

		txtNotes = new Text(compositeFields, SWT.V_SCROLL | SWT.MULTI | SWT.BORDER | SWT.WRAP);
		final FormData formData_10 = new FormData(SWT.DEFAULT, 100);
		formData_10.bottom = new FormAttachment(100, -112);
		formData_10.top = new FormAttachment(txtPassword, 5, SWT.BOTTOM);
		formData_10.right = new FormAttachment(btnShowPassword, 0, SWT.RIGHT);
		formData_10.left = new FormAttachment(txtPassword, 0, SWT.LEFT);

		txtNotes.setLayoutData(formData_10);
		txtNotes.addKeyListener(dirtyKeypress);
        if (entryToEdit.getNotes() != null)
            txtNotes.setText(entryToEdit.getNotes());

        // New fields for V3 Files
		final Label lblUrl = new Label(compositeFields, SWT.NONE);
		FormData formDataTemp = new FormData();
		formDataTemp.top = new FormAttachment(txtNotes, 10, SWT.BOTTOM);
		formDataTemp.left = new FormAttachment(lblNotes, 0, SWT.LEFT);
		lblUrl.setLayoutData(formDataTemp);
		lblUrl.setText("URL:");

		txtUrl = new Text(compositeFields, SWT.BORDER);
		formDataTemp = new FormData();
		formDataTemp.top = new FormAttachment(txtNotes, 10, SWT.BOTTOM);
		formDataTemp.left = new FormAttachment(txtNotes, 0, SWT.LEFT);
		formDataTemp.right = new FormAttachment(txtNotes, 0 , SWT.RIGHT);
		txtUrl.setLayoutData(formDataTemp);
		txtUrl.addKeyListener(dirtyKeypress);
        if (entryToEdit.getUrl() != null)
    		txtUrl.setText(entryToEdit.getUrl());

		final Label lblAutotype = new Label(compositeFields, SWT.NONE);
		formDataTemp = new FormData();
		formDataTemp.top = new FormAttachment(txtUrl, 10, SWT.BOTTOM);
		formDataTemp.left = new FormAttachment(lblUrl, 0, SWT.LEFT);
		lblAutotype.setLayoutData(formDataTemp);
		lblAutotype.setText("Autotype:");

		txtAutotype = new Text(compositeFields, SWT.BORDER);
		formDataTemp = new FormData();
		formDataTemp.top = new FormAttachment(txtUrl, 10, SWT.BOTTOM);
		formDataTemp.left = new FormAttachment(txtUrl, 0, SWT.LEFT);
		formDataTemp.right = new FormAttachment(txtPassword, 0 , SWT.RIGHT);
//		formDataTemp.bottom = new FormAttachment(100, -5);
		txtAutotype.setLayoutData(formDataTemp);
		txtAutotype.addKeyListener(dirtyKeypress);
        if (entryToEdit.getAutotype() != null)
    		txtAutotype.setText(entryToEdit.getAutotype());

		final Label lblPasswordExpire = new Label(compositeFields, SWT.NONE);
		final FormData fd_lblPasswordExpire = new FormData();
		fd_lblPasswordExpire.top = new FormAttachment(txtAutotype, 10, SWT.BOTTOM);
		fd_lblPasswordExpire.left = new FormAttachment(lblAutotype, 0, SWT.LEFT);
		lblPasswordExpire.setLayoutData(fd_lblPasswordExpire);
		lblPasswordExpire.setText("Password expires:");

		txtPasswordExpire = new Text(compositeFields, SWT.BORDER);
		final FormData fd_txtPasswordExpire = new FormData();
		fd_txtPasswordExpire.left = new FormAttachment(lblPasswordExpire, 0, SWT.RIGHT);
		fd_txtPasswordExpire.right = new FormAttachment(txtAutotype, 0, SWT.RIGHT);
		fd_txtPasswordExpire.top = new FormAttachment(txtAutotype, 10, SWT.BOTTOM);
		txtPasswordExpire.setLayoutData(fd_txtPasswordExpire);
		txtPasswordExpire.setText(format(entryToEdit.getExpires()));
		txtPasswordExpire.addKeyListener(dirtyKeypress);

//        dtPasswordExpire = new DateTime(compositeFields, SWT.DATE | SWT.MEDIUM);
//        final FormData fd_dtPasswordExpire = new FormData();
//        fd_dtPasswordExpire.left = new FormAttachment(txtPasswordExpire, 10, SWT.RIGHT);
//        fd_dtPasswordExpire.top = new FormAttachment(txtAutotype, 0, SWT.BOTTOM);
//        dtPasswordExpire.setLayoutData(fd_dtPasswordExpire);
//        dtPasswordExpire.addKeyListener(dirtyKeypress);

        addDateChooser (compositeFields);
        
        shell.setDefaultButton(createButtons(compositeFields, btnShowPassword));

		createTimesComposite(shell);
	}
	
	private void addDateChooser(Composite compositeFields) {
		Button open = new Button (compositeFields, SWT.PUSH);
		final FormData fd_dtPasswordExpire = new FormData();
		fd_dtPasswordExpire.left = new FormAttachment(txtPasswordExpire, 10, SWT.RIGHT);
		fd_dtPasswordExpire.top = new FormAttachment(txtPasswordExpire, 0, SWT.TOP);
		open.setLayoutData(fd_dtPasswordExpire);
		open.setText ("Calendar");
		open.addSelectionListener (new SelectionAdapter () {
			public void widgetSelected (SelectionEvent e) {
				DateDialog dialog = new DateDialog(shell);
				dialog.setDate(entryToEdit.getExpires());
				Date result = dialog.open();
				if (result != null && ! result.equals(entryToEdit.getExpires())) {
					txtPasswordExpire.setText(format(result));
					setDirty(true);
				}
			}
		});
	}
	
	/**
	 * Creates the controlling buttons on the dialog
	 * @param compositeFields
	 * @param btnShowPassword
	 * return the default button
	 */
	private Button createButtons(final Composite compositeFields, final Button btnShowPassword) {
		final Button btnOk = new Button(compositeFields, SWT.NONE);
		btnOk.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
                if (isDirty()) {
                	final Date now = new Date();
                	entryToEdit.setLastChange(now);
					entryToEdit.setLastAccess(now);
                    entryToEdit.setGroup(txtGroup.getText());
                    entryToEdit.setTitle(txtTitle.getText());
                    entryToEdit.setUsername(txtUsername.getText());
                    if (! txtPassword.getText().equals(entryToEdit.getPassword())) {
                    	entryToEdit.setPassword(txtPassword.getText());
                    	entryToEdit.setLastPwChange(now);
                    }
                    entryToEdit.setNotes(txtNotes.getText());
                    String fieldText = txtPasswordExpire.getText();
					try {
						Date expireDate = DateFormat.getDateInstance().parse(fieldText);
						Calendar cal = Calendar.getInstance();
						cal.setTime(expireDate);
						int year = cal.get(Calendar.YEAR);
						if (year < 2000) { 
							if (year < 100) 
								year += 2000; // avoid years like 07 passing as 0007 (Linux / DE)
							else
								year += 100; // avoid years like 07 passing as 1907 (Win / US)
							cal.set(Calendar.YEAR, year);
							expireDate = cal.getTime();
						}
		
						entryToEdit.setExpires(expireDate);
					} catch (ParseException e1) {
			            MessageBox mb = new MessageBox(shell, SWT.ICON_WARNING | SWT.YES | SWT.NO);
			            mb.setText("Expiry date not valid");
			            mb.setMessage("The password expiry date is not valid and will be ignored - continue anyway?");
			            int result = mb.open();
			            if (result == SWT.NO) {
			                return;
			            }

					}
					entryToEdit.setUrl(txtUrl.getText());
                    entryToEdit.setAutotype(txtAutotype.getText());
                    result = entryToEdit;   
                } else {
                	result = null;
                }
                shell.dispose();
			}
		});
		final FormData formData_11 = new FormData();
		formData_11.top = new FormAttachment(txtGroup, 0, SWT.TOP);
		formData_11.left = new FormAttachment(100,-80);
		formData_11.right = new FormAttachment(100, -10);
		btnOk.setLayoutData(formData_11);
		btnOk.setText("OK");

		final Button btnCancel = new Button(compositeFields, SWT.NONE);
		btnCancel.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
                result = null;
                shell.dispose();
			}
		});
		final FormData formData_12 = new FormData();
		formData_12.top = new FormAttachment(btnOk, 5);
		formData_12.left = new FormAttachment(btnOk, 0, SWT.LEFT);
		formData_12.right = new FormAttachment(btnOk, 0, SWT.RIGHT);
		btnCancel.setLayoutData(formData_12);
		btnCancel.setText("Cancel");

		final Button btnHelp = new Button(compositeFields, SWT.NONE);
		final FormData formData_13 = new FormData();
		formData_13.top = new FormAttachment(btnCancel, 5);
		formData_13.left = new FormAttachment(btnCancel, 0, SWT.LEFT);
		formData_13.right = new FormAttachment(btnCancel, 0, SWT.RIGHT);
		btnHelp.setLayoutData(formData_13);
		btnHelp.setText("Help");

		final Group group = new Group(compositeFields, SWT.NONE);
		group.setLayout(new GridLayout());
		group.setText("Random Password");
		final FormData formData_14 = new FormData();
//		formData_14.left = new FormAttachment(txtNotes, 10, SWT.RIGHT);
		formData_14.left = new FormAttachment(100, -160);
		formData_14.top = new FormAttachment(btnShowPassword, 5, SWT.TOP);
		formData_14.right = new FormAttachment(100, -10);
		group.setLayoutData(formData_14);

		final Button btnGenerate = new Button(group, SWT.NONE);
		btnGenerate.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				String generatedPassword = generatePassword();
				txtPassword.setText(generatedPassword);
			}
		});
		btnGenerate.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL));
		btnGenerate.setText("Generate");

		final Button chkOverride = new Button(group, SWT.CHECK);
		chkOverride.setText("Override Policy");
		chkOverride.setEnabled(false); //TODO: Open policy dialog and generate a password with it on exit
		
		return btnOk;
	}
	
	/**
	 * Creates a line showing change information about the record. 
	 * @param aShell to Add the Composite to
	 */
	private void createTimesComposite(final Shell aShell) {
		final GridData gridData = new GridData(GridData.HORIZONTAL_ALIGN_FILL);
		final Composite timesGroup = new Composite(aShell, SWT.NONE);
		timesGroup.setRedraw(true);
		final GridData timesGridData = new GridData(SWT.FILL, SWT.CENTER, true, false);
		gridData.widthHint = 550;
		timesGroup.setLayoutData(timesGridData);
		final GridLayout gridLayout = new GridLayout();
		gridLayout.numColumns = 8;
		timesGroup.setLayout(gridLayout);


		final CLabel createdLbl = new CLabel(timesGroup, SWT.NONE);
		createdLbl.setText("Created");

		createTime = new CLabel(timesGroup, SWT.NONE);			
		createTime.setText(format(entryToEdit.getCreated()));

		final CLabel lastAccessLbl = new CLabel(timesGroup, SWT.NONE);
		lastAccessLbl.setText("Last Access");

		lastAccess = new CLabel(timesGroup, SWT.NONE);
		lastAccess.setText(format(entryToEdit.getLastAccess()));

		final CLabel changedLbl = new CLabel(timesGroup, SWT.NONE);
		changedLbl.setText("Changed");

		changed = new CLabel(timesGroup, SWT.NONE);
		changed.setText(format(entryToEdit.getLastChange()));

		final CLabel passwordChangeLbl = new CLabel(timesGroup, SWT.NONE);
		passwordChangeLbl.setText("Password Change");

		passwordChange = new CLabel(timesGroup, SWT.NONE);
		passwordChange.setText(format(entryToEdit.getLastPwChange()));
	}
	
	private String generatePassword() {
		String BASE_LETTERS = "abcdefghijklmnopqrstuvwxyz";
        String BASE_LETTERS_EASY = "abcdefghjkmnpqrstuvwxyz";
		String BASE_DIGITS = "1234567890";
        String BASE_DIGITS_EASY = "23456789";
		String BASE_SYMBOLS = "!@#$%^&*()";
		StringBuffer pwSet = new StringBuffer();
		
		UserPreferences.reload(); // make sure we have a fresh copy
		UserPreferences preferenceStore = UserPreferences.getInstance();
		
		String passwordLengthStr = preferenceStore.getString(PasswordPolicyPreferences.DEFAULT_PASSWORD_LENGTH);
		int passwordLength = 0;
		if (passwordLengthStr != null && passwordLengthStr.trim().length() > 0) {
			passwordLength = Integer.parseInt(passwordLengthStr);
		}
		if (passwordLength <= 0)
			passwordLength = 8; //let's be sensible about this..
		
		boolean useLowerCase = preferenceStore.getBoolean(PasswordPolicyPreferences.USE_LOWERCASE_LETTERS);
		boolean useUpperCase = preferenceStore.getBoolean(PasswordPolicyPreferences.USE_UPPERCASE_LETTERS);
		boolean useDigits = preferenceStore.getBoolean(PasswordPolicyPreferences.USE_DIGITS);
		boolean useSymbols = preferenceStore.getBoolean(PasswordPolicyPreferences.USE_SYMBOLS);
		boolean useEasyToRead = preferenceStore.getBoolean(PasswordPolicyPreferences.USE_EASY_TO_READ);
		
		if (useLowerCase) {
			if (useEasyToRead) {
                pwSet.append(BASE_LETTERS_EASY.toLowerCase());
            } else {
                pwSet.append(BASE_LETTERS.toLowerCase());
            }
		}

		if (useUpperCase) {
            if (useEasyToRead) {
                pwSet.append(BASE_LETTERS_EASY.toUpperCase());
            } else {
                pwSet.append(BASE_LETTERS.toUpperCase());
            }
		}

		if (useDigits) {
            if (useEasyToRead) {
                pwSet.append(BASE_DIGITS_EASY);
            } else {
                pwSet.append(BASE_DIGITS);
            }
		}

		
		if (useSymbols) {
			pwSet.append(BASE_SYMBOLS);
		}
		
		
		StringBuffer sb = new StringBuffer();
		if (pwSet.length() > 0) {
			Random rand = new Random(System.currentTimeMillis());
			for (int i = 0; i < passwordLength; i++) {
				int randOffset = rand.nextInt(pwSet.length());
				sb.append(pwSet.charAt(randOffset));
			}
		} else {
			sb.append("Must Edit Password Generation Options");
		}

		return sb.toString();

	}
	
	private String format (Date aDate) {
		if (aDate != null)
			return DateFormat.getDateInstance().format(aDate);
		else
			return "";
	}
}
