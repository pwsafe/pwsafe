package org.pwsafe.passwordsafeswt.dialog;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Dialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;
import org.pwsafe.passwordsafeswt.util.ShellHelpers;
import org.pwsafe.passwordsafeswt.util.VersionInfo;

import com.swtdesigner.SWTResourceManager;

/**
 * StartupDialog is shown when the app starts up and is modal in front on the main window.
 * 
 * @author Glen Smith
 */
public class StartupDialog extends Dialog {

	private Combo cboFilename;
	private Text txtPassword;
	protected Object result;
	protected Shell shell;
	private String[] mruList;
	
	private String selectedFile;
	private String selectedPassword;
	
	public static final String OPEN_FILE = "open-selected";  // open the selected file
	public static final String OPEN_OTHER = "open-other"; // open file dialog for other file
	public static final String NEW_FILE = "new";    // create a new safe
	public static final String CANCEL = "cancel";   // exit the app
	
	public StartupDialog(Shell parent, int style) {
		super(parent, style);
	}
	
	public StartupDialog(Shell parent, String[] mru) {
		this(parent, SWT.NONE);
		this.mruList = mru;
	}
	
	public Object open() {
	    result = StartupDialog.CANCEL;
		createContents();
		ShellHelpers.centreShell(getParent(), shell);
		shell.open();
		shell.layout();
		if (mruList != null) {
			for (int i=0; i < mruList.length; i++) {
				cboFilename.add(mruList[i]);
			}
			cboFilename.setText(mruList[0]);
		}
		Display display = getParent().getDisplay();
		while (!shell.isDisposed()) {
			if (!display.readAndDispatch())
				display.sleep();
		}
		return result;
	}
	
