/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.exception;

/**
 * A generic exception.
 * 
 * @author Kevin Preece
 */
public class PasswordSafeException extends Exception
{
	/**
	 * 
	 */
	public PasswordSafeException()
	{
		super();
	}

	/**
	 * @param arg0
	 */
	public PasswordSafeException(String arg0)
	{
		super(arg0);
	}

	/**
	 * @param arg0
	 */
	public PasswordSafeException(Throwable arg0)
	{
		super(arg0);
	}

	/**
	 * @param arg0
	 * @param arg1
	 */
	public PasswordSafeException(String arg0, Throwable arg1)
	{
		super(arg0, arg1);
	}
}
