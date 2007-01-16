package org.pwsafe.passwordsafeswt.model;

import org.eclipse.jface.viewers.ILabelProvider;
import org.eclipse.jface.viewers.ILabelProviderListener;
import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.swt.graphics.Image;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV2;
import org.pwsafe.lib.file.PwsRecordV3;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;

/**
 * Label provider for tree viewer.
 * Also implements {@link org.eclipse.jface.viewers.ITableLabelProvider} to allow for tree columns.
 *
 * @author Glen Smith
 */
public class PasswordTreeLabelProvider implements ILabelProvider, ITableLabelProvider {

	/**
	 * @see org.eclipse.jface.viewers.ILabelProvider#getImage(java.lang.Object)
	 */
	public Image getImage(Object node) {
		return null;
	}

	/**
	 * @see org.eclipse.jface.viewers.ILabelProvider#getText(java.lang.Object)
	 */
	public String getText(Object node) {
        String result = "<unknown node type>";
		if (node instanceof String) {
			result = node.toString();
		} else if (node instanceof PwsRecord) {
		    PwsRecord record = (PwsRecord) node;
        	if (record instanceof PwsRecordV3) {
        		result = PwsEntryDTO.getSafeValue(record, PwsRecordV3.TITLE);
        	} else {
        		result = PwsEntryDTO.getSafeValue(record, PwsRecordV2.TITLE);	
        	}
		} else if (node instanceof PasswordTreeContentProvider.TreeGroup) {
		    result = node.toString();
		}
        return result;
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

    /* (non-Javadoc)
     * @see org.eclipse.jface.viewers.ITableLabelProvider#getColumnImage(java.lang.Object, int)
     */
    public Image getColumnImage(Object element, int columnIndex) {
        return null;
    }

    /* (non-Javadoc)
     * @see org.eclipse.jface.viewers.ITableLabelProvider#getColumnText(java.lang.Object, int)
     */
    public String getColumnText(Object element, int columnIndex) {
        String result = "";
        switch (columnIndex) {
        case 0:
            result = "<unknown node type>";
            if (element instanceof String) {
                result = element.toString();
            } else if (element instanceof PwsRecord) {
                PwsRecord record = (PwsRecord) element;
                if (record instanceof PwsRecordV3) {
                	result = PwsEntryDTO.getSafeValue(record, PwsRecordV3.TITLE);
                } else {
                	result = PwsEntryDTO.getSafeValue(record, PwsRecordV2.TITLE);
                }
            } else if (element instanceof PasswordTreeContentProvider.TreeGroup) {
                result = element.toString();
            }
            break;
        case 1:
            if (element instanceof PwsRecord) {
            	if (element instanceof PwsRecordV3) {
            		result = PwsEntryDTO.getSafeValue((PwsRecord) element, PwsRecordV3.USERNAME);
            	} else {
            		result = PwsEntryDTO.getSafeValue((PwsRecord) element, PwsRecordV2.USERNAME);	
            	}
            }
            break;
        case 2:
            if (element instanceof PwsRecord) {
            	if (element instanceof PwsRecordV3) {
            		result = PwsEntryDTO.getSafeValue((PwsRecord) element, PwsRecordV3.NOTES);
            	} else {
            		result = PwsEntryDTO.getSafeValue((PwsRecord) element, PwsRecordV2.NOTES);	
            	}
            }
            break;
        case 3:
            if (element instanceof PwsRecord) {
            	if (element instanceof PwsRecordV3) {
            		result = PwsEntryDTO.getSafeValue((PwsRecord) element, PwsRecordV3.PASSWORD);
            	} else {
            		result = PwsEntryDTO.getSafeValue((PwsRecord) element, PwsRecordV2.PASSWORD);	
            	}
            }
            break;
        }
        return result;
    }

}
