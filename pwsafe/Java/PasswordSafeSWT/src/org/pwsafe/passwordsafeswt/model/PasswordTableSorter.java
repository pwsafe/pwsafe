package org.pwsafe.passwordsafeswt.model;

import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerSorter;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.passwordsafeswt.dto.PwsEntryDTO;

/**
 * Implements the sorting logic for the table. Most of this was lifted straight
 * from "Definitive Guide to SWT & JFace" - a very cool SWT book.
 * 
 * @author Glen Smith
 *
 */
public class PasswordTableSorter extends ViewerSorter {

	public int ASCENDING = 0;
	public int DESCENDING = 1;
	
	private int column;
	private int direction;
	
	public void sortOnColumn(int columnNumber) {
		if (columnNumber == column) {
			direction = 1 - direction;
		} else {
			this.column = columnNumber;
			direction = ASCENDING;
		}
		
	}
	
	public int compare(Viewer arg0, Object a, Object b) {
		int rc = 0;
		
		//FIXME avoid recreating new EntryDTOs all the time, for example by an adapter
		PwsEntryDTO entry1 = PwsEntryDTO.fromPwsRecord((PwsRecord) a);
		PwsEntryDTO entry2 = PwsEntryDTO.fromPwsRecord((PwsRecord) b);
		
		switch(column) {
		
		case 1:
			rc = collator.compare(entry1.getTitle(), entry2.getTitle());
			break;
		case 2:
			rc = collator.compare(entry1.getUsername(), entry2.getUsername());
			break;
		
		case 3:
			rc = collator.compare(entry1.getNotes(), entry2.getNotes());
			break;
			
		}
		
		
		
		if (direction == DESCENDING)
			rc = -rc;
		
		return rc;
	}
	
}
