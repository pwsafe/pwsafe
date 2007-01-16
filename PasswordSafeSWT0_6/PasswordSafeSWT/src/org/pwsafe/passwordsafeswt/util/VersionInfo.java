package org.pwsafe.passwordsafeswt.util;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * 
 * Convenience class for access build version numbers for display
 * in the ui. Depends on a build.version properties file in the
 * classpath.
 * 
 * @author Glen Smith
 */
public class VersionInfo {

	private static final Log log = LogFactory.getLog(VersionInfo.class);
	
	/**
	 * Returns the version string (major.minor) for display in the UI.
	 * 
	 * @return a version string
	 */
	public static String getVersion() {

		String versionInfo = VersionInfo.class.getPackage().getImplementationVersion();
		if (versionInfo == null) 
			versionInfo = "?";
		return versionInfo;
		
	}
	

}
