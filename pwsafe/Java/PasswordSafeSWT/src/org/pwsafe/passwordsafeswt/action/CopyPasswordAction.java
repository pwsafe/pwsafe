package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.dnd.Clipboard;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;

/**
 * Copies the password from selected item to the clipboard.
 *
 * @author Glen Smith
 */
public class CopyPasswordAction extends Action {

    public CopyPasswordAction() {
        super("Copy Password@Ctrl+C");
        setImageDescriptor(ImageDescriptor.createFromURL(this.getClass().getClassLoader().getResource("org/pwsafe/passwordsafeswt/images/tool_newbar_pwd.gif")));
        setToolTipText("Copy Password To Clipboard");

    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();

        PwsRecord recordToCopy = app.getSelectedRecord();

        Clipboard cb = new Clipboard(app.getShell().getDisplay());

        app.copyToClipboard(cb, recordToCopy, recordToCopy instanceof PwsRecordV1 ? PwsRecordV1.PASSWORD : PwsRecordV2.PASSWORD);

        cb.dispose();

    }

}