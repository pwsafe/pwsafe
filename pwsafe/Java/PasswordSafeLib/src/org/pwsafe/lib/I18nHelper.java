package org.pwsafe.lib;

import java.text.MessageFormat;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Generation - Code and Comments
 */
public class I18nHelper
{
	private static final Log LOG = Log.getInstance(I18nHelper.class.getPackage().getName());

	private static ResourceBundle	TheBundle	= null;
	private static Locale			TheLocale	= Locale.getDefault();

	static
	{
		LOG.debug1( "I18Helper class loaded" );
	}

	/**
	 * 
	 */
	private I18nHelper()
	{
	}

	private static synchronized ResourceBundle getBundle()
	{
		LOG.enterMethod( "getBundle" );

		if ( TheBundle == null )
		{
			TheBundle = ResourceBundle.getBundle( "CorelibStrings", TheLocale );
			// TODO handle the case where the file cannot be found
			// catch MissingResourceException
		}

		LOG.leaveMethod( "getBundle" );

		return TheBundle;
	}
	
	public static synchronized void setLocale( Locale locale )
	{
		LOG.enterMethod( "setLocale" );

		TheLocale	= locale;
		TheBundle	= null;

		LOG.leaveMethod( "setLocale" );
	}
	
	public static String formatMessage( String key )
	{
		return formatMessage( key, null );
	}
	
	public static String formatMessage( String key, Object [] args )
	{
		String	msg;

		msg = getBundle().getString(key);
		return (args == null) ? msg : key + " - " + MessageFormat.format( msg, args );
	}
}
