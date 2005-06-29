package org.pwsafe.passwordsafeswt.listener;

import org.eclipse.jface.viewers.DoubleClickEvent;
import org.eclipse.jface.viewers.IDoubleClickListener;
import org.pwsafe.passwordsafeswt.action.CopyPasswordAction;

/**
 * Used for double click events on the tree or list viewer.
 * 
 * @author Glen Smith
 */
public class ViewerDoubleClickListener implements IDoubleClickListener {
    
    CopyPasswordAction cpa;
    
    public ViewerDoubleClickListener() {
    	cpa = new CopyPasswordAction();
    }
    /**
	 * @see org.eclipse.jface.viewers.IDoubleClickListener#doubleClick(org.eclipse.jface.viewers.DoubleClickEvent)
	 */
	public void doubleClick(DoubleClickEvent dce) {
		cpa.run();
	}

}
