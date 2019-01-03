<?xml version="1.0" encoding="UTF-8"?>
<!--
  Copyright (c) 2018-2019 Rony Shapiro <ronys@pwsafe.org>.
  All rights reserved. Use of the code is allowed under the
  Artistic License 2.0 terms, as specified in the LICENSE file
  distributed with this code, or available from
  http://www.opensource.org/licenses/artistic-license-2.0.php

  This file will convert XML files exported by KeePassX V0.4.3
  into the equivalent XML files that can be imported into PasswordSafe V3.26 or later.

  The delimiter attribute in the 'passwordsafe' element is set to "right angle quotation
  marks" and any carriage returns/line feeds in the Notes field will be converted this
  character to enable the Notes field to be processed correctly by PasswordSafe during import.

  KeePass exports the Group Tree structure using nested groups. PasswordSafe uses a dot ('.').
  Therefore this XSLT code first changes any dot in the group field to a forward slash ('/').
  This will create the correct group path in Password Safe but any dots in the original
  group field will remain a forward slash.

  Attachments are not imported, but attachment's description, if any, will be put into notes.

  This XSLT file conforms to V1.0 of XSLT as described in http://www.w3.org/TR/xslt

  Under Windows, the following 3 programs can process this file:
  a. Command line program msxml.exe from Microsoft (Note: the executable states it
     is V1.1.0.1 but the website says it is V2.0).
  b. Saxon-HE (Home Edition)
     See http://saxon.sourceforge.net/.
     There is also a graphical front-end for Saxon called Kernow.
     See http://kernowforsaxon.sourceforge.net
  c. Command line program xsltproc (part of libxslt) http://xmlsoft.org/XSLT/xsltproc.html
  All programs are free.
-->

<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xs="http://www.w3.org/2001/XMLSchema"
  exclude-result-prefixes="xs">

  <xsl:output method="xml" encoding="UTF-8" indent="yes"
     cdata-section-elements="group title password username url notes autotype"/>

  <xsl:variable name="delimiter" select="'&#0187;'"/>
  <xsl:variable name="crlf" select="'&#0013;&#0010;'"/>
  <xsl:variable name="lfcr" select="'&#0010;&#0013;'"/>

  <xsl:variable name="loCase" select="'abcdefghijklmnopqrstuvwxyz'"/>
  <xsl:variable name="upCase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'"/>

  <xsl:variable name="autotypePrefix" select="'Auto-Type:'"/>

  <xsl:template match="/">
    <passwordsafe>
      <xsl:attribute name="xsi:noNamespaceSchemaLocation"
         namespace="http://www.w3.org/2001/XMLSchema-instance">pwsafe.xsd</xsl:attribute>
      <xsl:attribute name="delimiter"><xsl:value-of select="$delimiter"/></xsl:attribute>
      <!-- process root group -->
      <xsl:for-each select="database/group">
        <xsl:call-template name="groupTemplate">
          <xsl:with-param name="group" select="''"/>
        </xsl:call-template>
      </xsl:for-each>
      <xsl:value-of select="$crlf" disable-output-escaping="yes"/>
      <xsl:value-of select="$crlf" disable-output-escaping="yes"/>
    </passwordsafe>
  </xsl:template>

  <!-- process group -->
  <xsl:template name="groupTemplate">
    <xsl:param name="group"/>

    <!-- build group name -->
    <xsl:variable name="currentGroup">
      <xsl:choose>
        <xsl:when test="normalize-space($group) != ''">
          <xsl:value-of select="concat($group, '.', translate(title,'.','/'))"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="translate(title,'.','/')"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <!-- process elements -->
    <xsl:for-each select="entry">
      <xsl:call-template name="entryTemplate">
        <xsl:with-param name="group" select="$currentGroup"/>
      </xsl:call-template>
    </xsl:for-each>

    <!-- process nested groups -->
    <xsl:for-each select="group">
      <xsl:call-template name="groupTemplate">
        <xsl:with-param name="group" select="$currentGroup"/>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:template>

  <!-- process entry -->
  <xsl:template name="entryTemplate">
    <xsl:param name="group"/>

    <xsl:value-of select="$crlf" disable-output-escaping="yes"/>
    <xsl:value-of select="$crlf" disable-output-escaping="yes"/>

    <entry>
      <!-- group -->
      <group>
        <xsl:value-of select="$group"/>
      </group>

      <!-- title -->
      <title>
        <xsl:value-of select="string(title)"/>
      </title>
      <!-- username -->
      <username>
        <xsl:value-of select="string(username)"/>
      </username>
      <!-- password -->
      <password>
        <xsl:value-of select="string(password)"/>
      </password>
      <!-- url -->
      <xsl:if test="normalize-space(url) != ''">
        <xsl:call-template name="urlParser">
          <xsl:with-param name="url" select="normalize-space(url)"/>
        </xsl:call-template>
      </xsl:if>

      <xsl:variable name="comment">
        <xsl:apply-templates select="comment"/>
      </xsl:variable>

      <!-- notes and other fields -->
      <notes>
        <!-- \n translated into &#0187;, \r removed -->
        <xsl:value-of select="translate($comment, '\n', $delimiter)"/>
        <!-- Attachment descriptions -->
        <xsl:if test="normalize-space(bindesc) != ''">
          <xsl:value-of select="concat($delimiter,'Attachment description: ',bindesc)"/>
        </xsl:if>
      </notes>

      <!-- autotype -->
      <xsl:if test="contains($comment, $autotypePrefix)">
        <xsl:variable name="autotypeStart" select="substring-after($comment,$autotypePrefix)"/>
        <xsl:variable name="autotype">
          <xsl:choose>
            <xsl:when test="contains($autotypeStart,'\n')">
              <xsl:value-of select="normalize-space(substring-before($autotypeStart,'\n'))"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="normalize-space($autotypeStart)"/>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:variable>

        <xsl:if test="$autotype != ''">
          <autotype>
            <xsl:call-template name="autotypeParser">
              <xsl:with-param name="autotype" select="$autotype"/>
            </xsl:call-template>
          </autotype>
        </xsl:if>
      </xsl:if>

      <!-- times -->
      <ctimex>
        <xsl:value-of select="string(creation)"/>
      </ctimex>
      <atimex>
        <xsl:value-of select="string(lastaccess)"/>
      </atimex>
      <xsl:if test="string(expire) != 'Never'">
        <xtimex>
          <xsl:value-of select="string(expire)"/>
        </xtimex>
      </xsl:if>
      <pmtimex>
        <xsl:value-of select="string(lastmod)"/>
      </pmtimex>
      <rmtimex>
        <xsl:value-of select="string(lastmod)"/>
      </rmtimex>
    </entry>
  </xsl:template>

