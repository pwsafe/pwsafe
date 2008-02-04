package org.pwsafe.passwordsafeswt.dialog;

import java.util.Date;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.DateTime;
import org.eclipse.swt.widgets.Dialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

/**
 * A dialog to choose a Date from a calendar.
 * Designed similar to org.eclipse.swt.widgets.ColorDialog.
 *
 * @author David Müller
 */
public class DateDialog extends Dialog {

	private Date date;

	public DateDialog(Shell shell) {
		super(shell, SWT.DIALOG_TRIM | SWT.APPLICATION_MODAL);
	}

	public DateDialog(Shell shell, int i) {
		super(shell, i);
	}
	
	public Date getDate() {
		return date;
	}

	public Date open() {				
		final Shell shell = new Shell (getParent(), getStyle());
		shell.setText(getText());
		createContents(shell);
		shell.pack();
		shell.open();
		Display display = getParent().getDisplay();
		while (!shell.isDisposed()) {
			if (!display.readAndDispatch()) {
				display.sleep();
			}
		}		
		return date;
	}

	private void createContents(final Shell shell) {
		shell.setLayout (new GridLayout (2, true));

		final DateTime calendar = new DateTime (shell, SWT.CALENDAR | SWT.BORDER);
		GridData data = new GridData();
		data.horizontalSpan = 2;
		calendar.setLayoutData(data);
		//TODO: set DateTime value from java.util.Date...
		
		Button ok = new Button (shell, SWT.PUSH);
		ok.setText ("OK");
		ok.setLayoutData(new GridData (SWT.FILL, SWT.CENTER, false, false));
		ok.addSelectionListener (new SelectionAdapter () {
			public void widgetSelected (SelectionEvent e) {
				System.out.println ("Calendar date selected (MM/DD/YYYY) = " + (calendar.getMonth () + 1) + "/" + calendar.getDay () + "/" + calendar.getYear ());
//				date =  // TODO: convert DateTime result to java.util.Date...
				shell.close ();
			}
		});
		shell.setDefaultButton (ok);
		
		Button cancel = new Button (shell, SWT.PUSH);
		cancel.setText ("Cancel");
		cancel.setLayoutData(new GridData (SWT.FILL, SWT.CENTER, false, false));
		cancel.addSelectionListener (new SelectionAdapter () {
			public void widgetSelected (SelectionEvent e) {
				System.out.println ("Calendar date selected (MM/DD/YYYY) = null");
				date = null;
				shell.close ();
			}
		});

	}

	public void setDate(Date aDate) {
		date = aDate;
	}
}
