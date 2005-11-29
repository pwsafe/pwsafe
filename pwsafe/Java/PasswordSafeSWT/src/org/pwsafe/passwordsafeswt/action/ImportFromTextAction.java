package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;
import org.pwsafe.passwordsafeswt.dialog.PasswordDialog;

/**
 * Imports a file from CSV Source
 * 
 * @author Glen Smith
 */
public class ImportFromTextAction extends Action {

	public ImportFromTextAction() {
		super("Plain Text (tab separated)...");
	}
	
	/**
	 * @see org.eclipse.jface.action.Action#run()
	 */
	public void run() {
		PasswordSafeJFace app = PasswordSafeJFace.getApp();
    	FileDialog fod = new FileDialog(app.getShell(), SWT.OPEN);
    	String fileName = fod.open();
		if (fileName != null) {
	
				try {
					app.importFromText(fileName);
				} catch (Exception e) {
					app.displayErrorDialog("Error Importing Text", "Invalid Source File", e);
				}

			
		}
	}

}