<!-- ====================================================================== -->

<!-- process url field -->
  <!--
    + cmd:// -> run
    + title (run only)
    + username (run only)
    + password (run only)
  -->
  <xsl:template name="urlParser">
    <xsl:param name="url"/>
    <xsl:choose>
      <xsl:when test="translate(substring($url, 1, 6), $loCase, $upCase) = 'CMD://'">
        <runcommand>
          <!-- {TITLE} -->
          <xsl:variable name="substTitle">
            <xsl:call-template name="replaceSubstring">
              <xsl:with-param name="str" select="substring($url, 7)"/>
              <xsl:with-param name="replace" select="'{TITLE}'"/>
              <xsl:with-param name="by" select="'\i'"/>
            </xsl:call-template>
          </xsl:variable>
          <!-- {USERNAME} -->
          <xsl:variable name="substUsername">
            <xsl:call-template name="replaceSubstring">
              <xsl:with-param name="str" select="$substTitle"/>
              <xsl:with-param name="replace" select="'{USERNAME}'"/>
              <xsl:with-param name="by" select="'\u'"/>
            </xsl:call-template>
          </xsl:variable>
          <!-- {PASSWORD} -->
          <xsl:call-template name="replaceSubstring">
            <xsl:with-param name="str" select="$substUsername"/>
            <xsl:with-param name="replace" select="'{PASSWORD}'"/>
            <xsl:with-param name="by" select="'\o'"/>
          </xsl:call-template>
        </runcommand>
      </xsl:when>
      <xsl:otherwise>
        <url>
            <xsl:value-of select="$url"/>
        </url>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- process autotype sequences -->
  <!--
    Autotype: src/libAutoTypeX11.cpp:AutoTypeX11::templateToKeysyms
    + Special key codes are case-insensitive
    + \ -> \\
    + title
    + username
    + url
    + password
    + space
    + backspace/bs/bksp
    - break
    - capslock
    - del/delete
    + end
    + enter
    - esc
    - help
    + home
    - insert/ins
    - numlock
    - scroll
    + pgdn
    + pgup
    - prtsc
    + up
    + down
    + left
    + right
    - f1-f16
    + add/plus
    + subtract
    + multiply
    + divide
    + at
    + percent
    + caret
    + tilde
    + leftbrace
    + rightbrace
    + leftparent
    + rightparen
    - winl/winr/win

  -->
  <xsl:template name="autotypeParser">
    <xsl:param name="autotype"/>
    <!-- \ => \\  It should be done first -->
    <xsl:variable name="repSlash">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$autotype"/>
        <xsl:with-param name="replace" select="'\'"/>
        <xsl:with-param name="by" select="'\\'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {TITLE} -->
    <xsl:variable name="substTitle">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$repSlash"/>
        <xsl:with-param name="replace" select="'{TITLE}'"/>
        <xsl:with-param name="by" select="'\i'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {USERNAME} -->
    <xsl:variable name="substUsername">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substTitle"/>
        <xsl:with-param name="replace" select="'{USERNAME}'"/>
        <xsl:with-param name="by" select="'\u'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {PASSWORD} -->
    <xsl:variable name="substPassword">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substUsername"/>
        <xsl:with-param name="replace" select="'{PASSWORD}'"/>
        <xsl:with-param name="by" select="'\p'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {SPACE} -->
    <xsl:variable name="substSpace">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substPassword"/>
        <xsl:with-param name="replace" select="'{SPACE}'"/>
        <xsl:with-param name="by" select="' '"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {BACKSPACE} -->
    <xsl:variable name="substBs">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substSpace"/>
        <xsl:with-param name="replace" select="'{BACKSPACE}'"/>
        <xsl:with-param name="by" select="'\b'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {BS} -->
    <xsl:variable name="substBs2">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substBs"/>
        <xsl:with-param name="replace" select="'{BS}'"/>
        <xsl:with-param name="by" select="'\b'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {BKSP} -->
    <xsl:variable name="substBs3">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substBs2"/>
        <xsl:with-param name="replace" select="'{BKSP}'"/>
        <xsl:with-param name="by" select="'\b'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {URL} -->
    <xsl:variable name="substUrl">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substBs3"/>
        <xsl:with-param name="replace" select="'{URL}'"/>
        <xsl:with-param name="by" select="'\l'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {END} -->
    <xsl:variable name="substEnd">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substUrl"/>
        <xsl:with-param name="replace" select="'{END}'"/>
        <xsl:with-param name="by" select="'\{End}'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {ENTER} -->
    <xsl:variable name="substEnter">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substEnd"/>
        <xsl:with-param name="replace" select="'{ENTER}'"/>
        <xsl:with-param name="by" select="'\{Enter}'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {HOME} -->
    <xsl:variable name="substHome">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substEnter"/>
        <xsl:with-param name="replace" select="'{HOME}'"/>
        <xsl:with-param name="by" select="'\{Home}'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {PGDN} -->
    <xsl:variable name="substPgDn">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substHome"/>
        <xsl:with-param name="replace" select="'{PGDN}'"/>
        <xsl:with-param name="by" select="'\{PgDn}'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {PGUP} -->
    <xsl:variable name="substPgUp">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substPgDn"/>
        <xsl:with-param name="replace" select="'{PGUP}'"/>
        <xsl:with-param name="by" select="'\{PgUp}'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {UP} -->
    <xsl:variable name="substUp">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substPgUp"/>
        <xsl:with-param name="replace" select="'{UP}'"/>
        <xsl:with-param name="by" select="'\{Up}'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {DOWN} -->
    <xsl:variable name="substDown">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substUp"/>
        <xsl:with-param name="replace" select="'{DOWN}'"/>
        <xsl:with-param name="by" select="'\{Down}'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {LEFT} -->
    <xsl:variable name="substLeft">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substDown"/>
        <xsl:with-param name="replace" select="'{LEFT}'"/>
        <xsl:with-param name="by" select="'\{Left}'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {RIGHT} -->
    <xsl:variable name="substRight">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substLeft"/>
        <xsl:with-param name="replace" select="'{RIGHT}'"/>
        <xsl:with-param name="by" select="'\{Right}'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {ADD} -->
    <xsl:variable name="substAdd">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substRight"/>
        <xsl:with-param name="replace" select="'{ADD}'"/>
        <xsl:with-param name="by" select="'+'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {PLUS} -->
    <xsl:variable name="substAdd2">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substAdd"/>
        <xsl:with-param name="replace" select="'{PLUS}'"/>
        <xsl:with-param name="by" select="'+'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {SUBTRACT} -->
    <xsl:variable name="substSub">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substAdd2"/>
        <xsl:with-param name="replace" select="'{SUBTRACT}'"/>
        <xsl:with-param name="by" select="'-'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {MULTIPLY} -->
    <xsl:variable name="substMul">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substSub"/>
        <xsl:with-param name="replace" select="'{MULTIPLY}'"/>
        <xsl:with-param name="by" select="'*'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {DIVIDE} -->
    <xsl:variable name="substDiv">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substMul"/>
        <xsl:with-param name="replace" select="'{DIVIDE}'"/>
        <xsl:with-param name="by" select="'/'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {AT} -->
    <xsl:variable name="substAt">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substDiv"/>
        <xsl:with-param name="replace" select="'{AT}'"/>
        <xsl:with-param name="by" select="'@'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {PERCENT} -->
    <xsl:variable name="substPercent">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substAt"/>
        <xsl:with-param name="replace" select="'{PERCENT}'"/>
        <xsl:with-param name="by" select="'%'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {CARET} -->
    <xsl:variable name="substCaret">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substPercent"/>
        <xsl:with-param name="replace" select="'{CARET}'"/>
        <xsl:with-param name="by" select="'^'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {TILDE} -->
    <xsl:variable name="substTilde">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substCaret"/>
        <xsl:with-param name="replace" select="'{TILDE}'"/>
        <xsl:with-param name="by" select="'~'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {LEFTBRACE} -->
    <xsl:variable name="substLBrace">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substTilde"/>
        <xsl:with-param name="replace" select="'{LEFTBRACE}'"/>
        <xsl:with-param name="by" select="'{'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {RIGHTBRACE} -->
    <xsl:variable name="substRBrace">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substLBrace"/>
        <xsl:with-param name="replace" select="'{RIGHTBRACE}'"/>
        <xsl:with-param name="by" select="'}'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {LEFTPAREN} -->
    <xsl:variable name="substLParen">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substRBrace"/>
        <xsl:with-param name="replace" select="'{LEFTPAREN}'"/>
        <xsl:with-param name="by" select="'('"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {RIGHTPAREN} -->
    <xsl:variable name="substRParen">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substLParen"/>
        <xsl:with-param name="replace" select="'{RIGHTPAREN}'"/>
        <xsl:with-param name="by" select="')'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {TAB} -->
    <xsl:variable name="substTab">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substRParen"/>
        <xsl:with-param name="replace" select="'{TAB}'"/>
        <xsl:with-param name="by" select="'\t'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {DELAY ...} -->
    <xsl:call-template name="substDelaySequence">
      <xsl:with-param name="str" select="$substTab"/>
    </xsl:call-template>
  </xsl:template>

  <!-- substitutions for {DELAY X} tag sequences -->
  <xsl:template name="substDelaySequence">
    <xsl:param name="str"/>

    <xsl:variable name="tag" select="'DELAY'"/>
    <xsl:variable name="openTag" select="'{'"/>
    <xsl:variable name="closeTag" select="'}'"/>

    <!-- convert tag for case-insensitive search -->
    <xsl:variable name="sTag" select="concat($openTag, translate($tag,$loCase,$upCase))"/>

    <!-- convert string for case-insensitive search -->
    <xsl:variable name="sStr" select="translate($str,$loCase,$upCase)"/>

    <xsl:choose>
      <xsl:when test="contains($sStr, $sTag)">
        <xsl:value-of select="substring($str, 1, string-length(substring-before($sStr, $sTag)))"/>
        <xsl:variable name="tail" select="substring($str, string-length(substring-before($sStr, $sTag))+string-length($sTag)+1)"/>
        <!-- get count-->
        <xsl:variable name="cnt" select="normalize-space(substring-before($tail, $closeTag))"/>
        <xsl:choose>
          <xsl:when test="$cnt > 999">
            <xsl:value-of select="concat('\W',floor($cnt div 1000))"/>
            <xsl:if test="$cnt mod 1000 > 0">
              <xsl:value-of select="concat('\w',$cnt mod 1000)"/>
            </xsl:if>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="concat('\w',$cnt)"/>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:call-template name="substDelaySequence">
          <xsl:with-param name="str" select="substring-after($tail, $closeTag)"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$str"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- replace all substrings -->
  <xsl:template name="replaceSubstring">
    <xsl:param name="str"/>
    <xsl:param name="replace"/>
    <xsl:param name="by"/>

    <!-- convert search string for case-insensitive search -->
    <xsl:variable name="sReplace" select="translate($replace,$loCase,$upCase)"/>

    <!-- convert string for case-insensitive search -->
    <xsl:variable name="sStr" select="translate($str,$loCase,$upCase)"/>

    <xsl:choose>
      <xsl:when test="contains($sStr, $sReplace)">
        <xsl:value-of select="substring($str, 1, string-length(substring-before($sStr, $sReplace)))"/>
        <xsl:value-of select="$by"/>
        <xsl:call-template name="replaceSubstring">
          <xsl:with-param name="str" select="substring($str, string-length(substring-before($sStr, $sReplace))+string-length($sReplace)+1)"/>
          <xsl:with-param name="replace" select="$replace"/>
          <xsl:with-param name="by" select="$by"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$str"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

<xsl:template match="br">
  <xsl:value-of select="'\n'"/>
</xsl:template>
</xsl:stylesheet>
