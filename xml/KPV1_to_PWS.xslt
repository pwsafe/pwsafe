<?xml version="1.0" encoding="UTF-8"?>
<!--
  Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
  All rights reserved. Use of the code is allowed under the
  Artistic License 2.0 terms, as specified in the LICENSE file
  distributed with this code, or available from
  http://www.opensource.org/licenses/artistic-license-2.0.php

  This file will convert XML files exported by KeePass V1 (tested with KeePass v1.19b)
  into the equivalent XML files that can be imported into PasswordSafe V3.26 or later.

  The delimiter attribute in the 'passwordsafe' element is set to "right angle quotation
  marks" and any carriage returns/line feeds in the Notes field will be converted this
  character to enable the Notes field to be processed correctly by PasswordSafe during import.

  KeePass exports the Group Tree structure using a backward slash('\') as a delimiter
  between groups.  PasswordSafe uses a dot ('.').  Therefore this XSLT code first
  changes any dot in the group or title files to a forward slash ('/') and then any
  backward slashes to dots.  This will create the correct group path in PasswordSafe but
  any dots in the original group and title fields will remain a forward slash.

  This XSLT file conforms to V1.0 of XSLT as described in http://www.w3.org/TR/xslt

  During testing under Windows,
  a. It appears that the old command line program from Microsoft (msxml.exe V1.1.0.1
     dated 2001) cannot process this XSLT file.
  b. The command line progam NSLT2 (V2.2 dated 2005) can correctly process this XSLT
     file but it no longer seems to be under active development and, although can be
     found on the web, the home web site no longer allows downloads.
  c. AltovaXML Community Edition (current version 2011r2sp1) can also correctly process
     this XSLT file. See http://www.altova.com/altovaxml.html.
  d. Saxon-HE (Home Edition) (current version 9.3.0.5) may support this XSLT file but
     has not yet been tested. See http://saxon.sourceforge.net/.

  All four programs are free.
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
         xmlns:xs="http://www.w3.org/2001/XMLSchema" exclude-result-prefixes="xs">
  <xsl:output method="xml" encoding="UTF-8" indent="yes"
         cdata-section-elements="group title password username url notes uuid"/>
  <xsl:template match="/">
    <passwordsafe delimiter="&#0187;">
      <xsl:attribute name="xsi:noNamespaceSchemaLocation"
            namespace="http://www.w3.org/2001/XMLSchema-instance">pwsafe.xsd</xsl:attribute>
      <xsl:for-each select="pwlist/pwentry">
        <xsl:variable name="vgroup" select="translate(group, '.', '/')"/>
        <xsl:variable name="vtemp1" select="string(group/@tree)"/>
        <xsl:variable name="vtemp2" select="translate($vtemp1, '.', '/')"/>
        <xsl:variable name="vgrouptree" select="translate($vtemp2, '\', '.')"/>
        <xsl:variable name="vtitle" select="translate(title, '.', '/')"/>
        <xsl:variable name="vnotes" select="notes"/>
        <xsl:variable name="vexpiretime" select="expiretime"/>
        <xsl:variable name="vexpires" select="string($vexpiretime/@expires)"/>
        <xsl:variable name="vlastmodtime" select="string(lastmodtime)"/>
        <entry>
          <xsl:attribute name="id">
            <xsl:value-of select="position()"/>
          </xsl:attribute>
          <xsl:choose>
            <xsl:when test="string(normalize-space($vgrouptree)) != ''">
            <xsl:variable name="vgroups" select="concat($vgrouptree, '.', $vgroup)"/>
            <group>
              <xsl:value-of select="string($vgroups)"/>
            </group>
            </xsl:when>
            <xsl:otherwise>
            <group>
              <xsl:value-of select="string($vgroup)"/>
            </group>
            </xsl:otherwise>
          </xsl:choose>
          <title>
            <xsl:value-of select="string($vtitle)"/>
          </title>
          <username>
            <xsl:value-of select="string(username)"/>
          </username>
          <password>
            <xsl:value-of select="string(password)"/>
          </password>
          <url>
            <xsl:value-of select="string(url)"/>
          </url>
          <notes>
            <xsl:variable name="vnotes_nocrlf">
              <xsl:call-template name="string-replace-all">
                <xsl:with-param name="text" select="$vnotes"/>
                <xsl:with-param name="replace" select="'\r\n'"/>
                <xsl:with-param name="by" select="'&#0187;'"/>
              </xsl:call-template>
            </xsl:variable>
            <xsl:value-of select="string($vnotes_nocrlf)"/>
          </notes>
          <uuid>
            <xsl:value-of select="string(uuid)"/>
          </uuid>
          <ctimex>
            <xsl:value-of select="string(creationtime)"/>
          </ctimex>
          <atimex>
            <xsl:value-of select="string(lastaccesstime)"/>
          </atimex>
          <xsl:if test="string(((normalize-space($vexpires) = 'true') or (normalize-space($vexpires) = '1'))) != 'false'">
            <xtimex>
              <xsl:value-of select="string($vexpiretime)"/>
            </xtimex>
          </xsl:if>
          <pmtimex>
            <xsl:value-of select="$vlastmodtime"/>
          </pmtimex>
          <rmtimex>
            <xsl:value-of select="$vlastmodtime"/>
          </rmtimex>
        </entry>
      </xsl:for-each>
    </passwordsafe>
  </xsl:template>

  <xsl:template name="string-replace-all">
    <xsl:param name="text"/>
    <xsl:param name="replace"/>
    <xsl:param name="by"/>
    <xsl:choose>
      <xsl:when test="contains($text, $replace)">
        <xsl:value-of select="substring-before($text, $replace)"/>
        <xsl:value-of select="$by"/>
        <xsl:call-template name="string-replace-all">
          <xsl:with-param name="text" select="substring-after($text, $replace)"/>
          <xsl:with-param name="replace" select="$replace"/>
          <xsl:with-param name="by" select="$by"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$text"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>
