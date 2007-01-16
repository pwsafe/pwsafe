/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.exception;

/**
 * An exception to indicate that the passphrase given for a file is incorrect.
 * 
 * @author Kevin Preece
 */
public class InvalidPassphraseException extends Exception
{
	/**
	 * 
	 */
	public InvalidPassphraseException()
	{
		super();
	}

	/**
	 * @param message
	 */
	public InvalidPassphraseException(String message)
	{
		super(message);
	}

	/**
	 * @param message
	 * @param cause
	 */
	public InvalidPassphraseException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * @param cause
	 */
	public InvalidPassphraseException(Throwable cause)
	{
		super(cause);
	}
}
