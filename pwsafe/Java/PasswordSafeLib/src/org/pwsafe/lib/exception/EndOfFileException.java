package org.pwsafe.lib.exception;

/**
 * 
 */
public class EndOfFileException extends Exception
{

	/**
	 * 
	 */
	public EndOfFileException()
	{
		super();
	}

	/**
	 * @param arg0
	 */
	public EndOfFileException(String arg0)
	{
		super(arg0);
	}

	/**
	 * @param arg0
	 */
	public EndOfFileException(Throwable arg0)
	{
		super(arg0);
	}

	/**
	 * @param arg0
	 * @param arg1
	 */
	public EndOfFileException(String arg0, Throwable arg1)
	{
		super(arg0, arg1);
	}
}
