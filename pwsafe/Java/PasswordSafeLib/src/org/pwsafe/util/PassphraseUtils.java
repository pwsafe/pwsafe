/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.util;

import org.pwsafe.lib.I18nHelper;
import org.pwsafe.lib.Log;
import org.pwsafe.lib.Util;
import org.pwsafe.lib.exception.InvalidPassphrasePolicy;

/**
 *
 */
public class PassphraseUtils
{
	/**
	 * Log4j logger
	 */
	private static final Log LOG = Log.getInstance(PassphraseUtils.class.getPackage().getName());

	/**
	 * Standard lowercase characters.
	 */
	public static final char []	LOWERCASE_CHARS			= "abcdefghijklmnopqrstuvwxyz".toCharArray();

	/**
	 * Standard uppercase characters.
	 */
	public static final char []	UPPERCASE_CHARS			= "ABCDEFGHIJKLMNOPQRSTUVWXYZ".toCharArray();

	/**
	 * Standard digit characters.
	 */
	public static final char []	DIGIT_CHARS				= "1234567890".toCharArray();

	/**
	 * Standard symbol characters.
	 */
	public static final char []	SYMBOL_CHARS			= "+-=_@#$%^&;:,.<>/~\\[](){}?!|".toCharArray();

	/**
	 * Lowercase characters with confusable characters removed.
	 */
	public static final char []	EASYVISION_LC_CHARS		= "abcdefghijkmnopqrstuvwxyz".toCharArray();

	/**
	 * Uppercase characters with confusable characters removed.
	 */
	public static final char []	EASYVISION_UC_CHARS		= "ABCDEFGHJKLMNPQRTUVWXY".toCharArray();

	/**
	 * Digit characters with confusable characters removed.
	 */
	public static final char []	EASYVISION_DIGIT_CHARS	= "346789".toCharArray();

	/**
	 * Symbol characters with confusable characters removed.
	 */
	public static final char []	EASYVISION_SYMBOL_CHARS	= "+-=_@#$%^&<>/~\\?".toCharArray();

	/**
	 * The minimum length that a password must be to be not weak.
	 */
	public static final int		MIN_PASSWORD_LEN		= 4;

	/**
	 * Private for singleton pattern
	 */
	private PassphraseUtils()
	{
		super();
	}

	/**
	 * Generates a new random password according to the policy supplied.
	 * 
	 * @param policy the {@link PassphrasePolicy} policy
	 * 
	 * @return A new random password.
	 * 
	 * @throws InvalidPassphrasePolicy
	 */
	public static String makePassword( PassphrasePolicy policy )
	throws InvalidPassphrasePolicy
	{
		LOG.enterMethod( "makePassword" );

		LOG.debug2( policy.toString() );

		char			allChars [][];
		boolean			typesSeen[];
		StringBuffer	password;
		int				typeCount;

		if ( !policy.isValid() )
		{
			LOG.info( I18nHelper.getInstance().formatMessage("I0004", new Object [] { policy.toString() } ) );
			throw new InvalidPassphrasePolicy();
		}

		password	= new StringBuffer( policy.Length );
		typeCount	= 0;

		if ( policy.DigitChars )	++typeCount;
		if ( policy.LowercaseChars)	++typeCount;
		if ( policy.UppercaseChars)	++typeCount;
		if ( policy.SymbolChars)	++typeCount;

		allChars	= new char[ typeCount ][];
		typesSeen	= new boolean[ 4 ];

		for ( int ii = 0; ii < typeCount; ++ii )
		{
			typesSeen[ ii ] = true;
		}

		if ( policy.Easyview )
		{
			int	ii	= 0;

			if ( policy.DigitChars )	allChars[ ii++ ] = EASYVISION_DIGIT_CHARS;
			if ( policy.LowercaseChars)	allChars[ ii++ ] = EASYVISION_LC_CHARS;
			if ( policy.UppercaseChars)	allChars[ ii++ ] = EASYVISION_UC_CHARS;
			if ( policy.SymbolChars)	allChars[ ii++ ] = EASYVISION_SYMBOL_CHARS;
		}
		else
		{
			int	ii	= 0;

			if ( policy.DigitChars )	allChars[ ii++ ] = DIGIT_CHARS;
			if ( policy.LowercaseChars)	allChars[ ii++ ] = LOWERCASE_CHARS;
			if ( policy.UppercaseChars)	allChars[ ii++ ] = UPPERCASE_CHARS;
			if ( policy.SymbolChars)	allChars[ ii++ ] = SYMBOL_CHARS;
		}

		do
		{
			password.delete( 0, password.length() );

			for ( int ii = 0; ii < policy.Length; ++ii )
			{
				int	type;
	
				type				= Util.positiveRand() % typeCount;
				typesSeen[ type ]	= false;
	
				password.append( allChars[type][ Util.positiveRand() % allChars[type].length ] );
			}
		}
		while ( typesSeen[0] || typesSeen[1] || typesSeen[2] || typesSeen[3] );

		LOG.debug2( "Generated password is " + password.toString() );

		LOG.leaveMethod( "makePassword" );

		return password.toString();
	}

	/**
	 * Checks the password against a set of rules to determine whether it is
	 * considered weak.  The rules are:
	 * </p><p>
	 * <ul>
	 *   <li>It is at least <code>MIN_PASSWORD_LEN</code> characters long.
	 *   <li>At least one lowercase character.
	 *   <li>At least one uppercase character.
	 *   <li>At least one digit or symbol character.
	 * </ul>
	 * 
	 * @param password the password to check.
	 * 
	 * @return <code>true</code> if the password is considered to be weak,
	 *         <code>false</code> otherwise.
	 */
	public static boolean isWeakPassword( String password )
	{
		boolean	hasUC		= false;
		boolean	hasLC		= false;
		boolean	hasDigit	= false;
		boolean	hasSymbol	= false;

		if ( password.length() < MIN_PASSWORD_LEN )
		{
			return true;
		}

		for ( int ii = 0; ii < password.length(); ++ii )
		{
			char	c;

			c = password.charAt( ii );

			if ( Character.isDigit(c) )				hasDigit	= true;
			else if ( Character.isUpperCase(c) )	hasUC		= true;
			else if ( Character.isLowerCase(c) )	hasLC		= true;
			else 									hasSymbol	= true;
		}

		if ( hasUC && hasLC && (hasDigit || hasSymbol) )
		{
			return false;
		}
		return true;
	}
}
