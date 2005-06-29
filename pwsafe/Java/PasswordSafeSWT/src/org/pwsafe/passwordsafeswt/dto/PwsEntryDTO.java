package org.pwsafe.passwordsafeswt.dto;

import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;
import org.pwsafe.lib.file.PwsStringField;

/**
 * Convenience class for transferring password info around in a 
 * version-independent manner.
 * 
 * @author Glen Smith
 */
public class PwsEntryDTO {
    
    String group;
    String title;

    String username;
    String password;
    String notes;
    
    String version;
    
    
    /**
     * Default constructor.
     *
     */
    public PwsEntryDTO() {
        
    }
    
    
    public PwsEntryDTO(String group, String username, String password, String notes) {
     
        this.group = group;
        this.username = username;
        this.password = password;
        this.notes = notes;
    }

	/**
	 * @return Returns the group.
	 */
	public String getGroup() {
		return group;
	}
	/**
	 * @param group The group to set.
	 */
	public void setGroup(String group) {
		this.group = group;
	}
	/**
	 * @return Returns the notes.
	 */
	public String getNotes() {
		return notes;
	}
	/**
	 * @param notes The notes to set.
	 */
	public void setNotes(String notes) {
		this.notes = notes;
	}
	/**
	 * @return Returns the password.
	 */
	public String getPassword() {
		return password;
	}
	/**
	 * @param password The password to set.
	 */
	public void setPassword(String password) {
		this.password = password;
	}
	/**
	 * @return Returns the username.
	 */
	public String getUsername() {
		return username;
	}
	/**
	 * @param username The username to set.
	 */
	public void setUsername(String username) {
		this.username = username;
	}
    
    /**
     * @return Returns the title.
     */
    public String getTitle() {
        return title;
    }
    /**
     * @param title The title to set.
     */
    public void setTitle(String title) {
        this.title = title;
    }
    
    /**
     * @return Returns the version.
     */
    public String getVersion() {
        return version;
    }
    /**
     * @param version The version to set.
     */
    public void setVersion(String version) {
        this.version = version;
    }

    
    public static PwsEntryDTO fromPwsRecord(PwsRecord nextRecord) {
        PwsEntryDTO newEntry = new PwsEntryDTO();
        if (nextRecord instanceof PwsRecordV2) {
            
            PwsRecordV2 v2 = (PwsRecordV2) nextRecord;
            
            String groupName = v2.getField(PwsRecordV2.GROUP).getValue().toString();
            newEntry.setGroup(groupName);
            
            String title = v2.getField(PwsRecordV2.TITLE).getValue().toString();
            newEntry.setTitle(title);
            
            String user = v2.getField(PwsRecordV2.USERNAME).getValue().toString();
            newEntry.setUsername(user);
            
            String password = v2.getField(PwsRecordV2.PASSWORD).getValue().toString();
            newEntry.setPassword(password);
            
            String notes = v2.getField(PwsRecordV2.NOTES).getValue().toString();
            newEntry.setNotes(notes);
            
            newEntry.setVersion("2");
            
        } else {
            PwsRecordV1 v1 = (PwsRecordV1) nextRecord;
            
            String title = v1.getField(PwsRecordV1.TITLE).getValue().toString();
            newEntry.setTitle(title);
            
            String user = v1.getField(PwsRecordV1.USERNAME).getValue().toString();
            newEntry.setUsername(user);
            
            String password = v1.getField(PwsRecordV1.PASSWORD).getValue().toString();
            newEntry.setPassword(password);
            
            String notes = v1.getField(PwsRecordV1.NOTES).getValue().toString();
            newEntry.setNotes(notes);
            
            newEntry.setVersion("1");
        }
        return newEntry;
    }
    
    
	/**
     * Moves the contents of the DTO into the supplied PwsRecord.
     * 
	 * @param nextRecord the record to place the data into
	 */
	public void toPwsRecord(PwsRecord nextRecord) {
		
        if (nextRecord instanceof PwsRecordV2) {
            
            PwsRecordV2 v2 = (PwsRecordV2) nextRecord;
            v2.setField(new PwsStringField(PwsRecordV2.GROUP , getGroup()));
            v2.setField(new PwsStringField(PwsRecordV2.TITLE , getTitle()));
            v2.setField(new PwsStringField(PwsRecordV2.USERNAME , getUsername()));
            v2.setField(new PwsStringField(PwsRecordV2.PASSWORD , getPassword()));
            v2.setField(new PwsStringField(PwsRecordV2.NOTES , getNotes()));
            
        } else {
            
            PwsRecordV1 v1 = (PwsRecordV1) nextRecord;
            v1.setField(new PwsStringField(PwsRecordV1.TITLE , getTitle()));
            v1.setField(new PwsStringField(PwsRecordV1.USERNAME , getUsername()));
            v1.setField(new PwsStringField(PwsRecordV1.PASSWORD , getPassword()));
            v1.setField(new PwsStringField(PwsRecordV1.NOTES , getNotes()));  
            
        }

		
	}
}
