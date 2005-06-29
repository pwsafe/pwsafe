package org.pwsafe.passwordsafeswt.model;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;
import org.pwsafe.lib.file.PwsFile;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;

/**
 * Content provider for the tree.
 * 
 * @author Glen Smith
 */
public class PasswordTreeContentProvider implements ITreeContentProvider {

	private static final Log log = LogFactory.getLog(PasswordTreeContentProvider.class);
	
	PwsFile file;
	

	/**
	 * @see org.eclipse.jface.viewers.ITreeContentProvider#getChildren(java.lang.Object)
	 */
	public Object[] getChildren(Object parentElement) {
		List matchingRecs = new ArrayList();
		if (parentElement instanceof String) {
			// return all record matching this group...
			for (Iterator iter = file.getRecords(); iter.hasNext(); ) {
				PwsRecordV2 nextRecord = (PwsRecordV2) iter.next();
				String recGroup = (String)nextRecord.getField(PwsRecordV2.GROUP).getValue();
				if (((String)parentElement).equalsIgnoreCase(recGroup)) {
						matchingRecs.add(nextRecord);
					}
			}
		}
		return matchingRecs.toArray(new PwsRecord[0]);
	}

	/**
	 * @see org.eclipse.jface.viewers.ITreeContentProvider#getParent(java.lang.Object)
	 */
	public Object getParent(Object element) {
		if (element instanceof PwsRecordV2) {
			return ((PwsRecordV2) element).getField(PwsRecordV2.GROUP)
					.getValue();
		}
		return null;
	}

	/**
	 * @see org.eclipse.jface.viewers.ITreeContentProvider#hasChildren(java.lang.Object)
	 */
	public boolean hasChildren(Object node) {
		return (node instanceof String); // only group strings have children
	}

	/**
	 * @see org.eclipse.jface.viewers.IStructuredContentProvider#getElements(java.lang.Object)
	 */
	public Object[] getElements(Object inputElement) {
		Set rootElements = new LinkedHashSet();
		if (inputElement instanceof PwsFile) {
			file = (PwsFile) inputElement;
			for (Iterator iter = file.getRecords(); iter.hasNext(); ) {
				PwsRecord thisRecord = (PwsRecord) iter.next();
				if (thisRecord instanceof PwsRecordV2) {
					PwsRecordV2 nextRecord = (PwsRecordV2) thisRecord;	
					String recGroup = (String)nextRecord.getField(PwsRecordV2.GROUP).getValue();
					rootElements.add(recGroup);
				} else {
					PwsRecordV1 nextRecord = (PwsRecordV1) thisRecord;	
					rootElements.add(nextRecord);
				}
			}

		} 		
		return rootElements.toArray();
	}

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
	public void inputChanged(Viewer tv, Object oldInput, Object newInput) {
		if (newInput instanceof PwsFile) {
			file = (PwsFile) newInput;
		}
		if (log.isDebugEnabled()) 
			log.debug("Input changed fired");
	}

}