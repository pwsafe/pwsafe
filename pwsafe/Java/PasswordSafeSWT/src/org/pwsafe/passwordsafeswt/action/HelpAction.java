package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;

/**
 * Displays the Help.
 *
 * @author Glen Smith
 */
public class HelpAction extends Action {

    public HelpAction() {
        super("Get Help@F1");
        setImageDescriptor(ImageDescriptor.createFromURL(this.getClass().getClassLoader().getResource("org/pwsafe/passwordsafeswt/images/tool_newbar_help.gif")));
        setToolTipText("Help");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        final PasswordSafeJFace app = PasswordSafeJFace.getApp();
        //TODO show help here...
    }

}