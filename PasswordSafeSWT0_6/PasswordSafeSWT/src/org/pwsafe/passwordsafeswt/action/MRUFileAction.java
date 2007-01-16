package org.pwsafe.passwordsafeswt.action;

import java.io.IOException;

import org.eclipse.jface.action.Action;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.MessageBox;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;
import org.pwsafe.passwordsafeswt.dialog.PasswordDialog;

/**
 * Used for MRU files on the File menu. Each MRU menu item is an instance of this action.
 *
 * @author Glen Smith
 */
public class MRUFileAction extends Action {

    String fileName;

    public MRUFileAction(String fileName, String menuDetails) {
        super(menuDetails);
        this.fileName = fileName;
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();
        if (app.isDirty()) {
            int style = SWT.APPLICATION_MODAL | SWT.YES | SWT.NO | SWT.CANCEL;
            MessageBox messageBox = new MessageBox(app.getShell(), style);
            messageBox.setText("Save Changes");
            messageBox.setMessage("Do you want to save changes to the password list?");
            int result = messageBox.open();
            if (result == SWT.YES) {
                try {
                    app.saveFile();
                } catch (IOException e1) {
                    app.displayErrorDialog("Error Saving Safe", e1.getMessage(), e1);
                    return;
                }
            } else if (result == SWT.CANCEL) {
                return;
            }
        }
        PasswordDialog pd = new PasswordDialog(app.getShell());
        pd.setFileName(fileName);
        String password = (String) pd.open();
        if (password != null) {
            try {
                app.openFile(fileName, password);
            } catch (Exception e) {
                app.displayErrorDialog("Error Opening Safe", "Invalid Passphrase", e);
            }

        }

    }

}