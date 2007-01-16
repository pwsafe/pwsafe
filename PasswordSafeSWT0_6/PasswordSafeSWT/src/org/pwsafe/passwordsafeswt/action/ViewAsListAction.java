package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;

/**
 * Changes the safe to List view.
 *
 * @author Glen Smith
 */
public class ViewAsListAction extends Action {

    public ViewAsListAction() {
        super("List", AS_RADIO_BUTTON);
        setChecked(true); // default radio
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        final PasswordSafeJFace app = PasswordSafeJFace.getApp();
        app.showListView();
    }

}