package org.pwsafe.passwordsafeswt.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Interface to all user preference activity (such as password policy, MRU, and
 * all that jazz).
 *  
 * @author Glen Smith
 */
public class UserPreferences {

	private static final Log log = LogFactory.getLog(UserPreferences.class);

	private static UserPreferences prefs;
	private static Properties props;

	private static final String MRU = "mru.";
	private static int MAX_MRU = 5;

	public static final String PROPS_DIR = ".passwordsafe";
	private static final String PREFS_FILENAME = "preferences.properties";

	/**
	 * Private constructor enforces singleton.
	 * @throws IOException
	 */
	private UserPreferences() throws IOException {
		loadPreferences();
	}

    /**
     * Returns the name of the preference file.
     * 
     * @return the name of the preferences file.
     */
	public String getPreferencesFilename() {
		String userDir = System.getProperty("user.home") + File.separator + PROPS_DIR + File.separator + PREFS_FILENAME;
		return userDir;
	}

    /**
     * Loads preferences from a properties file.
     * 
     * @throws IOException if there are problems loading the preferences file
     */
	private void loadPreferences() throws IOException {
		props = new Properties();
		String userFile = getPreferencesFilename();
		if (log.isDebugEnabled())
			log.debug("Loading from [" + userFile + "]");
		File prefsFile = new File(userFile);
		if (!prefsFile.exists()) {
			File prefsDir = new File(System.getProperty("user.home") + File.separator + PROPS_DIR);
			if (!prefsDir.exists()) {
				prefsDir.mkdir();
			}
		}
		if (prefsFile.exists()) {
			FileInputStream fis = new FileInputStream(userFile);
			props.load(fis);
			fis.close();
		} else {
			props = new Properties();
		}
		if (log.isDebugEnabled())
			log.debug("Loaded " + props.size()
					+ " preference settings from file");
	}

    /**
     * Saves the preference to a properties file.
     * 
     * @throws IOException if there are problems saving the file
     */
	public void savePreferences() throws IOException {

		String userFile = getPreferencesFilename();
		if (log.isDebugEnabled())
			log.debug("Saving to [" + userFile + "]");
		FileOutputStream fos = new FileOutputStream(userFile);
		props.store(fos, "User Preferences for PasswordSafeSWT");
		fos.close();
		if (log.isDebugEnabled())
			log.debug("Saved " + props.size()
					+ " preference settings from file");
	}

    /**
     * Sets the name of the most recently opened file.
     * 
     * @param fileName the name of the file
     */
	public void setMostRecentFilename(String fileName) {
        
        if (log.isDebugEnabled())
            log.debug("Setting most recently opened file to: [" + fileName +"]");
        
        try {
			loadPreferences();  // make sure we get the latest
		} catch (IOException ioe) {
			log.error("Couldn't load preferences", ioe);
		} 
		Set newMRU = new LinkedHashSet();
        newMRU.add(fileName);
        newMRU.addAll(Arrays.asList(getMRUFiles()));		
		int mruCounter = 0;
		for (Iterator iter = newMRU.iterator(); iter.hasNext()
				&& mruCounter <= MAX_MRU;) {
			mruCounter++;
			String nextFilename = (String) iter.next();
			props.setProperty(MRU + mruCounter, nextFilename);
		}
        try {
			savePreferences();
		} catch (IOException e) {
			log.warn("Unable to save preferences file", e);
		}
	}

    /**
     * Returns an array of recently opened filename (most recent to oldest).
     * 
     * @return an array of recently opened filename
     */
	public String[] getMRUFiles() {

		List allFiles = new ArrayList();
		for (int i = 0; i <= MAX_MRU; i++) {
			String nextFile = props.getProperty(MRU + i);
			if (nextFile != null)
				allFiles.add(nextFile);
		}
		return (String[]) allFiles.toArray(new String[0]);

	}
    
    /**
     * Convenience routine for getting most recently opened file.
     * 
     * @return the filename of the MRU file (or null if there is no MRU file)
     */
    public String getMRUFile() {
        
        String[] allMRU = getMRUFiles();
        if (allMRU.length > 0) {
        	return allMRU[0];
        } else {
        	return null;
        }
        
    }
    
    public String getString(String propName) {
    	return props.getProperty(propName, "");
    }
    
    public void setString(String propName, String propValue) {
    	props.setProperty(propName, propValue);
    }
    
    public boolean getBoolean(String propName) {
    	return Boolean.valueOf(props.getProperty(propName, "false")).booleanValue();
    }

    /**
     * Singleton creator.
     * 
     * @return a handle to the UserPreferences object for this user
     */
	public static synchronized UserPreferences getInstance() {
		if (prefs == null) {
			try {
				prefs = new UserPreferences();
			} catch (IOException e) {
				log.error("Couldn't load preferences file.", e);
			}
		}
		return prefs;
	}

	public static synchronized void reload() {
		prefs = null;
	}
	
}