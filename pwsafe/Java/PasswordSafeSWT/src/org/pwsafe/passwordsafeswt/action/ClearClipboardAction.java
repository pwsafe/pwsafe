package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.dnd.Clipboard;
import org.eclipse.swt.dnd.TextTransfer;
import org.eclipse.swt.dnd.Transfer;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;

/**
 * Clears the current user clipboard.
 *
 * @author Glen Smith
 */
public class ClearClipboardAction extends Action {

    public ClearClipboardAction() {
        super("Clear Clipboard@Ctrl+Del");
        setImageDescriptor(ImageDescriptor.createFromURL(this.getClass().getClassLoader().getResource("org/pwsafe/passwordsafeswt/images/tool_newbar_clearclip.gif")));
        setToolTipText("Clear Clipboard");

    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();

        Clipboard cb = new Clipboard(app.getShell().getDisplay());

		cb.setContents(new Object[]{"  "},
                new Transfer[]{TextTransfer.getInstance()});

        cb.dispose();

    }

}