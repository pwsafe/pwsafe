/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib;

import java.text.MessageFormat;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * A singleton class to help provide messages in the users preferred language.  Messages
 * are stored in a file called CorelibStrings.properties and, where translations have
 * been provided, the appropriate localised version, CorelibStrings_en_US.properties
 * for example.
 * 
 * @author Kevin Preece
 */
public class I18nHelper
{
	/**
	 * Log4j logger
	 */
	private static final Log LOG = Log.getInstance(I18nHelper.class.getPackage().getName());

	/**
	 * The localised message store.
	 */
	private static ResourceBundle	TheBundle	= null;

	/**
	 * The users preferred locale or the default locale if no preference was given.
	 */
	private static Locale			TheLocale	= Locale.getDefault();

	static
	{
		LOG.debug1( "I18Helper class loaded" );
	}

	/**
	 * Private for the singleton pattern. 
	 */
	private I18nHelper()
	{
	}

	/**
	 * Loads the localised strings using the current locale.
	 * 
	 * @return The localised <code>ResourceBundle</code>.
	 */
	private static synchronized ResourceBundle getBundle()
	{
		LOG.enterMethod( "getBundle" );

		if ( TheBundle == null )
		{
			LOG.debug1( "Loading resource bundle for locale " + TheLocale.toString() );
			TheBundle = ResourceBundle.getBundle( "CorelibStrings", TheLocale );
			// TODO handle the case where the file cannot be found
			// catch MissingResourceException
		}

		LOG.leaveMethod( "getBundle" );

		return TheBundle;
	}

	/**
	 * Sets the locale and forces the <code>ResourceBundle</code> to be reloaded.
	 * 
	 * @param locale the locale.
	 */
	public static synchronized void setLocale( Locale locale )
	{
		LOG.enterMethod( "setLocale" );

		TheLocale	= locale;
		TheBundle	= null;

		LOG.debug1( "Locale set to " + locale.toString() );

		LOG.leaveMethod( "setLocale" );
	}

	/**
	 * Returns the message with the given key from the <code>ResourceBundle</code>.
	 * 
	 * @param key the ID of the message to retrieve.
	 * 
	 * @return The message with the kiven key.
	 */
	public static String formatMessage( String key )
	{
		return formatMessage( key, null );
	}

	/**
	 * Returns the message with the given key from the <code>ResourceBundle</code>.  Where
	 * paramaters are specified in the message they are replaced with the appropriate entry
	 * from <code>args</code>.
	 * 
	 * @param key  the message ID
	 * @param args arguments for paramater substitutions.
	 * 
	 * @return The message with parameters substituted.
	 */
	public static String formatMessage( String key, Object [] args )
	{
		String	msg;

		msg = getBundle().getString(key);
		return (args == null) ? msg : key + " - " + MessageFormat.format( msg, args );
	}
}
