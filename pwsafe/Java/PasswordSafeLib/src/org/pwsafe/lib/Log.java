package org.pwsafe.lib;

import org.apache.log4j.Logger;
import org.apache.log4j.xml.DOMConfigurator;

/**
 * This class provides logging facilities using log4j.  
 */
public class Log
{
	private int		DebugLevel;
	private Logger	TheLogger;

	static
	{
		DOMConfigurator.configure( "log-config.xml" );
	}

	private Log( String name )
	{
		TheLogger	= Logger.getLogger( name );
		setDebugLevel( 3 );
	}
	
	public static Log getInstance( String name )
	{
		return new Log( name );
	}
	
	public void debug1( String msg )
	{
		if ( isDebug1Enabled() )
		{
			TheLogger.debug( msg );
		}
	}

	public void debug2( String msg )
	{
		if ( isDebug2Enabled() )
		{
			TheLogger.debug( msg );
		}
	}

	public void debug3( String msg )
	{
		if ( isDebug3Enabled() )
		{
			TheLogger.debug( msg );
		}
	}

	public void debug4( String msg )
	{
		if ( isDebug4Enabled() )
		{
			TheLogger.debug( msg );
		}
	}

	public void debug5( String msg )
	{
		if ( isDebug5Enabled() )
		{
			TheLogger.debug( msg );
		}
	}
	
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
	
	public void error( String msg )
	{
		TheLogger.error( msg );
	}
	
	public void error( String msg, Throwable except )
	{
		TheLogger.error( msg, except );
	}
	
	public void error( Throwable except )
	{
		TheLogger.error( "An Exception has occurred", except );
	}

	/**
	 * @return Returns the debugLevel.
	 */
	public int getDebugLevel()
	{
		return DebugLevel;
	}

	public void info( String msg )
	{
		TheLogger.info( msg );
	}
	
	public boolean isDebug1Enabled()
	{
		return TheLogger.isDebugEnabled();
	}

	public boolean isDebug2Enabled()
	{
		return TheLogger.isDebugEnabled() && (DebugLevel >= 2);
	}

	public boolean isDebug3Enabled()
	{
		return TheLogger.isDebugEnabled() && (DebugLevel >= 3);
	}

	public boolean isDebug4Enabled()
	{
		return TheLogger.isDebugEnabled() && (DebugLevel >= 4);
	}

	public boolean isDebug5Enabled()
	{
		return TheLogger.isDebugEnabled() && (DebugLevel >= 5);
	}

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

	public void warn( String msg )
	{
		TheLogger.warn( msg );
	}

}
