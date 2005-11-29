package org.pwsafe.passwordsafeswt.dialog;

import java.util.Random;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.KeyAdapter;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Dialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;
import org.pwsafe.passwordsafeswt.preference.DisplayPreferences;
import org.pwsafe.passwordsafeswt.preference.PasswordPolicyPreferences;
import org.pwsafe.passwordsafeswt.preference.SecurityPreferences;
import org.pwsafe.passwordsafeswt.util.ShellHelpers;
import org.pwsafe.passwordsafeswt.util.UserPreferences;

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
		shell.setSize(530, 340);
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

		final Composite compositeLabel = new Composite(shell, SWT.NONE);
		final GridData gridData = new GridData(GridData.HORIZONTAL_ALIGN_FILL);
		gridData.widthHint = 499;
		compositeLabel.setLayoutData(gridData);
		compositeLabel.setLayout(new GridLayout());

		final Label labelInfo = new Label(compositeLabel, SWT.WRAP);
		final GridData gridData_1 = new GridData(GridData.HORIZONTAL_ALIGN_FILL | GridData.VERTICAL_ALIGN_FILL);
		gridData_1.widthHint = 499;
		labelInfo.setLayoutData(gridData_1);
		labelInfo.setText("To edit this entry from the current password file, simply make the desired changes in the fields below. Note that at least a title and a password are still required.");

		final Composite compositeFields = new Composite(shell, SWT.NONE);
		compositeFields.setLayout(new FormLayout());
		final GridData gridData_c = new GridData(GridData.FILL_BOTH);
		gridData_c.widthHint = 499;
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
		formData_1.right = new FormAttachment(45, 0);
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
		btnShowPassword.setText("Show Password");

		final Label lblNotes = new Label(compositeFields, SWT.NONE);
		final FormData formData_9 = new FormData();
		formData_9.top = new FormAttachment(txtPassword, 5, SWT.BOTTOM);
		formData_9.left = new FormAttachment(lblPassword, 0, SWT.LEFT);
		lblNotes.setLayoutData(formData_9);
		lblNotes.setText("Notes:");

		txtNotes = new Text(compositeFields, SWT.V_SCROLL | SWT.MULTI | SWT.BORDER | SWT.WRAP);
		final FormData formData_10 = new FormData();
		formData_10.bottom = new FormAttachment(100, -5);
		formData_10.top = new FormAttachment(txtPassword, 5, SWT.BOTTOM);
		formData_10.right = new FormAttachment(btnShowPassword, 0, SWT.RIGHT);
		formData_10.left = new FormAttachment(txtPassword, 0, SWT.LEFT);
		txtNotes.setLayoutData(formData_10);
		txtNotes.addKeyListener(dirtyKeypress);
        if (entryToEdit.getNotes() != null)
            txtNotes.setText(entryToEdit.getNotes());

		final Button btnOk = new Button(compositeFields, SWT.NONE);
		shell.setDefaultButton(btnOk);
		btnOk.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
                if (isDirty()) {
                    entryToEdit.setGroup(txtGroup.getText());
                    entryToEdit.setTitle(txtTitle.getText());
                    entryToEdit.setUsername(txtUsername.getText());
                    entryToEdit.setPassword(txtPassword.getText());
                    entryToEdit.setNotes(txtNotes.getText());
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
		formData_14.left = new FormAttachment(txtNotes, 10);
		formData_14.top = new FormAttachment(btnShowPassword, 5, SWT.BOTTOM);
		formData_14.right = new FormAttachment(100, 0);
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
	}
	
	private String generatePassword() {
		String BASE_LETTERS = "abcdefghijklmnopqrstuvwxyz";
		String BASE_DIGITS = "1234567890";
		String BASE_SYMBOLS = "!@#$%^&*()";
		StringBuffer pwSet = new StringBuffer();
		
		UserPreferences.reload(); // make sure we have a fresh copy
		UserPreferences preferenceStore = UserPreferences.getInstance();
		
		String passwordLengthStr = preferenceStore.getString(PasswordPolicyPreferences.DEFAULT_PASSWORD_LENGTH);
		int passwordLength = 0;
		if (passwordLengthStr != null) {
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
			pwSet.append(BASE_LETTERS.toLowerCase());
		}
		
		if (useUpperCase) {
			pwSet.append(BASE_LETTERS.toUpperCase());
		}
		
		if (useDigits) {
			pwSet.append(BASE_DIGITS);
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
}
