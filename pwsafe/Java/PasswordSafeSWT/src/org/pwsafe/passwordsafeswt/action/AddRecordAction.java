package org.pwsafe.passwordsafeswt.action;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.resource.ImageDescriptor;
import org.pwsafe.passwordsafeswt.PasswordSafeJFace;
import org.pwsafe.passwordsafeswt.dialog.EditDialog;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;
import org.pwsafe.passwordsafeswt.preference.UsernamePreferences;
import org.pwsafe.passwordsafeswt.util.UserPreferences;

/**
 * Adds a new record to the password safe.
 *
 * @author Glen Smith
 */
public class AddRecordAction extends Action {

    public AddRecordAction() {
        super("&Add Record@Ctrl+A");
        setImageDescriptor(ImageDescriptor.createFromURL(this.getClass().getClassLoader().getResource("org/pwsafe/passwordsafeswt/images/tool_newbar_add.gif")));
        setToolTipText("Add New Record");
    }

    /**
     * @see org.eclipse.jface.action.Action#run()
     */
    public void run() {
        PasswordSafeJFace app = PasswordSafeJFace.getApp();
        PwsEntryDTO newEntry = PwsEntryDTO.fromPwsRecord(app.getPwsFile().newRecord());     
		UserPreferences prefs = UserPreferences.getInstance();
		if (prefs.getBoolean(UsernamePreferences.USE_DEFAULT_USERNAME)) {
			newEntry.setUsername(prefs.getString(UsernamePreferences.DEFAULT_USERNAME));
		}
		if (app.isTreeViewShowing()) {
			// create new entry within existing group
			String selectedGroup = app.getSelectedTreeGroup();
			if (selectedGroup != null && selectedGroup.length() > 0) {
				newEntry.setGroup(selectedGroup);
			}
		}
        EditDialog ed = new EditDialog(app.getShell(), newEntry);
        newEntry = (PwsEntryDTO) ed.open();
        if (newEntry != null) {
            app.addRecord(newEntry);
        }

    }

}