package org.pwsafe.lib.file;

import org.pwsafe.lib.Log;
import org.pwsafe.lib.exception.UnimplementedConversionException;

/**
 * A record in PasswordSafe consists of a number of fields.  In V1.7 files all fields are
 * strings but in V2.0 they can be of type String, UUID, time_t (a 32-bit integer holding
 * the number of milliseconds since 00:00:00 on 1st January 1970) or integer.
 * <p />
 * This class is abstract because each subclass, although sharing similar features,
 * has unique characteristics that are best handled with polymorphism.  This allows 
 * easier support for new field types should new ones be introduced in later versions
 * of the program.
 * <p />
 * @author Kevin Preece
 */
public abstract class PwsField implements Comparable
{
	private static final Log LOG = Log.getInstance(PwsFileFactory.class.getPackage().getName());

	private Object	Value	= null;
	private int		Type	= 0;

	/**
	 * 
	 */
	protected PwsField( int type, Object value )
	{
		super();

		Type	= type;
		Value	= value;
	}

	/**
	 * Converts this field into an array of bytes suitable for writing to a PasswordSafe file.
	 * This method must be overridden by subclasses otherwise an <code>UnimplementedConversionException</code>
	 * runtime exception is thrown.
	 *  
	 * @return
	 * 
	 * @throws UnimplementedConversionException
	 */
	public byte [] getBytes()
	{
		throw new UnimplementedConversionException(); 
	}

	public int getType()
	{
		return Type;
	}

	protected Object getValue()
	{
		return Value;
	}

	/**
	 * @see java.lang.Object#hashCode()
	 */
	public int hashCode()
	{
		return Value.hashCode();
	}

	public String toString()
	{
		return Value.toString();
	}
}
