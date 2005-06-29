package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;
import org.pwsafe.passwordsafeswt.dialog.EditDialog;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;

/**
 * Displays the Edit dialog.
 *
 * @author Glen Smith
 */
public class EditRecordAction extends Action {

    public EditRecordAction() {
        super("&Edit Record");
        setImageDescriptor(ImageDescriptor.createFromURL(this.getClass().getClassLoader().getResource("org/pwsafe/passwordsafeswt/images/tool_newbar_edit.gif")));
        setToolTipText("Edit Selected Record");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();
        PwsRecord selectedRecord = app.getSelectedRecord();
        if (selectedRecord != null) {
            PwsEntryDTO newEntry = PwsEntryDTO.fromPwsRecord(selectedRecord);
            EditDialog ed = new EditDialog(app.getShell(), newEntry);
            newEntry = (PwsEntryDTO) ed.open();
            if (newEntry != null) {
                app.editRecord(newEntry);
            }
        }

    }

}