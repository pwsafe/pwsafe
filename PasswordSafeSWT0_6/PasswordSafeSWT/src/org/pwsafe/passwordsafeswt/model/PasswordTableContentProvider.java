package org.pwsafe.passwordsafeswt.model;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.Viewer;
import org.pwsafe.lib.file.PwsFile;
import org.pwsafe.lib.file.PwsRecord;

/**
 * Content Provider for the Table.
 * 
 * @author Glen Smith
 */
public class PasswordTableContentProvider implements IStructuredContentProvider {

	private static final Log log = LogFactory.getLog(PasswordTableContentProvider.class);
	
	PwsFile file;

	/**
	 * @see org.eclipse.jface.viewers.IContentProvider#dispose()
	 */
	public void dispose() {
		// TODO Auto-generated method stub

	}

	/**
	 * @see org.eclipse.jface.viewers.IContentProvider#inputChanged(org.eclipse.jface.viewers.Viewer,
	 *      java.lang.Object, java.lang.Object)
	 */
	public void inputChanged(Viewer vwr, Object oldInput, Object newInput) {
		if (newInput instanceof PwsFile) {
			file = (PwsFile) newInput;
		}
		if (log.isDebugEnabled()) 
			log.debug("Input changed fired");
	}

	/**
	 * @see org.eclipse.jface.viewers.IStructuredContentProvider#getElements(java.lang.Object)
	 */
	public Object[] getElements(Object inputElement) {

		List allRecords = new ArrayList();
		if (inputElement instanceof PwsFile) {
			file = (PwsFile) inputElement;
			for (Iterator iter = file.getRecords(); iter.hasNext();) {
				allRecords.add(iter.next());
			}
		}
		return allRecords.toArray(new PwsRecord[0]);
	}

}