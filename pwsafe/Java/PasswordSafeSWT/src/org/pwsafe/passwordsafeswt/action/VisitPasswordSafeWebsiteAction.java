package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.swt.SWT;
import org.eclipse.swt.program.Program;
import org.eclipse.swt.widgets.MessageBox;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;


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
                		Program.launch("http://www.bytecode.com.au/passwordsafeswt/");
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