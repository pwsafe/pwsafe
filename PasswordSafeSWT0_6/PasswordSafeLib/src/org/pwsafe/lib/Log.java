/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib;

import org.apache.commons.logging.LogFactory;
//import org.apache.log4j.Logger;
//import org.apache.log4j.xml.DOMConfigurator;

/**
 * This class provides logging facilities using log4j.
 * 
 * @author Kevin Preece  
 */
public class Log
{
	private int	DebugLevel;
	private org.apache.commons.logging.Log	TheLogger;

	static
	{
		//DOMConfigurator.configure( "log-config.xml" );
	}

	private Log( String name )
	{
		//TheLogger	= Logger.getLogger( name );
		TheLogger = LogFactory.getLog(name);
		setDebugLevel( 3 );
	}

	/**
	 * Returns an instance of <code>Log</code> for the Log4j logger named <code>name</code>.
	 * 
	 * @param name the Log4j logger name.
	 * 
	 * @return An <code>Log</code> instance.
	 */
	public static Log getInstance( String name )
	{
		return new Log( name );
	}

	/**
	 * Writes a message at debug level 1
	 * 
	 * @param msg the message to issue.
	 */
	public void debug1( String msg )
	{
		if ( isDebug1Enabled() )
		{
			TheLogger.debug( msg );
		}
	}

	/**
	 * Writes a message at debug level 2
	 * 
	 * @param msg the message to issue.
	 */
	public void debug2( String msg )
	{
		if ( isDebug2Enabled() )
		{
			TheLogger.debug( msg );
		}
	}

	/**
	 * Writes a message at debug level 3
	 * 
	 * @param msg the message to issue.
	 */
	public void debug3( String msg )
	{
		if ( isDebug3Enabled() )
		{
			TheLogger.debug( msg );
		}
	}

	/**
	 * Writes a message at debug level 4
	 * 
	 * @param msg the message to issue.
	 */
	public void debug4( String msg )
	{
		if ( isDebug4Enabled() )
		{
			TheLogger.debug( msg );
		}
	}

	/**
	 * Writes a message at debug level 5
	 * 
	 * @param msg the message to issue.
	 */
	public void debug5( String msg )
	{
		if ( isDebug5Enabled() )
		{
			TheLogger.debug( msg );
		}
	}
	
	/**
	 * Logs entry to a method.
	 * 
	 * @param method the method name.
	 */
	public void enterMethod( String method )
	{
		if ( TheLogger.isDebugEnabled() )
		{	
			if ( !method.endsWith( ")" ) )
			{
				method = method + "()";
			}
			TheLogger.debug( "Entering method " + method );
		}
	}
	
	/**
	 * Writes a message at error level
	 * 
	 * @param msg the message to issue.
	 */
	public void error( String msg )
	{
		TheLogger.error( msg );
	}
	
	/**
	 * Writes a message at error level along with details of the exception
	 * 
	 * @param msg    the message to issue.
	 * @param except the exeption to be logged.
	 */
	public void error( String msg, Throwable except )
	{
		TheLogger.error( msg, except );
	}
	
	/**
	 * Logs the exception at a level of error.
	 * 
	 * @param except the <code>Exception</code> to log.
	 */
	public void error( Throwable except )
	{
		TheLogger.error( "An Exception has occurred", except );
	}

	/**
	 * Returns the current debug level.
	 * 
	 * @return Returns the debugLevel.
	 */
	public int getDebugLevel()
	{
		return DebugLevel;
	}

	/**
	 * Writes a message at info level
	 * 
	 * @param msg the message to issue.
	 */
	public void info( String msg )
	{
		TheLogger.info( msg );
	}
	
	/**
	 * Returns <code>true</code> if debuuging at level 1 is enabled, <code>false</code> if it isn't.
	 * 
	 * @return <code>true</code> if debuuging at level 1 is enabled, <code>false</code> if it isn't.
	 */
	public boolean isDebug1Enabled()
	{
		return TheLogger.isDebugEnabled();
	}

	/**
	 * Returns <code>true</code> if debuuging at level 2 is enabled, <code>false</code> if it isn't.
	 * 
	 * @return <code>true</code> if debuuging at level 2 is enabled, <code>false</code> if it isn't.
	 */
	public boolean isDebug2Enabled()
	{
		return TheLogger.isDebugEnabled() && (DebugLevel >= 2);
	}

	/**
	 * Returns <code>true</code> if debuuging at level 3 is enabled, <code>false</code> if it isn't.
	 * 
	 * @return <code>true</code> if debuuging at level 3 is enabled, <code>false</code> if it isn't.
	 */
	public boolean isDebug3Enabled()
	{
		return TheLogger.isDebugEnabled() && (DebugLevel >= 3);
	}

	/**
	 * Returns <code>true</code> if debuuging at level 4 is enabled, <code>false</code> if it isn't.
	 * 
	 * @return <code>true</code> if debuuging at level 4 is enabled, <code>false</code> if it isn't.
	 */
	public boolean isDebug4Enabled()
	{
		return TheLogger.isDebugEnabled() && (DebugLevel >= 4);
	}

	/**
	 * Returns <code>true</code> if debuuging at level 5 is enabled, <code>false</code> if it isn't.
	 * 
	 * @return <code>true</code> if debuuging at level 5 is enabled, <code>false</code> if it isn't.
	 */
	public boolean isDebug5Enabled()
	{
		return TheLogger.isDebugEnabled() && (DebugLevel >= 5);
	}

	/**
	 * Logs exit from a method.
	 * 
	 * @param method the method name.
	 */
	public void leaveMethod( String method )
	{
		if ( TheLogger.isDebugEnabled() )
		{	
			if ( !method.endsWith( ")" ) )
			{
				method = method + "()";
			}
			TheLogger.debug( "Leaving method " + method );
		}
	}
	
	/**
	 * Sets the debug level.
	 * 
	 * @param debugLevel The debugLevel to set.
	 */
	public void setDebugLevel( int debugLevel )
	{
		if ( debugLevel < 1 )
		{
			debugLevel = 1;
		}
		else if ( debugLevel > 5 )
		{
			debugLevel = 5;
		}
		DebugLevel = debugLevel;
	}

	/**
	 * Logs a message at the warning level.
	 * 
	 * @param msg the message to issue.
	 */
	public void warn( String msg )
	{
		TheLogger.warn( msg );
	}

}