	/**
	 * Create dialog elements.
	 */
	protected void createContents() {
		shell = new Shell(getParent(), SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		shell.setLayout(new FormLayout());
		shell.setImage(SWTResourceManager.getImage(StartupDialog.class, "/org/pwsafe/passwordsafeswt/images/clogo.gif"));
		shell.setSize(550, 368);
		shell.setText("Safe Combination Entry");

		final Label lblTextLogo = new Label(shell, SWT.NONE);
		lblTextLogo.setAlignment(SWT.CENTER);
		lblTextLogo.setImage(SWTResourceManager.getImage(StartupDialog.class, "/org/pwsafe/passwordsafeswt/images/psafetxtNew.gif"));
		final FormData formData_10 = new FormData();
		formData_10.left = new FormAttachment(24, 0);
		formData_10.top = new FormAttachment(0, 15);
		lblTextLogo.setLayoutData(formData_10);
		
		final Label lblPleaseEnter = new Label(shell, SWT.NONE);
		final FormData formData = new FormData();
		//formData.top = new FormAttachment(0, 55);
		formData.top = new FormAttachment(lblTextLogo, 15);
		formData.left = new FormAttachment(0, 55);
		lblPleaseEnter.setLayoutData(formData);
		lblPleaseEnter.setText("Please enter the safe combination for the password database:");

		final Label lblFilename = new Label(shell, SWT.NONE);
		lblFilename.setText("&Filename:");
		final FormData formData_1 = new FormData();
		formData_1.top = new FormAttachment(lblPleaseEnter, 15, SWT.BOTTOM);
		formData_1.left = new FormAttachment(lblPleaseEnter, 15, SWT.LEFT);
		lblFilename.setLayoutData(formData_1);

		cboFilename = new Combo(shell, SWT.READ_ONLY);
		final FormData formData_1b = new FormData();
		formData_1b.top = new FormAttachment(lblFilename, 0, SWT.TOP);
		formData_1b.left = new FormAttachment(lblFilename, 15, SWT.RIGHT);
		formData_1b.right = new FormAttachment(100, -170);
		cboFilename.setLayoutData(formData_1b);
		
		final Label lblSafeCombination = new Label(shell, SWT.NONE);
		final FormData formData_2 = new FormData();
		formData_2.top = new FormAttachment(lblFilename, 20);
		formData_2.right = new FormAttachment(lblFilename, 0, SWT.RIGHT);
		lblSafeCombination.setLayoutData(formData_2);
		lblSafeCombination.setText("&Safe Combination:");

		txtPassword = new Text(shell, SWT.BORDER);
		txtPassword.setEchoChar('*');
		final FormData formData_3 = new FormData();
		formData_3.top = new FormAttachment(lblSafeCombination, 0, SWT.TOP);
		formData_3.left = new FormAttachment(cboFilename, 0, SWT.LEFT);
        formData_3.right = new FormAttachment(cboFilename, 0, SWT.RIGHT);
		txtPassword.setLayoutData(formData_3);

		final Button btnReadOnly = new Button(shell, SWT.CHECK);
		final FormData formData_4 = new FormData();
		formData_4.top = new FormAttachment(txtPassword, 15);
		formData_4.left = new FormAttachment(txtPassword, 0, SWT.LEFT);
		btnReadOnly.setLayoutData(formData_4);
		btnReadOnly.setText("Open as &read-only");

		final Button btnCreate = new Button(shell, SWT.NONE);
		btnCreate.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				result = StartupDialog.NEW_FILE;
				shell.dispose();
			}
		});
		final FormData formData_5 = new FormData();
        formData_5.top = new FormAttachment(cboFilename, 0, SWT.TOP);
		formData_5.right = new FormAttachment(100, -5);
		btnCreate.setLayoutData(formData_5);
		btnCreate.setText("&Create new safe...  ");

		final Button btnOpen = new Button(shell, SWT.NONE);
		btnOpen.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				result = StartupDialog.OPEN_OTHER;
				selectedFile = cboFilename.getText();
				selectedPassword = txtPassword.getText();
				shell.dispose();
			}
		});
		final FormData formData_6 = new FormData();
		formData_6.top = new FormAttachment(btnCreate, 10);
		formData_6.left = new FormAttachment(btnCreate, 0, SWT.LEFT);
        formData_6.right = new FormAttachment(btnCreate, 0, SWT.RIGHT);
		btnOpen.setLayoutData(formData_6);
		btnOpen.setText("&Open other safe...");

		final Button btnOk = new Button(shell, SWT.NONE);
		shell.setDefaultButton(btnOk);
		btnOk.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				selectedFile = cboFilename.getText();
				selectedPassword = txtPassword.getText();
				result = StartupDialog.OPEN_FILE;
				shell.dispose();
			}
		});
		final FormData formData_7 = new FormData();
		formData_7.width = 80;
		formData_7.left = new FormAttachment(50, -80);
		formData_7.bottom = new FormAttachment(100, -10);
		btnOk.setLayoutData(formData_7);
		btnOk.setText("OK");

		final Button btnCancel = new Button(shell, SWT.NONE);
		btnCancel.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				result = StartupDialog.CANCEL;
				shell.dispose();
			}
		});
		final FormData formData_8 = new FormData();
		formData_8.width = 80;
		formData_8.left = new FormAttachment(btnOk, 10);
		formData_8.top = new FormAttachment(btnOk, 0, SWT.TOP);
		btnCancel.setLayoutData(formData_8);
		btnCancel.setText("Cancel");

		final Button btnHelp = new Button(shell, SWT.NONE);
		final FormData formData_9 = new FormData();
		formData_9.width = 80;
		formData_9.top = new FormAttachment(btnCancel, 0, SWT.TOP);		
		formData_9.left = new FormAttachment(btnCancel, 10);
		btnHelp.setLayoutData(formData_9);
		btnHelp.setText("Help");
		btnHelp.setEnabled(false);    // there is no help yet

		final Label lblVersion = new Label(shell, SWT.NONE);
		final FormData formData_11 = new FormData();
		formData_11.top = new FormAttachment(btnReadOnly, 0, SWT.TOP);
		formData_11.left = new FormAttachment(btnOpen, 0, SWT.LEFT);
		lblVersion.setLayoutData(formData_11);
		lblVersion.setText("Version: " + VersionInfo.getVersion());
	}
	
	/**
	 * Returns the filename selected in the dialog.
	 * 
	 * @return the filename selected in the dialog
	 */
	public String getFilename() {
		return selectedFile;
	}
	
	/**
	 * Returns the password entered in the dialog.
	 * 
	 * @return the password entered in the dialog
	 */
	public String getPassword() {
		return selectedPassword;
	}
}
