package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;

/**
 * Changes the safe to Tree view.
 *
 * @author Glen Smith
 */
public class ViewAsTreeAction extends Action {

    public ViewAsTreeAction() {
        super("Tree", AS_RADIO_BUTTON);
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        final PasswordSafeJFace app = PasswordSafeJFace.getApp();
        app.showTreeView();
    }

}