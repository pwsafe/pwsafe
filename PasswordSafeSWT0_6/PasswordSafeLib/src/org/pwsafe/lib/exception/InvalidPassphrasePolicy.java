/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.exception;

/**
 *
 */
public class InvalidPassphrasePolicy extends Exception
{
	/**
	 * 
	 */
	public InvalidPassphrasePolicy()
	{
		super();
	}

	/**
	 * @param arg0
	 */
	public InvalidPassphrasePolicy( String arg0 )
	{
		super( arg0 );
	}

	/**
	 * @param arg0
	 */
	public InvalidPassphrasePolicy( Throwable arg0 )
	{
		super( arg0 );
	}

	/**
	 * @param arg0
	 * @param arg1
	 */
	public InvalidPassphrasePolicy( String arg0, Throwable arg1 )
	{
		super( arg0, arg1 );
	}
}
