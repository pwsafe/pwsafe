/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.exception;

/**
 * An exception thrown to indicate that the file is in a format that is not supported
 * by this software.
 * 
 * @author Kevin Preece
 */
public class UnsupportedFileVersionException extends Exception
{

	/**
	 * 
	 */
	public UnsupportedFileVersionException()
	{
		super();
	}

	/**
	 * @param arg0
	 */
	public UnsupportedFileVersionException(String arg0)
	{
		super(arg0);
	}

	/**
	 * @param arg0
	 */
	public UnsupportedFileVersionException(Throwable arg0)
	{
		super(arg0);
	}

	/**
	 * @param arg0
	 * @param arg1
	 */
	public UnsupportedFileVersionException(String arg0, Throwable arg1)
	{
		super(arg0, arg1);
	}
}
