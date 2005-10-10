package org.pwsafe.passwordsafeswt.util;

import java.io.FileInputStream;
import java.util.Properties;

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

	private static Properties props = new Properties();

	/**
	 * Loads build.version from the classpath.
	 * 
	 * @return a Properties file of build information (see build.version)
	 */
	private static Properties getProperties() {
		if (props.size() == 0) {
			synchronized (props) {
				try {
					props.load(new FileInputStream("build.version"));
				} catch (Exception e) {
					log.error("Could not load version file", e);
				}
			}
		}
		return props;
	}

	
	/**
	 * Returns the version string (major.minor) for display in the UI.
	 * 
	 * @return a version string
	 */
	public static String getVersion() {

		Properties buildProps = getProperties();
		String versionInfo = props.getProperty("build.release.major", "0")
				+ "." + props.getProperty("build.release.minor", "0");
		return versionInfo;
		
	}
	

}
