package org.pwsafe.lib.exception;

/**
 * @author Kevin Preece
 */
public class UnimplementedConversionException extends RuntimeException
{

	/**
	 * 
	 */
	public UnimplementedConversionException()
	{
		super();
	}

	/**
	 * @param arg0
	 */
	public UnimplementedConversionException(String arg0)
	{
		super(arg0);
	}

	/**
	 * @param arg0
	 */
	public UnimplementedConversionException(Throwable arg0)
	{
		super(arg0);
	}

	/**
	 * @param arg0
	 * @param arg1
	 */
	public UnimplementedConversionException(String arg0, Throwable arg1)
	{
		super(arg0, arg1);
	}
}
