package org.pwsafe.passwordsafeswt.action;

import java.io.IOException;

import org.eclipse.jface.action.Action;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;

/**
 * SaveAs command.
 *
 * @author Glen Smith
 */
public class SaveFileAsAction extends Action {

    public SaveFileAsAction() {
        super("Save &As");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();
        FileDialog fw = new FileDialog(app.getShell(), SWT.SAVE);
        String newFilename = fw.open();
        if (newFilename != null) {
            try {
                app.saveFileAs(newFilename);
            } catch (IOException e1) {
                app.displayErrorDialog("Error Saving Safe", e1.getMessage(), e1);
            }

        }

    }

}