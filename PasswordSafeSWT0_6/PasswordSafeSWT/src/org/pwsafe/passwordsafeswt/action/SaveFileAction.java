package org.pwsafe.passwordsafeswt.action;

import java.io.IOException;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;

/**
 * Save command.
 *
 * @author Glen Smith
 */
public class SaveFileAction extends Action {

    public SaveFileAction() {
        super("&Save@Ctrl+S");
        setImageDescriptor(ImageDescriptor.createFromURL(this.getClass().getClassLoader().getResource("org/pwsafe/passwordsafeswt/images/tool_newbar_save.gif")));
        setToolTipText("Save");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();
        try {
            app.saveFile();
        } catch (IOException e1) {
            app.displayErrorDialog("Error Saving Safe", e1.getMessage(), e1);
        }

    }

}