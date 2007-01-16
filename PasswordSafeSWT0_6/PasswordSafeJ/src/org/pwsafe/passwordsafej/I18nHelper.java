/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.passwordsafej;

import org.pwsafe.lib.I18nHelperBase;

/**
 *
 * @author Kevin Preece
 */
public class I18nHelper extends I18nHelperBase
{
	private static final I18nHelper	TheInstance	= new I18nHelper();

	private I18nHelper()
	{
		super();
	}

	/**
	 * 
	 * @return
	 * 
	 * @see org.pwsafe.lib.I18nHelper#getFilename()
	 */
	public String getFilename()
	{
		return "PasswordSafeJUI";
	}

	/**
	 * Returns an instance of this I18nHelper class.
	 * 
	 * @return An instance of this I18nHelper class.
	 */
	public static I18nHelper getInstance()
	{
		return TheInstance;
	}
}
