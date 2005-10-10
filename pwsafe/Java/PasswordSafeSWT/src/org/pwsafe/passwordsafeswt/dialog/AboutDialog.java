package org.pwsafe.passwordsafeswt.dialog;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Dialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.pwsafe.passwordsafeswt.util.ShellHelpers;
import org.pwsafe.passwordsafeswt.util.VersionInfo;

/**
 * AboutDialog shows author/contributor/contact details.
 * 
 * @author Glen Smith
 */
public class AboutDialog extends Dialog {

	protected Object result;
	protected Shell shell;
	public AboutDialog(Shell parent, int style) {
		super(parent, style);
	}
	public AboutDialog(Shell parent) {
		this(parent, SWT.NONE);
	}
	public Object open() {
		createContents();
		ShellHelpers.centreShell(getParent(), shell);
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
		shell.setLayout(new GridLayout());
		shell.setSize(401, 234);
		shell.setText("About PasswordSafe");

		final Group group = new Group(shell, SWT.NONE);
		group.setText("About");
		group.setLayoutData(new GridData(GridData.FILL_BOTH));
		group.setLayout(new GridLayout());

		final Label lblVersion = new Label(group, SWT.NONE);
		lblVersion.setAlignment(SWT.CENTER);
		lblVersion.setLayoutData(new GridData(GridData.FILL_HORIZONTAL | GridData.VERTICAL_ALIGN_FILL));
		lblVersion.setText("Version: " + VersionInfo.getVersion());

		final Composite composite = new Composite(shell, SWT.NONE);
		composite.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_END));
		composite.setLayout(new GridLayout());

		final Button btnOk = new Button(composite, SWT.NONE);
		btnOk.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
                shell.dispose();
			}
		});
		final GridData gridData = new GridData(GridData.HORIZONTAL_ALIGN_FILL);
		gridData.widthHint = 46;
		btnOk.setLayoutData(gridData);
		btnOk.setText("OK");
		//
	}
}
