package org.pwsafe.passwordsafeswt.model;

import org.eclipse.jface.viewers.ILabelProvider;
import org.eclipse.jface.viewers.ILabelProviderListener;
import org.eclipse.swt.graphics.Image;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;

/**
 * Label provider for tree viewer.
 *
 * @author Glen Smith
 */
public class PasswordTreeLabelProvider implements ILabelProvider {

	/**
	 * @see org.eclipse.jface.viewers.ILabelProvider#getImage(java.lang.Object)
	 */
	public Image getImage(Object node) {
		// TODO Auto-generated method stub
		return null;
	}

	/**
	 * @see org.eclipse.jface.viewers.ILabelProvider#getText(java.lang.Object)
	 */
	public String getText(Object node) {
		if (node instanceof String) {
			return node.toString();
		} else {
			if (node instanceof PwsRecordV2) {
				PwsRecordV2 v2 = (PwsRecordV2) node;
				return PwsEntryDTO.getSafeValue(v2, PwsRecordV2.TITLE) +
				" [" + PwsEntryDTO.getSafeValue(v2, PwsRecordV2.USERNAME) + "]";
			} else {
				PwsRecordV1 v1 = (PwsRecordV1) node;
				return PwsEntryDTO.getSafeValue(v1, PwsRecordV2.TITLE) +
				" [" + PwsEntryDTO.getSafeValue(v1, PwsRecordV2.USERNAME) + "]";
			}
		}
	}

	/**
	 * @see org.eclipse.jface.viewers.IBaseLabelProvider#addListener(org.eclipse.jface.viewers.ILabelProviderListener)
	 */
	public void addListener(ILabelProviderListener arg0) {
		// TODO Auto-generated method stub

	}

	/**
	 * @see org.eclipse.jface.viewers.IBaseLabelProvider#dispose()
	 */
	public void dispose() {
		// TODO Auto-generated method stub

	}

	/**
	 * @see org.eclipse.jface.viewers.IBaseLabelProvider#isLabelProperty(java.lang.Object, java.lang.String)
	 */
	public boolean isLabelProperty(Object arg0, String arg1) {
		// TODO Auto-generated method stub
		return false;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.IBaseLabelProvider#removeListener(org.eclipse.jface.viewers.ILabelProviderListener)
	 */
	public void removeListener(ILabelProviderListener arg0) {
		// TODO Auto-generated method stub

	}

}
