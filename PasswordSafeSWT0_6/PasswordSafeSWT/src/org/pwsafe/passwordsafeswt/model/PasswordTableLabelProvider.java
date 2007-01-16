package org.pwsafe.passwordsafeswt.model;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.eclipse.jface.viewers.ILabelProviderListener;
import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.swt.graphics.Image;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;
import org.pwsafe.lib.file.PwsRecordV3;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;

/**
 * Label Provider for the password Table.
 *
 * @author Glen Smith
 */
public class PasswordTableLabelProvider implements ITableLabelProvider {
	
	private static final Log log = LogFactory.getLog(PasswordTableLabelProvider.class);

	/**
	 * @see org.eclipse.jface.viewers.ITableLabelProvider#getColumnImage(java.lang.Object, int)
	 */
	public Image getColumnImage(Object element, int columnIndex) {
		return null;
	}

	/**
	 * @see org.eclipse.jface.viewers.ITableLabelProvider#getColumnText(java.lang.Object, int)
	 */
	public String getColumnText(Object element, int columnIndex) {
		String columnString = null;
		
		if (element instanceof PwsRecordV3) {
			PwsRecordV3 v3 = (PwsRecordV3) element;
			switch(columnIndex) {
				case 0:
					columnString =  PwsEntryDTO.getSafeValue(v3,PwsRecordV3.TITLE);
					break;
			    case 1:
			    	columnString =  PwsEntryDTO.getSafeValue(v3,PwsRecordV3.USERNAME);
			    	break;
			    case 2:
			    	columnString = PwsEntryDTO.getSafeValue(v3,PwsRecordV3.NOTES);
			    	break;
			    case 3:
			    	columnString = PwsEntryDTO.getSafeValue(v3,PwsRecordV3.PASSWORD);
			}
		} else if (element instanceof PwsRecordV2) {
			PwsRecordV2 v2 = (PwsRecordV2) element;
			switch(columnIndex) {
				case 0:
					columnString =  PwsEntryDTO.getSafeValue(v2,PwsRecordV2.TITLE);
					break;
			    case 1:
			    	columnString =  PwsEntryDTO.getSafeValue(v2,PwsRecordV2.USERNAME);
			    	break;
			    case 2:
			    	columnString = PwsEntryDTO.getSafeValue(v2,PwsRecordV2.NOTES);
			    	break;
			    case 3:
			    	columnString = PwsEntryDTO.getSafeValue(v2,PwsRecordV2.PASSWORD);
			}
		} else {
			PwsRecordV1 v1 = (PwsRecordV1) element;
			switch(columnIndex) {
				case 0:
					columnString = PwsEntryDTO.getSafeValue(v1,PwsRecordV1.TITLE);
					break;
			    case 1:
			    	columnString = PwsEntryDTO.getSafeValue(v1,PwsRecordV1.USERNAME);
			    	break;
			    case 2:
			    	columnString = PwsEntryDTO.getSafeValue(v1,PwsRecordV1.NOTES);
			    	break;
			    case 3:
			    	columnString = PwsEntryDTO.getSafeValue(v1,PwsRecordV1.PASSWORD);
			    	break;
			}
			
		}
		if (log.isDebugEnabled())
			log.debug("Setting column index " + columnIndex + " to [" + columnString + "]");
		return columnString;  // unknown column
	}

	/**
	 * @see org.eclipse.jface.viewers.IBaseLabelProvider#addListener(org.eclipse.jface.viewers.ILabelProviderListener)
	 */
	public void addListener(ILabelProviderListener listener) {
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
	public boolean isLabelProperty(Object element, String property) {
		// TODO Auto-generated method stub
		return false;
	}

	/**
	 * @see org.eclipse.jface.viewers.IBaseLabelProvider#removeListener(org.eclipse.jface.viewers.ILabelProviderListener)
	 */
	public void removeListener(ILabelProviderListener listener) {
		// TODO Auto-generated method stub

	}

}
