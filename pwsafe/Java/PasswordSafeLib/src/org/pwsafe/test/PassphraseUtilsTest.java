/*
 * $Id$
 * 
 * This file is provided under the standard terms of the Artistic Licence.  See the
 * LICENSE file that comes with this package for details.
 */
package org.pwsafe.test;

import org.pwsafe.lib.exception.InvalidPassphrasePolicy;
import org.pwsafe.util.PassphrasePolicy;
import org.pwsafe.util.PassphraseUtils;

import junit.framework.TestCase;

/**
 * A JUnit test case that tests the public methods of {@link PassphraseUtils}
 */
public class PassphraseUtilsTest extends TestCase
{
	/**
	 *
	 */
	public final void testMakePassword1()
	{
		makePassword( new PassphrasePolicy() );
	}

	/**
	 *
	 */
	public final void testMakePassword2()
	{
		PassphrasePolicy	policy;

		policy = new PassphrasePolicy();

		policy.Length	= 17;

		makePassword( policy );
	}

	/**
	 *
	 */
	public final void testMakePassword3()
	{
		PassphrasePolicy	policy;

		policy = new PassphrasePolicy();

		policy.Length			= 64;
		policy.LowercaseChars	= false;
		policy.DigitChars		= false;
		policy.SymbolChars		= false;

		makePassword( policy );
	}

	/**
	 *
	 */
	public final void testMakePassword4()
	{
		PassphrasePolicy	policy;

		policy = new PassphrasePolicy();

		policy.Length			= 1;
		policy.LowercaseChars	= false;
		policy.DigitChars		= false;
		policy.SymbolChars		= false;

		makePassword( policy );
	}

	/**
	 *
	 */
	public final void testIsWeakPassword1()
	{
		assertTrue( ! PassphraseUtils.isWeakPassword("aB\\q") );
	}

	/**
	 *
	 */
	public final void testIsWeakPassword2()
	{
		assertTrue( PassphraseUtils.isWeakPassword("aB\\") );
	}

	/**
	 *
	 */
	public final void testIsWeakPassword3()
	{
		assertTrue( ! PassphraseUtils.isWeakPassword("Th1s is not a weak password.") );
	}

	/**
	 *
	 */
	public final void testIsWeakPassword4()
	{
		assertTrue( PassphraseUtils.isWeakPassword("this is a weak password") );
	}

	/**
	 * Runs a single password policy test.
	 * 
	 * @param policy the password policy to use.
	 */
	private final void makePassword( PassphrasePolicy policy )
	{
		try
		{
			boolean	digitSeen;
			boolean	ucCharSeen;
			boolean	lcCharSeen;
			boolean	symbolSeen;
			String	passphrase;

			passphrase	= PassphraseUtils.makePassword( policy );
			digitSeen	= false;
			ucCharSeen	= false;
			lcCharSeen	= false;
			symbolSeen	= false;

			assertEquals( "Generated password is the wrong length", passphrase.length(), policy.Length );

			for ( int ii = 0; ii < passphrase.length(); ++ii )
			{
				char	c;

				c			= passphrase.charAt( ii );

				if ( Character.isDigit(c) )				digitSeen	= true;
				else if ( Character.isUpperCase(c) )	ucCharSeen	= true;
				else if ( Character.isLowerCase(c) )	lcCharSeen	= true;
				else									symbolSeen	= true;
			}
			
			assertTrue(
				"Password doesn\'t contain at least one character from each required category",
				!( (ucCharSeen ^ policy.UppercaseChars) | (lcCharSeen ^ policy.LowercaseChars) | (digitSeen ^ policy.DigitChars) | (symbolSeen ^ policy.SymbolChars) ) );
		}
		catch ( InvalidPassphrasePolicy e )
		{
			fail( "Passphrase policy is invalid" );
		}
	}
}
