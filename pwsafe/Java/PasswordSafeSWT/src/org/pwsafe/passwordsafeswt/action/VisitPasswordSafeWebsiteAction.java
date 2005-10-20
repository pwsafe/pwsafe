package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.MessageBox;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;

import edu.stanford.ejalbert.BrowserLauncher;

/**
 * Launches a browser to the passwordsafe website.
 *
 * @author Glen Smith
 */
public class VisitPasswordSafeWebsiteAction extends Action {

    public VisitPasswordSafeWebsiteAction() {
        super("Visit Password Safe website");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        final PasswordSafeJFace app = PasswordSafeJFace.getApp();
        new Thread() {
            public void run() {
                try {
                    BrowserLauncher
                            .openURL("http://passwordsafe.sourceforge.net/");
                } catch (Exception ioe) {
                    MessageBox mb = new MessageBox(app.getShell(),
                            SWT.ICON_ERROR);
                    mb.setText("Could not open URL");
                    mb
                            .setText("Could not launch browser to http://passwordsafe.sourceforge.net/");
                    mb.open();
                }
            }
        }.start();
    }

}