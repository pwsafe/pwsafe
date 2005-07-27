package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;
import org.pwsafe.passwordsafeswt.dialog.PasswordDialog;

/**
 * Imports a file from XML Source
 * 
 * @author Glen Smith
 */
public class ImportFromXMLAction extends Action {

	public ImportFromXMLAction() {
		super("&XML File...");
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
					app.importFromXML(fileName);
				} catch (Exception e) {
					app.displayErrorDialog("Error Importing XML", "Invalid Source File", e);
				}

			
		}
	}

}