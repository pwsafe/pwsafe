/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib;

/**
 * A singleton class to help provide messages in the users preferred language.  Messages
 * are stored in a file called CorelibStrings.properties and, where translations have
 * been provided, the appropriate localised version, CorelibStrings_en_US.properties
 * for example.
 * 
 * @author Kevin Preece
 */
public class I18nHelper extends I18nHelperBase
{
	/**
	 * Log4j logger
	 */
	private static final Log		LOG			= Log.getInstance(I18nHelper.class.getPackage().getName());

	private static final I18nHelper	TheInstance	= new I18nHelper();

	static
	{
		LOG.debug1( "I18nHelper class loaded" );
	}

	/**
	 * Private for the singleton pattern. 
	 */
	private I18nHelper()
	{
	}

	/**
	 * Returns an instance of I18nHelper.
	 * 
	 * @return An instance of I18nHelper.
	 */
	public static I18nHelper getInstance()
	{
		return TheInstance;
	}
}
