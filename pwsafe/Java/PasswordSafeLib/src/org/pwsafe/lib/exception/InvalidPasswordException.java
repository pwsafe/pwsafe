/*
 * Created on Dec 13, 2003
 *
 * To change the template for this generated file go to
 * Window - Preferences - Java - Code Generation - Code and Comments
 */
package org.pwsafe.lib.exception;

/**
 * @author Kevin
 *
 * To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Generation - Code and Comments
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
