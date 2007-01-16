/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.exception;

/**
 * An exception class to indicate when end-of-file is reached.
 * 
 * @author Kevin Preece 
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
