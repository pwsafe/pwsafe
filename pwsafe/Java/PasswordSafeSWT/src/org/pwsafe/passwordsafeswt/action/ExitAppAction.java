package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;

/**
 * Exit command.
 *
 * @author Glen Smith
 */
public class ExitAppAction extends Action {

    public ExitAppAction() {
        super("E&xit@Ctrl+Q");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();
        boolean cancelled = app.saveAppIfDirty();
        if (!cancelled) {
            app.exitApplication();
        }
    }

}