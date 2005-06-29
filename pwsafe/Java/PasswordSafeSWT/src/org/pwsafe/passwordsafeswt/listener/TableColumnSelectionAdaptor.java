package org.pwsafe.passwordsafeswt.listener;

import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.pwsafe.passwordsafeswt.model.PasswordTableSorter;

/**
 * An adaptor used for handing sorting of a column when the user clicks the
 * column header.
 *  
 * @author Glen Smith
 */
public class TableColumnSelectionAdaptor extends SelectionAdapter {
	
	private TableViewer tv;
	private int columnNumber;
	
	public TableColumnSelectionAdaptor(TableViewer tv, int columnNumber) {
		this.tv = tv;
		this.columnNumber = columnNumber;
	}
	
	public void widgetSelected(SelectionEvent se) {
		PasswordTableSorter pts = (PasswordTableSorter) tv.getSorter();
		pts.sortOnColumn(columnNumber);
		tv.refresh();
	}

}
