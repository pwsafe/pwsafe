/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.util;

/**
 * This class defines a policy that will be used to generate a random password.
 * The policy defines how long the generated password should be and which
 * character classes it should contain.  The character classes are:
 * <p>
 * <sl>
 *   <li>Upper case letters,
 *   <li>Lowercase letters,
 *   <li>Digits and certain symbol characters.
 * </sl>
 * </p><p>
 * In addition it also specifies whether certain confusable characters should
 * be removed from the password.  These are characters such as '1' and 'I'.
 */
public class PassphrasePolicy
{
	/**
	 * <code>true</code> if generated password should contain lowercase characters.
	 * The default is <code>true</code>.
	 */
	public boolean	LowercaseChars	= true;

	/**
	 * <code>true</code> if generated password should contain uppercase characters.
	 * The default is <code>true</code>.
	 */
	public boolean	UppercaseChars	= true;

	/**
	 * <code>true</code> if generated password should contain digit characters.
	 * The default is <code>true</code>.
	 */
	public boolean	DigitChars		= true;

	/**
	 * <code>true</code> if the generated password should contain symbol characters.
	 * The default is <code>true</code>.
	 */
	public boolean	SymbolChars		= true;

	/**
	 * <code>true</code> if the generated password should not contain confusable characters.
	 * The default is <code>false</code>.
	 */
	public boolean	Easyview		= false;

	/**
	 * The length of the generated password.  The default is 8.
	 */
	public int		Length			= 8;

	/**
	 * Creates a password policy with fairly strong deafults.  If unaltered
	 * this policy will cause a password to be generated that is 8 characters
	 * long with at least one of each character class, i.e. at least one
	 * uppercase, lowercase, digit, and symbol characters.
	 */
	public PassphrasePolicy()
	{
		super();
	}

	/**
	 * Checks that it is possible to generate a password using this policy.  Returns
	 * <code>true</code> if at least one character category is selected and the password
	 * length is equal to or greater than the number of classes selected.
	 * 
	 * @return
	 */
	public boolean isValid()
	{
		int		count	= 0;

		if ( LowercaseChars )	++count;
		if ( UppercaseChars )	++count;
		if ( DigitChars )		++count;
		if ( SymbolChars )		++count;

		if ( (count > 0) && (Length >= count) )
		{
			return true;
		}
		return false;
	}

	/**
	 * Returns a <code>String</code> representation of the object.
	 * 
	 * @return A <code>String</code> representation of the object.
	 */
	public String toString()
	{
		StringBuffer	sb;

		sb = new StringBuffer();

		sb.append( "PassphrasePolicy{ Length=" );
		sb.append( Length );
		sb.append( ", Uppercase=" );
		sb.append( UppercaseChars );
		sb.append( ", Lowercase=" );
		sb.append( LowercaseChars );
		sb.append( ", Digits=" );
		sb.append( DigitChars );
		sb.append( ", Symbols=" );
		sb.append( SymbolChars );
		sb.append( ", Easyview=" );
		sb.append( Easyview );
		sb.append( " }" );

		return sb.toString();
	}
}
