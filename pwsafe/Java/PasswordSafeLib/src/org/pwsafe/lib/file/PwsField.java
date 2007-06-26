/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.lib.file;


/**
 * A record in PasswordSafe consists of a number of fields.  In V1.7 files all fields are
 * strings but in V2.0 they can be of type String, UUID, time_t (a 32-bit integer holding
 * the number of milliseconds since 00:00:00 on 1st January 1970) or integer.
 * </p><p>
 * This class is abstract because each subclass, although sharing similar features,
 * has unique characteristics that are best handled with polymorphism.  This allows 
 * easier support for new field types should new ones be introduced in later versions
 * of the program.
 * </p>
 * 
 * @author Kevin Preece
 */
public abstract class PwsField implements Comparable
{
//	private static final Log LOG = Log.getInstance(PwsField.class.getPackage().getName());

	private Object	Value	= null;
	private final int Type;

	/**
	 * Creates the field object.
	 * 
	 * @param type  the field type.
	 * @param value the field value.
	 */
	protected PwsField( int type, Object value )
	{
		super();

		Type	= type;
		Value	= value;
	}

	/**
	 * Converts this field into an array of bytes suitable for writing to a PasswordSafe file.
	 * 
	 * @return The field as a byte array.
	 */
	public abstract byte [] getBytes();

	/**
	 * Returns the field type given when the object was created.  The field type is specific
	 * to the subclass of {@link PwsRecord} that this field belongs to.
	 * 
	 * @return the field type.
	 */
	public int getType()
	{
		return Type;
	}

	/**
	 * Returns the field value in its native form.
	 * 
	 * @return The field value in its native form.
	 */
	public Object getValue()
	{
		return Value;
	}

	/**
	 * Returns a hash code for this object.
	 * 
	 * @return the hash code for the object.
	 */
	public int hashCode()
	{
		return Value.hashCode();
	}

	/**
	 * Returns the string value of the field.
	 * 
	 * @return The string value of the field.
	 */
	public String toString()
	{
		return Value.toString();
	}
}
