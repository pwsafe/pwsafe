package org.pwsafe.lib.exception;

/**
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
