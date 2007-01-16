package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;
import org.pwsafe.passwordsafeswt.dialog.AboutDialog;

/**
 * Displays the About dialog.
 *
 * @author Glen Smith
 */
public class AboutAction extends Action {

    public AboutAction() {
        super("About");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();
        AboutDialog ad = new AboutDialog(app.getShell());
        ad.open();
    }

}