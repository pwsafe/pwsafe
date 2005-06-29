package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;
import org.pwsafe.passwordsafeswt.dialog.PasswordDialog;

/**
 * Changes the safe combination.
 *
 * @author Glen Smith
 */
public class ChangeSafeCombinationAction extends Action {

    public ChangeSafeCombinationAction() {
        super("Change Safe Combination");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();
        PasswordDialog pd = new PasswordDialog(app.getShell());
        if (app.getPwsFile() != null) {
            pd.setFileName(app.getPwsFile().getFilename());
            String newPassword = (String) pd.open();
            if (newPassword != null) {
                app.setPassphrase(newPassword);
            }
        }
    }

}