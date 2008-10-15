package org.pwsafe.passwordsafeswt.dto;

import java.util.Date;
import org.pwsafe.lib.UUID;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.pwsafe.lib.file.PwsField;
import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;
import org.pwsafe.lib.file.PwsRecordV3;
import org.pwsafe.lib.file.PwsStringField;
import org.pwsafe.lib.file.PwsStringUnicodeField;
import org.pwsafe.lib.file.PwsTimeField;
import org.pwsafe.lib.file.PwsUUIDField;

/**
 * Convenience class for transferring password info around in a 
 * version-independent manner.
 * 
 * @author Glen Smith
 */
public class PwsEntryDTO {
    
	private static final Log log = LogFactory.getLog(PwsEntryDTO.class);

	UUID id;
    String group;
    String title;

    String username;
    String password;
    String notes;
    String url;
    String autotype;
    
    String version;
    
    Date lastAccess;
    Date created;
    Date lastPwChange;
    Date lastChange;
    Date expires;
    
    /**
     * Default constructor.
     *
     */
    public PwsEntryDTO() {
        super();
    }
    
    
    public PwsEntryDTO(String group, String username, String password, String notes) {
    	this();
        this.group = group;
        this.username = username;
        this.password = password;
        this.notes = notes;
    }

	public UUID getId() {
		return id;
	}

