/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.util;

import org.pwsafe.lib.file.PwsRecord;
import org.pwsafe.lib.file.PwsRecordV1;
import org.pwsafe.lib.file.PwsRecordV2;

/**
 *
 */
public class DefaultRecordWrapper
{
	private PwsRecord	Record;

	/**
	 * 
	 * @param rec
	 */
	public DefaultRecordWrapper( PwsRecord rec )
	{
		Record	= rec;
	}

	/**
	 * 
	 * @return
	 */
	public PwsRecord getRecord()
	{
		return Record;
	}

	/**
	 * 
	 * 
	 * @return
	 */
	public String toString()
	{
		if ( Record instanceof PwsRecordV1 )
		{
			return ((PwsRecordV1) Record).getField(PwsRecordV1.TITLE).toString();
		}
		else if ( Record instanceof PwsRecordV2 )
		{
			return ((PwsRecordV2) Record).getField(PwsRecordV2.TITLE).toString();
		}
		return null;
	}
}
