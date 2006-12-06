package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;
import org.pwsafe.passwordsafeswt.dialog.PasswordDialog;

/**
 * Open command.
 *
 * @author Glen Smith
 */
public class OpenFileAction extends Action {

    public OpenFileAction() {
        super("&Open File@Ctrl+O");
        setImageDescriptor(ImageDescriptor.createFromURL(this.getClass().getClassLoader().getResource("org/pwsafe/passwordsafeswt/images/tool_newbar_open.gif")));
        setToolTipText("Open Existing Safe");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();
        boolean cancelled = app.saveAppIfDirty();
        if (!cancelled) {
            FileDialog fod = new FileDialog(app.getShell(), SWT.OPEN);
            fod.setFilterExtensions(new String[] { "*.psafe3", "*.dat", "*.*" });
            fod.setFilterNames(new String[] { "PasswordSafe v3 Files (*.psafe3)", "PasswordSafe v1/v2 Files (*.dat)", "All Files (*.*)"} );
            String fileName = fod.open();
            if (fileName != null) {
                PasswordDialog pd = new PasswordDialog(app.getShell());
                pd.setFileName(fileName);
                String password = (String) pd.open();
                if (password != null) {
                    try {
                        app.openFile(fileName, password);
                    } catch (Exception e) {
                        app.displayErrorDialog("Error Opening Safe", "Invalid Passphrase", e);
                    }
                }
            }
        }
    }

}