package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.MessageBox;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;

/**
 * Deletes the selected record after prompting.
 *
 * @author Glen Smith
 */
public class DeleteRecordAction extends Action {

    public DeleteRecordAction() {
        super("&Delete Record@Del");
        setImageDescriptor(ImageDescriptor.createFromURL(this.getClass().getClassLoader().getResource("org/pwsafe/passwordsafeswt/images/tool_newbar_delete.gif")));
        setToolTipText("Delete Selected Record");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();
        PwsRecord selectedRec = app.getSelectedRecord();
        if (selectedRec != null) {
            MessageBox mb = new MessageBox(app.getShell(), SWT.ICON_QUESTION | SWT.YES | SWT.NO);
            mb.setText("Delete Record");
            PwsEntryDTO entry = PwsEntryDTO.fromPwsRecord(selectedRec);
            mb.setMessage("Are you sure you want to delete record " + entry.getTitle() + " ?");
            int result = mb.open();
            if (result == SWT.YES) {
                app.deleteRecord();
            }
        }

    }

}