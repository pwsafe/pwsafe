package org.pwsafe.passwordsafeswt.model;

import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;
import org.pwsafe.lib.file.PwsField;
import org.pwsafe.lib.file.PwsFile;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;
import org.pwsafe.lib.file.PwsRecordV3;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;

/**
 * Content provider for the tree.
 * 
 * @author Glen Smith
 */
public class PasswordTreeContentProvider implements ITreeContentProvider {

	private static final Log log = LogFactory.getLog(PasswordTreeContentProvider.class);

	PwsFile file;

    /**
     * This class represents a group displayed in the tree.
     */
    static final class TreeGroup {
        String parent;
        String name;

        public TreeGroup(String groupPath)
        {
            final int lastDot = groupPath.lastIndexOf('.');
            this.parent = groupPath.substring(0, lastDot);
            this.name = groupPath.substring(lastDot + 1);
        }

        public String toString() {
            return name;
        }

        /* (non-Javadoc)
         * @see java.lang.Object#hashCode()
         */
        public int hashCode() {
            final int PRIME = 31;
            int result = 1;
            result = PRIME * result + ((parent == null) ? 0 : parent.hashCode());
            result = PRIME * result + ((name == null) ? 0 : name.hashCode());
            return result;
        }

        /* (non-Javadoc)
         * @see java.lang.Object#equals(java.lang.Object)
         */
        public boolean equals(Object obj) {
            if (this == obj) {
                return true;
            }
            if (obj == null) {
                return false;
            }
            if (getClass() != obj.getClass()) {
                return false;
            }
            final TreeGroup other = (TreeGroup) obj;
            if (name == null) {
                if (other.name != null) {
                    return false;
                }
            } else if (!name.equals(other.name)) {
                return false;
            }
            if (parent == null) {
                if (other.parent != null) {
                    return false;
                }
            } else if (!parent.equals(other.parent)) {
                return false;
            }
            return true;
        }
    }

	/**
	 * @see org.eclipse.jface.viewers.ITreeContentProvider#getChildren(java.lang.Object)
	 */
	public Object[] getChildren(Object parentElement) {
		Set matchingRecs = new LinkedHashSet();
        String stringParent = null;
		if (parentElement instanceof String) {
            stringParent = (String) parentElement;
        } else if (parentElement instanceof TreeGroup) {
            TreeGroup element = (TreeGroup) parentElement;
            stringParent = element.parent + "." + element.name;
        }

        if (stringParent != null) {
			// return all record matching this group...
			for (Iterator iter = file.getRecords(); iter.hasNext(); ) {
				String recGroup = null;
				Object nextRecord = iter.next();
				
				if (nextRecord instanceof PwsRecordV3) {
					PwsRecordV3 nextRecordv3 = (PwsRecordV3) nextRecord;
					recGroup = PwsEntryDTO.getSafeValue(nextRecordv3, PwsRecordV3.GROUP);					
				} else if (nextRecord instanceof PwsRecordV2) {
					PwsRecordV2 nextRecordv2 = (PwsRecordV2) nextRecord;
					recGroup = PwsEntryDTO.getSafeValue(nextRecordv2, PwsRecordV2.GROUP);					
				}
				
                if (stringParent.equalsIgnoreCase(recGroup)) {
                    log.debug("Adding record");
                    matchingRecs.add(nextRecord);
                }
                else if (recGroup.length() > stringParent.length()
                        && stringParent.regionMatches(true, 0, recGroup, 0, stringParent.length())) {
                    log.debug("Adding group");
                    int nextDot = recGroup.indexOf('.', stringParent.length() + 1);
                    int endOfGroup = nextDot > 0 ? nextDot : recGroup.length();
                    String subGroup = recGroup.substring(0, endOfGroup);
					matchingRecs.add(new TreeGroup(subGroup));
                }
			}
		}
		return matchingRecs.toArray();
	}

	/**
	 * @see org.eclipse.jface.viewers.ITreeContentProvider#getParent(java.lang.Object)
	 */
	public Object getParent(Object element) {
		if (element instanceof PwsRecordV2) {
			return PwsEntryDTO.getSafeValue(((PwsRecordV2) element),PwsRecordV2.GROUP);
		}
        // TODO handle TreeGroup instances here?
		return null;
	}

	/**
	 * @see org.eclipse.jface.viewers.ITreeContentProvider#hasChildren(java.lang.Object)
	 */
	public boolean hasChildren(Object node) {
		return node instanceof String || node instanceof TreeGroup; // only groups have children
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
				if (thisRecord instanceof PwsRecordV1) {
					PwsRecordV1 nextRecord = (PwsRecordV1) thisRecord;	
					rootElements.add(nextRecord);
				} else {
					String recGroup = "";
					if (thisRecord instanceof PwsRecordV3) {
						PwsRecordV3 nextRecord = (PwsRecordV3) thisRecord;	
						PwsField field = nextRecord.getField(PwsRecordV3.GROUP);
						if (field != null) {
							recGroup = (String)field.getValue();
						}
					} else if (thisRecord instanceof PwsRecordV2 ) {
						PwsRecordV2 nextRecord = (PwsRecordV2) thisRecord;	
						recGroup = (String)nextRecord.getField(PwsRecordV2.GROUP).getValue();
					}
					if (recGroup.trim().length() == 0) { // empty group name
						rootElements.add(thisRecord);
					} else { // add node for group name
                        if (recGroup.indexOf('.') > 0) {
                            recGroup = recGroup.substring(0, recGroup.indexOf('.'));
                        }
						rootElements.add(recGroup);
					}				
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