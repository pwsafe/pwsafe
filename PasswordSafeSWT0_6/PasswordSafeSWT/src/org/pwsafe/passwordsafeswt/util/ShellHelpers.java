package org.pwsafe.passwordsafeswt.util;

import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.widgets.Shell;

/**
 * Utility routines related to shells (ie. windows).
 * 
 * @author Glen Smith
 */
public class ShellHelpers {
	
	public static void centreShell(Shell parent, Shell shell) {
		Rectangle bounds = parent.getBounds ();
		Rectangle rect = shell.getBounds ();
		int x = bounds.x + (bounds.width - rect.width) / 2;
		int y = bounds.y + (bounds.height - rect.height) / 2;
		shell.setLocation (x, y);
	}

}
