/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.exception;

/**
 * An exception to indicate that the password given for a file is incorrect.
 * 
 * @author Kevin Preece
 */
public class InvalidPasswordException extends Exception
{
	/**
	 * 
	 */
	public InvalidPasswordException()
	{
		super();
	}

	/**
	 * @param message
	 */
	public InvalidPasswordException(String message)
	{
		super(message);
	}

	/**
	 * @param message
	 * @param cause
	 */
	public InvalidPasswordException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * @param cause
	 */
	public InvalidPasswordException(Throwable cause)
	{
		super(cause);
	}
}
