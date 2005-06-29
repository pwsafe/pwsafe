package org.pwsafe.passwordsafeswt.action;

import java.io.IOException;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.MessageBox;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;
import org.pwsafe.passwordsafeswt.dialog.NewSafeDialog;

/**
 * New command.
 *
 * @author Glen Smith
 */
public class NewFileAction extends Action {

    public NewFileAction() {
        super("&New File@Ctrl+N");
        setImageDescriptor(ImageDescriptor.createFromURL(this.getClass().getClassLoader().getResource("org/pwsafe/passwordsafeswt/images/tool_newbar_new.gif")));
        setToolTipText("Create New Safe");
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
            messageBox
                    .setMessage("Do you want to save changes to the password list?");
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
        NewSafeDialog nsf = new NewSafeDialog(app.getShell());
        String passphrase = (String) nsf.open();
        if (passphrase != null) {
            app.newFile(passphrase);
        }
    }

}