<?xml version="1.0" encoding="ISO-8859-1"?><xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"><xsl:template match="/">
<!--
  Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
  All rights reserved. Use of the code is allowed under the
  Artistic License 2.0 terms, as specified in the LICENSE file
  distributed with this code, or available from
  http://www.opensource.org/licenses/artistic-license-2.0.php
-->

<html>
	<body>
		<h2>PasswordSafe Database</h2>
		<table border="1">
			<tr>
				<th align="left">Group</th>
				<th align="left">Title</th>
				<th align="left">Username</th>
				<th align="left">Password</th>
				<th align="left">URL</th>
				<th align="left">Autotype</th>
				<th align="left">Creation Time</th>
				<th align="left">Last Access Time</th>
				<th align="left">Password Expiry Time</th>
				<th align="left">Password Mod. Time</th>
				<th align="left">Record Mod. Time</th>
				<th align="left">Notes</th>
				<th align="left">Password History Status</th>
				<th align="left">Maximum Saved</th>
				<th align="left">Current Number</th>
			</tr>
			<xsl:for-each select="passwordsafe/entry">
				<xsl:sort select="group"/>
				<xsl:sort select="title"/>
				<xsl:sort select="username"/>
				<xsl:sort select="url"/>
				<xsl:sort select="ctime"/>
				<xsl:sort select="atime"/>
				<xsl:sort select="ltime"/>
				<xsl:sort select="pmtime"/>
				<xsl:sort select="rmtime"/>
				<xsl:sort select="notes"/>
				<tr>
					<td><xsl:value-of select="group"/></td>
					<td><xsl:value-of select="title"/></td>
					<td><xsl:value-of select="username"/></td>
					<td><xsl:value-of select="password"/></td>
					<td><xsl:value-of select="url"/></td>
					<td><xsl:value-of select="autotype"/></td>
					<td><xsl:value-of select="ctime"/></td>
					<td><xsl:value-of select="atime"/></td>
					<td><xsl:value-of select="ltime"/></td>
					<td><xsl:value-of select="pmtime"/></td>
					<td><xsl:value-of select="rmtime"/></td>
					<td><xsl:value-of select="notes"/></td>
					<td><xsl:value-of select="status"/></td>
					<td><xsl:value-of select="max"/></td>
					<td><xsl:value-of select="num"/></td>
				</tr>
			</xsl:for-each>
		</table>
	</body>
	</html>
</xsl:template></xsl:stylesheet>
