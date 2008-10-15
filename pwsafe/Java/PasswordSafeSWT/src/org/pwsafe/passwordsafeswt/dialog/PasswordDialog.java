package org.pwsafe.passwordsafeswt.dialog;

import java.io.File;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Dialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;
import org.pwsafe.passwordsafeswt.util.ShellHelpers;

import com.swtdesigner.SWTResourceManager;

/**
 * PasswordDialog accepts password (and confirmation password) to either open a safe
 * or change a combination.
 * 
 * @author Glen Smith
 */
public class PasswordDialog extends Dialog {

	private Label lblFilename;
	protected Object result;
	protected Shell shell; 
    private String fileName;

	public PasswordDialog(Shell parent, int style) {
		super(parent, style);
	}
	public PasswordDialog(Shell parent) {
		this(parent, SWT.NONE);
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
	protected void createContents() {
		shell = new Shell(getParent(), SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
		final GridLayout gridLayout = new GridLayout();
		gridLayout.numColumns = 2;
		shell.setLayout(gridLayout);
		shell.setSize(500, 172);
		shell.setText("Safe Combination Entry");

		final Composite composite_1 = new Composite(shell, SWT.NONE);
		composite_1.setLayout(new GridLayout());

		final Label lblLogo = new Label(composite_1, SWT.NONE);
		lblLogo.setImage(SWTResourceManager.getImage(PasswordDialog.class, "/org/pwsafe/passwordsafeswt/images/clogo.gif"));

		final Composite composite = new Composite(shell, SWT.NONE);
		composite.setLayoutData(new GridData(GridData.FILL_HORIZONTAL | GridData.GRAB_VERTICAL | GridData.VERTICAL_ALIGN_BEGINNING));
		composite.setLayout(new GridLayout());

		final Label lblEnterPassword = new Label(composite, SWT.NONE);
		lblEnterPassword.setText("Please enter the safe combination for this password database");

		lblFilename = new Label(composite, SWT.NONE);
		lblFilename.setText(new File(fileName).getName());
		lblFilename.setToolTipText(fileName);

		final Composite composite_2 = new Composite(composite, SWT.NONE);
		composite_2.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
		final GridLayout gridLayout_1 = new GridLayout();
		gridLayout_1.numColumns = 2;
		composite_2.setLayout(gridLayout_1);

		final Label lblCombination = new Label(composite_2, SWT.NONE);
		lblCombination.setText("Safe Combination:");

		final Text txtCombination = new Text(composite_2, SWT.BORDER);
		txtCombination.setEchoChar('*');
		txtCombination.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));

		final Composite composite_3 = new Composite(composite, SWT.NONE);
		composite_3.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_END));
		final GridLayout gridLayout_2 = new GridLayout();
		gridLayout_2.makeColumnsEqualWidth = true;
		gridLayout_2.numColumns = 3;
		composite_3.setLayout(gridLayout_2);

		final Button btnOk = new Button(composite_3, SWT.NONE);
		btnOk.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
                result = txtCombination.getText();
                shell.dispose();
			}
		});
		btnOk.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL));
		shell.setDefaultButton(btnOk);
		btnOk.setText("OK");

		final Button btnCancel = new Button(composite_3, SWT.NONE);
		btnCancel.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
                shell.dispose();
			}
		});
		btnCancel.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL));
		btnCancel.setText("Cancel");

		final Button btnHelp = new Button(composite_3, SWT.NONE);
		btnHelp.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL));
		btnHelp.setText("Help");
		//
	}
	/**
	 * @param fileName The fileName to set.
	 */
	public void setFileName(String fileName) {
		this.fileName = fileName;
	}
}