	public void setId(UUID id) {
		this.id = id;
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

    public String getUrl() {
		return url;
	}

	public void setUrl(String url) {
		this.url = url;
	}

	public String getAutotype() {
		return autotype;
	}


	public void setAutotype(String autotype) {
		this.autotype = autotype;
	}

	public Date getCreated() {
		return created;
	}

	public void setCreated(Date created) {
		this.created = created;
	}

	public Date getLastAccess() {
		return lastAccess;
	}

	public void setLastAccess(Date lastAccess) {
		this.lastAccess = lastAccess;
	}

	public Date getLastChange() {
		return lastChange;
	}

	public void setLastChange(Date lastChange) {
		this.lastChange = lastChange;
	}

	public Date getLastPwChange() {
		return lastPwChange;
	}

	public void setLastPwChange(Date lastPwChange) {
		this.lastPwChange = lastPwChange;
	}

	public Date getExpires() {
		return expires;
	}

	public void setExpires(Date expires) {
		this.expires = expires;
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

    
    
	public String toString() {
		StringBuffer all = new StringBuffer (200);
		all.append("PwsEntryDTO ").append(version).append(": ID ");
		all.append(id != null ? id.toString() : null);
		all.append(", Group ").append(group); 
		all.append(", Title ").append(title);
		all.append(", User ").append(username);
		all.append(", Notes ").append(notes);
		all.append(", Url ").append(url);
		all.append(", Created ").append(created);
		all.append(", Changed ").append(lastChange);
		all.append(", Expires ").append(expires);
		return all.toString();
	}


	/**
     * A safer version of field retrieval that is null-safe.
     * 
     * @param record the record to retrieve the field from
     * @param fieldCode the code of the field
     * @return the value of the field or an empty string if the field is null
     */
    public static String getSafeValue(PwsRecord record, int fieldCode) {
    	String fieldValue = "";
    	PwsField field = record.getField(fieldCode);
    	if (field != null && field.getValue() != null) {
    		fieldValue = field.getValue().toString();
    	} 
    	return fieldValue;
    }
    
    /**
     * A safer version of date retrieval that is null-safe.
     * 
     * @param v3
     * @param type
     * @return the Field as Date
     */
	public static Date getSafeDate(final PwsRecordV3 v3, final int type) {
		final PwsTimeField field = (PwsTimeField) v3.getField(type);

		return field != null ? (Date) field.getValue() : null; 
	}
    
    public static PwsEntryDTO fromPwsRecord(PwsRecord nextRecord) {
        PwsEntryDTO newEntry = new PwsEntryDTO();
        
        if (nextRecord instanceof PwsRecordV3) {
        	
        	PwsRecordV3 v3 = (PwsRecordV3) nextRecord;

            PwsUUIDField idField = (PwsUUIDField) v3.getField(PwsRecordV3.UUID);
            if (idField != null)
            	newEntry.setId((UUID) idField.getValue());
            
            String groupName = getSafeValue(v3, PwsRecordV3.GROUP);
            newEntry.setGroup(groupName);
            
            String title = getSafeValue(v3,PwsRecordV3.TITLE);
            newEntry.setTitle(title);
            
            String user = getSafeValue(v3, PwsRecordV3.USERNAME);
            newEntry.setUsername(user);
            
            String password = getSafeValue(v3,PwsRecordV3.PASSWORD);
            newEntry.setPassword(password);
            
            String notes = getSafeValue(v3,PwsRecordV3.NOTES);
            newEntry.setNotes(notes);

            String url = getSafeValue(v3,PwsRecordV3.URL);
            newEntry.setUrl(url);

            String autotype = getSafeValue(v3,PwsRecordV3.AUTOTYPE);
            newEntry.setAutotype(autotype);

            newEntry.setLastChange(getSafeDate(v3, PwsRecordV3.LAST_MOD_TIME));
            
            newEntry.setCreated(getSafeDate(v3, PwsRecordV3.CREATION_TIME));
 
            newEntry.setLastAccess(getSafeDate(v3, PwsRecordV3.LAST_ACCESS_TIME));
            
            newEntry.setLastPwChange(getSafeDate(v3, PwsRecordV3.PASSWORD_MOD_TIME));
            
           	newEntry.setExpires(getSafeDate(v3, PwsRecordV3.PASSWORD_LIFETIME));
            
            newEntry.setVersion("3");
            
            if (log.isDebugEnabled())
            	log.debug("PwsDTO created " + newEntry.toString());
        	
        } else if (nextRecord instanceof PwsRecordV2) {
            
            PwsRecordV2 v2 = (PwsRecordV2) nextRecord;
            
            String groupName = getSafeValue(v2, PwsRecordV2.GROUP);
            newEntry.setGroup(groupName);
            
            String title = getSafeValue(v2,PwsRecordV2.TITLE);
            newEntry.setTitle(title);
            
            String user = getSafeValue(v2, PwsRecordV2.USERNAME);
            newEntry.setUsername(user);
            
            String password = getSafeValue(v2,PwsRecordV2.PASSWORD);
            newEntry.setPassword(password);
            
            String notes = getSafeValue(v2,PwsRecordV2.NOTES);
            newEntry.setNotes(notes);
            
            newEntry.setVersion("2");
            
        } else {
            PwsRecordV1 v1 = (PwsRecordV1) nextRecord;
            
            String title = getSafeValue(v1,PwsRecordV1.TITLE);
            newEntry.setTitle(title);
            
            String user = getSafeValue(v1,PwsRecordV1.USERNAME);
            newEntry.setUsername(user);
            
            String password = getSafeValue(v1,PwsRecordV1.PASSWORD);
            newEntry.setPassword(password);
            
            String notes = getSafeValue(v1,PwsRecordV1.NOTES);
            newEntry.setNotes(notes);
            
            newEntry.setVersion("1");
        }
        return newEntry;
    }
    
    /**
     * Only set a date into a PwsTimeField if the date != null.  
     * @param v3
     * @param type
     * @param aDate
     * @return true if the date != null, else false 
     */
	private boolean setSafeDate(final PwsRecordV3 v3, final int type, final Date aDate) {
		if (aDate == null)
			return false;
		v3.setField(new PwsTimeField(type, aDate));
		return true;
	}

    
	/**
     * Moves the contents of the DTO into the supplied PwsRecord.
     * 
	 * @param nextRecord the record to place the data into
	 */
	public void toPwsRecord(PwsRecord nextRecord) {
		
		if (nextRecord instanceof PwsRecordV3) {
			
            PwsRecordV3 v3 = (PwsRecordV3) nextRecord;
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.GROUP , getGroup()));// + '\u0000'));
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.TITLE , getTitle()));
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.USERNAME , getUsername()));
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.PASSWORD , getPassword()));
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.NOTES , getNotes()));
            
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.URL , getUrl()));
            v3.setField(new PwsStringUnicodeField(PwsRecordV3.AUTOTYPE , getAutotype()));
            setSafeDate(v3,PwsRecordV3.LAST_ACCESS_TIME, getLastAccess());
            setSafeDate(v3,PwsRecordV3.LAST_MOD_TIME, getLastChange());
            setSafeDate(v3,PwsRecordV3.PASSWORD_MOD_TIME, getLastPwChange());
            setSafeDate(v3,PwsRecordV3.PASSWORD_LIFETIME, getExpires());
            // never changes, so dont't mess:
//          v3.setField(new PwsUUIDField(PwsRecordV3.UUID, getId()));
//          v3.setField(new PwsTimeField(PwsRecordV3.CREATION_TIME, getCreated()));

		} else if (nextRecord instanceof PwsRecordV2) {
            
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
