package org.pwsafe.passwordsafeswt.listener;

import org.eclipse.jface.viewers.DoubleClickEvent;
import org.eclipse.jface.viewers.IDoubleClickListener;
import org.pwsafe.passwordsafeswt.action.CopyPasswordAction;
import org.pwsafe.passwordsafeswt.action.EditRecordAction;
import org.pwsafe.passwordsafeswt.preference.MiscPreferences;
import org.pwsafe.passwordsafeswt.util.UserPreferences;

/**
 * Used for double click events on the tree or list viewer.
 * 
 * @author Glen Smith
 */
public class ViewerDoubleClickListener implements IDoubleClickListener {

	CopyPasswordAction cpa;
	EditRecordAction era;

	public ViewerDoubleClickListener() {
		cpa = new CopyPasswordAction();
		era = new EditRecordAction();
	}
	/**
	 * @see org.eclipse.jface.viewers.IDoubleClickListener#doubleClick(org.eclipse.jface.viewers.DoubleClickEvent)
	 */
	public void doubleClick(DoubleClickEvent dce) {
		if (UserPreferences.getInstance().getBoolean(
				MiscPreferences.DOUBLE_CLICK_COPIES_TO_CLIPBOARD)) {
			cpa.run();
		} else {
			era.run();
		}
	}

}
