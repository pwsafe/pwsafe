/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib;

import java.text.MessageFormat;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 *
 */
public class I18nHelperBase
{
	/**
	 * Log4j logger
	 */
	private static final Log		LOG	= Log.getInstance( I18nHelperBase.class.getPackage().getName() );

	/**
	 * The localised message store.
	 */
	private static ResourceBundle	TheBundle	= null;

	/**
	 * The users preferred locale or the default locale if no preference was given.
	 */
	private static Locale			TheLocale	= Locale.getDefault();

	/**
	 * 
	 */
	protected I18nHelperBase()
	{
		super();
		// TODO Auto-generated constructor stub
	}

	static
	{
		LOG.debug1( "I18nHelper class loaded" );
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
	public String formatError( String key, Object [] args )
	{
		String	msg;

		msg = getString(key);
		return (args == null) ? msg : key + " - " + MessageFormat.format( msg, args );
	}

	/**
	 * Returns the message with the given key from the <code>ResourceBundle</code>.
	 * 
	 * @param key the ID of the message to retrieve.
	 * 
	 * @return The message with the kiven key.
	 */
	public String formatMessage( String key )
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
	public String formatMessage( String key, Object [] args )
	{
		String	msg;

		msg = getString(key);
		return (args == null) ? msg : MessageFormat.format( msg, args );
	}

	/**
	 * Loads the localised strings using the current locale.
	 * 
	 * @return The localised <code>ResourceBundle</code>.
	 */
	private synchronized ResourceBundle getBundle()
	{
		LOG.enterMethod( "getBundle" );

		if ( TheBundle == null )
		{
			LOG.debug1( "Loading resource bundle for locale " + TheLocale.toString() );
			TheBundle = ResourceBundle.getBundle( getFilename(), TheLocale );
			// TODO handle the case where the file cannot be found
			// catch MissingResourceException
		}

		LOG.leaveMethod( "getBundle" );

		return TheBundle;
	}

	/**
	 * Returns the base name of the properties file that contains the localised
	 * strings.
	 * 
	 * @return
	 */
	public String getFilename()
	{
		return "CorelibStrings";
	}

	private String getString( String key )
	{
		try
		{
			return getBundle().getString( key );
		}
		catch ( MissingResourceException e )
		{
			// N.B. Special case - this message is not loaded from the resource file.
			LOG.error( getFilename() + ".properties : Missing Resource - \"" + key + "\"" );
		}
		return key + " (value not found)";
	}

	/**
	 * Sets the locale and forces the <code>ResourceBundle</code> to be reloaded.
	 * 
	 * @param locale the locale.
	 */
	public synchronized void setLocale( Locale locale )
	{
		LOG.enterMethod( "setLocale" );

		TheLocale	= locale;
		TheBundle	= null;

		LOG.debug1( "Locale set to " + locale.toString() );

		LOG.leaveMethod( "setLocale" );
	}
}
