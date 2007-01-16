package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
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
        boolean cancelled = app.saveAppIfDirty();
        if (!cancelled) {
            NewSafeDialog nsf = new NewSafeDialog(app.getShell());
            String passphrase = (String) nsf.open();
            if (passphrase != null) {
                app.newFile(passphrase);
            }
        }
    }

}