package org.pwsafe.passwordsafeswt.dialog;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Dialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;
import org.pwsafe.passwordsafeswt.util.ShellHelpers;

/**
 * Prompts the user for passwords for a new safe.
 *
 * @author Glen Smith
 */
public class NewSafeDialog extends Dialog {

	private Text txtVerify;
	private Text txtCombination;
	protected Object result;
	protected Shell shell;
	public NewSafeDialog(Shell parent, int style) {
		super(parent, style);
	}
	public NewSafeDialog(Shell parent) {
		this(parent, SWT.NONE);
	}
	public Object open() {
		createContents();
		ShellHelpers.centreShell(getParent(), shell);
		//shell.pack();
		shell.open();
		shell.layout();
		Display display = getParent().getDisplay();
		while (!shell.isDisposed()) {
			if (!display.readAndDispatch())
				display.sleep();
		}
		return result;
	}
	protected void createContents() {
		shell = new Shell(getParent(), SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		shell.setLayout(new FormLayout());
		shell.setSize(380, 250);
		shell.setText("Safe Combination Setup");
		final Label label = new Label(shell, SWT.WRAP);
		final FormData formData = new FormData();
		formData.top = new FormAttachment(0, 5);
		formData.right = new FormAttachment(100, -5);
		formData.left = new FormAttachment(0, 5);
		label.setLayoutData(formData);
		label.setText("A new password database will be created. The safe combination you enter will be used to encrypt the password database file. The safe combination can use any keyboard character and is case sensitive.");

		final Label lblCombination = new Label(shell, SWT.NONE);
		final FormData formData_1 = new FormData();
		formData_1.top = new FormAttachment(label, 30, SWT.BOTTOM);
		formData_1.left = new FormAttachment(10, 0);
		lblCombination.setLayoutData(formData_1);
		lblCombination.setText("Safe Combination:");

		txtCombination = new Text(shell, SWT.PASSWORD | SWT.BORDER);
		final FormData formData_2 = new FormData();
		formData_2.top = new FormAttachment(lblCombination, 0, SWT.TOP);
		formData_2.left = new FormAttachment(lblCombination, 5);
		formData_2.right = new FormAttachment(85, 0);
		txtCombination.setLayoutData(formData_2);

		final Label lblVerify = new Label(shell, SWT.NONE);
		final FormData formData_3 = new FormData();
		formData_3.top = new FormAttachment(lblCombination, 20);
		formData_3.right = new FormAttachment(lblCombination, 0, SWT.RIGHT);
		lblVerify.setLayoutData(formData_3);
		lblVerify.setText("Verify:");

		txtVerify = new Text(shell, SWT.PASSWORD | SWT.BORDER);
		final FormData formData_4 = new FormData();
		formData_4.top = new FormAttachment(lblVerify, 0, SWT.TOP);
		formData_4.left = new FormAttachment(txtCombination, 0, SWT.LEFT);
		formData_4.right = new FormAttachment(txtCombination, 0, SWT.RIGHT);
		txtVerify.setLayoutData(formData_4);

		final Button btnCancel = new Button(shell, SWT.NONE);
		btnCancel.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				shell.dispose();
			}
		});
		final FormData formData_7 = new FormData();
		formData_7.width = 80;
		formData_7.bottom = new FormAttachment(100, -10);
		formData_7.left = new FormAttachment(50, -5);
		btnCancel.setLayoutData(formData_7);
		btnCancel.setText("Cancel");

		final Button btnOk = new Button(shell, SWT.NONE);
		shell.setDefaultButton(btnOk);
		btnOk.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				if (txtCombination.getText().equals(txtVerify.getText())) {
					result = txtCombination.getText(); 
					shell.dispose();
				} else {
					MessageBox mb = new MessageBox(shell, SWT.ICON_ERROR | SWT.OK);
					mb.setText("Password Mismatch");
					mb.setMessage("The two entries do not match");
					mb.open();
				}
			}
		});
		final FormData formData_6 = new FormData();
		formData_6.width = 80;
		formData_6.top = new FormAttachment(btnCancel, 0, SWT.TOP);
		formData_6.right = new FormAttachment(btnCancel, -10, SWT.LEFT);
		btnOk.setLayoutData(formData_6);
		btnOk.setText("OK");

		final Button btnHelp = new Button(shell, SWT.NONE);
		final FormData formData_8 = new FormData();
		formData_8.width = 80;
		formData_8.top = new FormAttachment(btnCancel, 0, SWT.TOP);
		formData_8.left = new FormAttachment(btnCancel, 10, SWT.RIGHT);
		btnHelp.setLayoutData(formData_8);
		btnHelp.setText("Help");
	}
}
