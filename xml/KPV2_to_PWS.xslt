<?xml version="1.0" encoding="UTF-8"?>
<!--
  Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
  All rights reserved. Use of the code is allowed under the
  Artistic License 2.0 terms, as specified in the LICENSE file
  distributed with this code, or available from
  http://www.opensource.org/licenses/artistic-license-2.0.php

  This file will convert XML files exported by KeePass V2 (tested with KeePass v2.15)
  into the equivalent XML files that can be imported into PasswordSafe V3.26 or later.

  The delimiter attribute in the 'passwordsafe' element is set to "right angle quotation
  marks" and any carriage returns/line feeds in the Notes field will be converted this
  character to enable the Notes field to be processed correctly by PasswordSafe during import.

  KeePass exports the Group Tree structure using nested groups. PasswordSafe uses a dot ('.').
  Therefore this XSLT code first changes any dot in the group field to a forward slash ('/').
  This will create the correct group path in Password Safe but any dots in the original
  group field will remain a forward slash.

  This XSLT file conforms to V1.0 of XSLT as described in http://www.w3.org/TR/xslt

  Under Windows, the following 3 programs can process this file:
  a. Command line program msxml.exe from Microsoft (Note: the executable states it
     is V1.1.0.1 but the website says it is V2.0).
  b. AltovaXML Community Edition (current version 2011r3). 
     See http://www.altova.com/altovaxml.html.
  c. Saxon-HE (Home Edition) (current version 9.3.0.5).
     See http://saxon.sourceforge.net/.
     There is also a graphical front-end for Saxon called Kernow.
     See http://kernowforsaxon.sourceforge.net

  All programs are free.
-->

<xsl:stylesheet version="1.0" 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xs="http://www.w3.org/2001/XMLSchema"
  exclude-result-prefixes="xs">

  <xsl:output method="xml" encoding="UTF-8" indent="yes"
     cdata-section-elements="group title password username url notes uuid runcommand autotype oldpassword"/>

  <xsl:variable name="delimiter" select="'&#0187;'"/>
  <xsl:variable name="crlf" select="'&#0013;&#0010;'"/>
  <xsl:variable name="lfcr" select="'&#0010;&#0013;'"/>

  <xsl:variable name="loCase" select="'abcdefghijklmnopqrstuvwxyz'"/>
  <xsl:variable name="upCase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'"/>

  <xsl:template match="/">
    <passwordsafe>
      <xsl:attribute name="xsi:noNamespaceSchemaLocation"
         namespace="http://www.w3.org/2001/XMLSchema-instance">pwsafe.xsd</xsl:attribute>
      <xsl:attribute name="delimiter"><xsl:value-of select="$delimiter"/></xsl:attribute>
      <!-- process root group -->
      <xsl:for-each select="KeePassFile/Root/Group">
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
    <xsl:param name="groupAutotype"/>

    <!-- build group name -->
    <xsl:variable name="currentGroup">
      <xsl:choose>
        <xsl:when test="normalize-space($group) != ''">
          <xsl:value-of  select="concat($group, '.', translate(Name,'.','/'))"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="translate(Name,'.','/')"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <!-- set autotype -->
    <xsl:variable name="currentAutotype">
      <xsl:choose>
        <xsl:when test="normalize-space(DefaultAutoTypeSequence) != ''">
          <xsl:value-of select="DefaultAutoTypeSequence"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$groupAutotype"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <!-- process elements -->
    <xsl:for-each select="Entry">
      <xsl:call-template name="entryTemplate">
        <xsl:with-param name="group" select="$currentGroup"/>
        <xsl:with-param name="groupAutotype" select="$currentAutotype"/>
      </xsl:call-template>
    </xsl:for-each>

    <!-- process nested groups -->
    <xsl:for-each select="Group">
      <xsl:call-template name="groupTemplate">
        <xsl:with-param name="group" select="$currentGroup"/>
        <xsl:with-param name="groupAutotype" select="$currentAutotype"/>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:template>

  <!-- process entry -->
  <xsl:template name="entryTemplate">
    <xsl:param name="group"/>
    <xsl:param name="groupAutotype"/>

    <xsl:value-of select="$crlf" disable-output-escaping="yes"/>
    <xsl:value-of select="$crlf" disable-output-escaping="yes"/>

    <entry>
      <!-- group -->
      <group>
        <xsl:value-of select="$group"/>
      </group>

      <!-- process strings (unknown string are placed into Notes) -->
      <!-- title -->
      <title>
        <xsl:value-of select="String[Key='Title']/Value"/>
      </title>
      <!-- username -->
      <username>
        <xsl:value-of select="String[Key='UserName']/Value"/>
      </username>
      <!-- password -->
      <password>
        <xsl:value-of select="String[Key='Password']/Value"/>
      </password>
      <!-- url/run -->
      <xsl:if test="normalize-space(String[Key='URL']/Value) != ''">
        <xsl:call-template name="urlParser">
          <xsl:with-param name="url" select="String[Key='URL']/Value"/>
          <xsl:with-param name="useAlt">
            <xsl:choose>
              <xsl:when test="normalize-space(OverrideURL) = ''">
                0
              </xsl:when>
              <xsl:otherwise>
                1
              </xsl:otherwise>
            </xsl:choose>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:if>

      <!-- notes and other fields -->
      <notes>
        <!-- \n translated into &#0187;, \r removed -->
        <xsl:value-of select="translate(String[Key='Notes']/Value, $lfcr, $delimiter)"/>
        <!-- Add unknown strings -->
        <xsl:for-each select="String">
          <xsl:if test="(string(Key) != 'Title') and (string(Key) != 'UserName') and 
           (string(Key) != 'Password') and (string(Key) != 'URL') and (string(Key) != 'Notes')">
            <xsl:value-of select="$delimiter"/><xsl:value-of select="concat(Key,': ',translate(Value, $lfcr, $delimiter))"/>
          </xsl:if>
        </xsl:for-each>
        <!-- override URL value if not empty -->
        <xsl:if test="normalize-space(OverrideURL) != ''">
          <xsl:value-of select="$delimiter"/><xsl:value-of select="concat('OverrideURL: ',OverrideURL)"/>
        </xsl:if>
        <!-- Tags -->
        <xsl:if test="normalize-space(Tags) != ''">
          <xsl:value-of select="$delimiter"/><xsl:value-of select="concat('Tags: ',Tags)"/>
        </xsl:if>
      </notes>

      <!-- UUID -->
      <uuid>
        <xsl:call-template name="base64ToHex">
          <xsl:with-param name="val" select="UUID"/>
        </xsl:call-template>
      </uuid>

      <!-- autotype -->
      <xsl:if test="string(AutoType/Enabled) = 'True'">
        <xsl:variable name="autotype">
          <xsl:choose>
            <xsl:when test="normalize-space(AutoType/DefaultSequence) != ''">
              <xsl:value-of select="AutoType/DefaultSequence"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="$groupAutotype"/>
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
        <xsl:value-of select="Times/CreationTime"/>
      </ctimex>
      <atimex>
        <xsl:value-of select="Times/LastAccessTime"/>
      </atimex>
      <xsl:if test="string(Times/Expires) = 'True'">
        <xtimex>
          <xsl:value-of select="Times/ExpiryTime"/>
        </xtimex>
      </xsl:if>
      <pmtimex>
        <xsl:value-of select="Times/LastModificationTime"/>
      </pmtimex>
      <rmtimex>
        <xsl:value-of select="Times/LastModificationTime"/>
      </rmtimex>

      <!-- history -->
      <pwhistory>
        <status>1</status>
        <max>
          <xsl:value-of select="/KeePassFile/Meta/HistoryMaxItems"/>
        </max>
        <num>
          <xsl:value-of select="count(History/Entry)"/>
        </num>
        <xsl:if test="count(History/Entry) > 0">
          <history_entries>
            <xsl:for-each select="History/Entry">
              <xsl:call-template name="historyTemplate"/>
            </xsl:for-each>
          </history_entries>
        </xsl:if>
      </pwhistory>
    </entry>
  </xsl:template>

<!-- ====================================================================== -->
  <!-- process history entries -->
  <xsl:template name="historyTemplate">
    <history_entry>
      <xsl:attribute name="num">
        <xsl:value-of select="position()"/>
      </xsl:attribute>
      <changedx>
        <xsl:value-of select="Times/LastModificationTime"/>
      </changedx>
      <oldpassword>
        <xsl:value-of select="String[Key='Password']/Value"/>
      </oldpassword>
    </history_entry>
  </xsl:template>


  <!-- process autotype sequences -->
  <!-- 
    Autotype: http://keepass.info/help/base/autotype.html
    + Special key codes are case-insensitive
    + Tab  {TAB} => \t
    + +{TAB} => \s (Shift-Tab)
    + Enter  {ENTER} or ~ => \n
    * Arrow Up  {UP} 
    * Arrow Down  {DOWN}
    * Arrow Left  {LEFT}
    * Arrow Right  {RIGHT}
    * Insert  {INSERT} or {INS}
    * Delete  {DELETE} or {DEL}
    * Home  {HOME}
    * End  {END}
    * Page Up  {PGUP}
    * Page Down  {PGDN}
    + Backspace  {BACKSPACE}, {BS} or {BKSP} => /b
    * Break  {BREAK}
    * Caps-Lock  {CAPSLOCK}
    * Escape  {ESC}
    * Help  {HELP}
    * Numlock  {NUMLOCK}
    * Print Screen  {PRTSC}
    * Scroll Lock  {SCROLLLOCK}
    * F1 - F16  {F1} - {F16}
    + Keypad +  {ADD} => +
    + Keypad -  {SUBTRACT} => -
    + Keypad *  {MULTIPLY} => *
    + Keypad /  {DIVIDE} => /
    * Shift  +
    * Ctrl  ^
    * Alt  %
    + +  {+} => +
    + ^  {^} => ^
    + %  {%} => %
    + ~  {~} => ~
    + (, )  {(}, {)} => (, )
    * {, }  {{}, {}} => {, }
    + {DELAY X}  Delays X milliseconds. => \wX (\WX)
    * {VKEY X}  Sends the virtual key of value X.
    + {DELAY=X}  Sets the default delay to X milliseconds for all standard keypresses in this sequence. => \dX
    + \ => \\
    + Keys and special keys (not placeholders or commands) can be repeated by appending a number within the code. For example, {TAB 5} presses the Tab key 5 times. 
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
    <!-- ~ => \n -->
    <xsl:variable name="repTilde">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$repSlash"/>
        <xsl:with-param name="replace" select="'~'"/>
        <xsl:with-param name="by" select="'\n'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- Fix {~} replaced with {\n}  -->
    <xsl:variable name="repTilde2">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$repTilde"/>
        <xsl:with-param name="tag" select="'\n'"/>
        <xsl:with-param name="by" select="'~'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {ENTER} -->
    <xsl:variable name="substEnter">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$repTilde2"/>
        <xsl:with-param name="tag" select="'ENTER'"/>
        <xsl:with-param name="by" select="'\n'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {TAB} -->
    <xsl:variable name="substTab">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substEnter"/>
        <xsl:with-param name="tag" select="'TAB'"/>
        <xsl:with-param name="by" select="'\t'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- +\t -->
    <xsl:variable name="repSTab">
      <xsl:call-template name="replaceSubstring">
        <xsl:with-param name="str" select="$substTab"/>
        <xsl:with-param name="replace" select="'+\t'"/>
        <xsl:with-param name="by" select="'\s'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {BACKSPACE} -->
    <xsl:variable name="substBs">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$repSTab"/>
        <xsl:with-param name="tag" select="'BACKSPACE'"/>
        <xsl:with-param name="by" select="'\b'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {BS} -->
    <xsl:variable name="substBs2">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substBs"/>
        <xsl:with-param name="tag" select="'BS'"/>
        <xsl:with-param name="by" select="'\b'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {BKSP} -->
    <xsl:variable name="substBs3">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substBs2"/>
        <xsl:with-param name="tag" select="'BKSP'"/>
        <xsl:with-param name="by" select="'\b'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {ADD} -->
    <xsl:variable name="substAdd">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substBs3"/>
        <xsl:with-param name="tag" select="'ADD'"/>
        <xsl:with-param name="by" select="'+'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {+} -->
    <xsl:variable name="substAdd2">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substAdd"/>
        <xsl:with-param name="tag" select="'+'"/>
        <xsl:with-param name="by" select="'+'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {SUBTRACT} -->
    <xsl:variable name="substSubtract">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substAdd2"/>
        <xsl:with-param name="tag" select="'SUBTRACT'"/>
        <xsl:with-param name="by" select="'-'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {MULTIPLY} -->
    <xsl:variable name="substMultiply">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substSubtract"/>
        <xsl:with-param name="tag" select="'MULTIPLY'"/>
        <xsl:with-param name="by" select="'*'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {DIVIDE} -->
    <xsl:variable name="substDivide">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substMultiply"/>
        <xsl:with-param name="tag" select="'DIVIDE'"/>
        <xsl:with-param name="by" select="'/'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {^} -->
    <xsl:variable name="substHat">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substDivide"/>
        <xsl:with-param name="tag" select="'^'"/>
        <xsl:with-param name="by" select="'^'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {%} -->
    <xsl:variable name="substPct">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substHat"/>
        <xsl:with-param name="tag" select="'%'"/>
        <xsl:with-param name="by" select="'%'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {(} -->
    <xsl:variable name="substRBrace">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substPct"/>
        <xsl:with-param name="tag" select="'('"/>
        <xsl:with-param name="by" select="'('"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {)} -->
    <xsl:variable name="substLBrace">
      <xsl:call-template name="substSequence">
        <xsl:with-param name="str" select="$substRBrace"/>
        <xsl:with-param name="tag" select="')'"/>
        <xsl:with-param name="by" select="')'"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- {DELAY ...} -->
    <xsl:variable name="substDelay">
      <xsl:call-template name="substDelaySequence">
        <xsl:with-param name="str" select="$substLBrace"/>
      </xsl:call-template>
    </xsl:variable>
    <!-- substitute placeholders -->
    <xsl:call-template name="substPlaceholder">
      <xsl:with-param name="str" select="$substDelay"/>
      <xsl:with-param name="inAutotype" select="1"/>
    </xsl:call-template>
  </xsl:template>


  <!-- Placeholder substitution -->
  <!--
    Placeholders http://keepass.info/help/base/placeholders.html

    + Placeholders are case-insensitive.
    + Title  {TITLE}
    + User Name  {USERNAME}
    +(non-autotype) URL  {URL}
    + Password  {PASSWORD}
    + Notes  {NOTES}
    +(non-autotype as url) {URL:RMVSCM}  Entry URL without scheme specifier.
    * {INTERNETEXPLORER}  Path of Internet Explorer, if installed.
    * {FIREFOX}  Path of Mozilla Firefox, if installed.
    * {OPERA}  Path of Opera, if installed.
    * {GOOGLECHROME}  Path of Google Chrome, if installed.
    +(non-autotype) {APPDIR}  KeePass application directory path.
    + {GROUP}  Name of the entry's parent group.
    +(non-autotype) {GROUPPATH}  Full group path of the entry.
    +(non-autotype) {DB_PATH}  Full path of the current database.
    +(non-autotype) {DB_DIR}  Directory of the current database.
    +(non-autotype) {DB_NAME}  File name (including extension) of the current database.
    +(non-autotype) {DB_BASENAME}  File name (excluding extension) of the current database.
    +(non-autotype) {DB_EXT}  File name extension of the current database.
    * {ENV_DIRSEP}  Directory separator ('\' on Windows, '/' on Unix).
    * {DT_SIMPLE}  Current local date/time as a simple, sortable string.
    * {DT_YEAR}  Year component of the current local date/time.
    * {DT_MONTH}  Month component of the current local date/time.
    * {DT_DAY}  Day component of the current local date/time.
    * {DT_HOUR}  Hour component of the current local date/time.
    * {DT_MINUTE}  Minute component of the current local date/time.
    * {DT_SECOND}  Seconds component of the current local date/time.
    * {DT_UTC_SIMPLE}  Current UTC date/time as a simple, sortable string.
    * {DT_UTC_YEAR}  Year component of the current UTC date/time.
    * {DT_UTC_MONTH}  Month component of the current UTC date/time.
    * {DT_UTC_DAY}  Day component of the current UTC date/time.
    * {DT_UTC_HOUR}  Hour component of the current UTC date/time.
    * {DT_UTC_MINUTE}  Minute component of the current UTC date/time.
    * {DT_UTC_SECOND}  Seconds component of the current UTC date/time.
    * {PICKCHARS}
    * {PICKCHARS:Fld:Opt}  Shows a dialog to pick certain characters from an entry string. See below.
    * {NEWPASSWORD}  Generates a new password. See below.
    * {PASSWORD_ENC}  Password in encrypted form. See below.
    * {HMACOTP}  Generates a one-time password. See below.
    +(non-autotype) $ => \$
    +(non-autotype) \{ => \\{
    + Custom strings can be referenced using {S:Name}. For example, if you have a custom string named "eMail", you can use the placeholder {S:eMail}. 
  -->

  <xsl:template name="substPlaceholder">
    <xsl:param name="str"/>
    <xsl:param name="inAutotype"/><!-- if =1, than use sequences for autotype, else for cmd -->
    <xsl:choose>
      <xsl:when test="$inAutotype = 1" >
        <!-- {TITLE} -->
        <xsl:variable name="substTitle">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$str"/>
            <xsl:with-param name="tag" select="'TITLE'"/>
            <xsl:with-param name="by" select="'\i'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {USERNAME} -->
        <xsl:variable name="substUsername">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substTitle"/>
            <xsl:with-param name="tag" select="'USERNAME'"/>
            <xsl:with-param name="by" select="'\u'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {PASSWORD} -->
        <xsl:variable name="substPassword">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substUsername"/>
            <xsl:with-param name="tag" select="'PASSWORD'"/>
            <xsl:with-param name="by" select="'\p'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {NOTES} -->
        <xsl:variable name="substNotes">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substPassword"/>
            <xsl:with-param name="tag" select="'NOTES'"/>
            <xsl:with-param name="by" select="'\o'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {GROUP} -->
        <xsl:variable name="substGroup">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substNotes"/>
            <xsl:with-param name="tag" select="'GROUP'"/>
            <xsl:with-param name="by" select="'\g'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {S:FieldName} -->
        <xsl:variable name="substF">
          <xsl:call-template name="substField">
            <xsl:with-param name="str" select="$substGroup"/>
          </xsl:call-template>
        </xsl:variable>
        
         <xsl:value-of select="$substF"/>
      </xsl:when>
      <xsl:otherwise>
        <!-- \{ => \\{ For correct slash escaping after vars substitution. It should be done first -->
        <xsl:variable name="repSlash">
          <xsl:call-template name="replaceSubstring">
            <xsl:with-param name="str" select="$str"/>
            <xsl:with-param name="replace" select="'\{'"/>
            <xsl:with-param name="by" select="'\\{'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- $ => \$ Escape $. -->
        <xsl:variable name="repS">
          <xsl:call-template name="replaceSubstring">
            <xsl:with-param name="str" select="$repSlash"/>
            <xsl:with-param name="replace" select="'$'"/>
            <xsl:with-param name="by" select="'\$'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {TITLE} -->
        <xsl:variable name="substTitle">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$repS"/>
            <xsl:with-param name="tag" select="'TITLE'"/>
            <xsl:with-param name="by" select="'${t}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {USERNAME} -->
        <xsl:variable name="substUsername">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substTitle"/>
            <xsl:with-param name="tag" select="'USERNAME'"/>
            <xsl:with-param name="by" select="'${u}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {PASSWORD} -->
        <xsl:variable name="substPassword">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substUsername"/>
            <xsl:with-param name="tag" select="'PASSWORD'"/>
            <xsl:with-param name="by" select="'${p}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {NOTES} -->
        <xsl:variable name="substNotes">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substPassword"/>
            <xsl:with-param name="tag" select="'NOTES'"/>
            <xsl:with-param name="by" select="'${n}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {URL} -->
        <xsl:variable name="substUrl">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substNotes"/>
            <xsl:with-param name="tag" select="'URL'"/>
            <xsl:with-param name="by" select="'${url}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {URL:RMVSCM} -->
        <xsl:variable name="substUrl2">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substUrl"/>
            <xsl:with-param name="tag" select="'URL:RMVSCM'"/>
            <xsl:with-param name="by" select="'${url}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {APPDIR} -->
        <xsl:variable name="substAppdir">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substUrl2"/>
            <xsl:with-param name="tag" select="'APPDIR'"/>
            <xsl:with-param name="by" select="'${appdir}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {GROUP} -->
        <xsl:variable name="substGroup">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substAppdir"/>
            <xsl:with-param name="tag" select="'GROUP'"/>
            <xsl:with-param name="by" select="'${G}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {GROUPPATH} -->
        <xsl:variable name="substFullGroup">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substGroup"/>
            <xsl:with-param name="tag" select="'GROUPPATH'"/>
            <xsl:with-param name="by" select="'${g}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {DB_PATH} -->
        <xsl:variable name="substDBPath">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substFullGroup"/>
            <xsl:with-param name="tag" select="'DB_PATH'"/>
            <xsl:with-param name="by" select="'${fulldb}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {DB_DIR} -->
        <xsl:variable name="substDBDir">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substDBPath"/>
            <xsl:with-param name="tag" select="'DB_DIR'"/>
            <xsl:with-param name="by" select="'${dbdir}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {DB_NAME} -->
        <xsl:variable name="substDBName">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substDBDir"/>
            <xsl:with-param name="tag" select="'DB_NAME'"/>
            <xsl:with-param name="by" select="concat('${dbname}','.','${dbextn}')"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {DB_BASENAME} -->
        <xsl:variable name="substDBBaseName">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substDBName"/>
            <xsl:with-param name="tag" select="'DB_BASENAME'"/>
            <xsl:with-param name="by" select="'${dbname}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {DB_EXT} -->
        <xsl:variable name="substDBExt">
          <xsl:call-template name="substSequence">
            <xsl:with-param name="str" select="$substDBBaseName"/>
            <xsl:with-param name="tag" select="'DB_EXT'"/>
            <xsl:with-param name="by" select="'${dbextn}'"/>
          </xsl:call-template>
        </xsl:variable>
        <!-- {S:FieldName} -->
        <xsl:variable name="substF">
          <xsl:call-template name="substField">
            <xsl:with-param name="str" select="$substDBExt"/>
          </xsl:call-template>
        </xsl:variable>

         <xsl:value-of select="$substF"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- process url field -->
  <!-- 
    URL Field Capabilities: http://keepass.info/help/base/autourl.html
    + To tell KeePass that the line you entered is a command line, prefix it using cmd://.
    + Windows-style UNC paths (starting with \\) are directly supported, i.e. do not need to be prefixed with cmd://.
    +(cmd-only) In the URL field, you can use several placeholders that will get automatically replaced when the URL is executed.
    + {S:FieldName} subst. in URL
  -->
  <xsl:template name="urlParser">
    <xsl:param name="url"/>
    <xsl:param name="useAlt" select="0"/>
    <xsl:choose>
      <xsl:when test="substring($url, 1, 2) = '\\'">
        <runcommand>
          <!-- substitute placeholders -->
          <xsl:call-template name="substPlaceholder">
            <xsl:with-param name="str" select="$url"/>
            <xsl:with-param name="inAutotype" select="0"/>
          </xsl:call-template>
        </runcommand>
      </xsl:when>
      <xsl:when test="translate(substring($url, 1, 6), $loCase, $upCase) = 'CMD://'">
        <runcommand>
          <!-- substitute placeholders -->
          <xsl:call-template name="substPlaceholder">
            <xsl:with-param name="str" select="substring($url, 7)"/>
            <xsl:with-param name="inAutotype" select="0"/>
          </xsl:call-template>
        </runcommand>
      </xsl:when>
      <xsl:otherwise>
        <url>
          <!-- {S:FieldName} -->
          <xsl:variable name="substF">
            <xsl:call-template name="substField">
              <xsl:with-param name="str" select="$url"/>
            </xsl:call-template>
          </xsl:variable>
          <xsl:choose>
            <xsl:when test="$useAlt = 1">
              <xsl:value-of select="concat('[alt]',$substF)"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="$substF"/>
            </xsl:otherwise>
          </xsl:choose>
        </url>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <!-- convert base64Binary to hexBinary -->
  <xsl:template name="base64ToHex">
    <xsl:param name="val"/>
    <xsl:choose>
      <xsl:when test="string-length($val) mod 4 != 0">
        Invalid Base64 string
      </xsl:when>
      <xsl:when test="string-length($val) > 0">
        <xsl:call-template name="parseQuad">
          <xsl:with-param name="quad" select="substring($val, 1, 4)"/>
        </xsl:call-template>
        <xsl:call-template name="base64ToHex">
          <xsl:with-param name="val" select="substring($val, 5)"/>
        </xsl:call-template>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <!-- parse four letters in base64  -->
  <xsl:template name="parseQuad">
    <xsl:param name="quad"/>
    <xsl:variable name="digs" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/='"/>

    <xsl:variable name="v1" select="string-length(substring-before($digs, substring($quad, 1, 1)))"/>
    <xsl:variable name="v2" select="string-length(substring-before($digs, substring($quad, 2, 1)))"/>
    <xsl:variable name="v3" select="string-length(substring-before($digs, substring($quad, 3, 1)))"/>
    <xsl:variable name="v4" select="string-length(substring-before($digs, substring($quad, 4, 1)))"/>

    <xsl:variable name="res" select="$v1*262144 + $v2*4096 + $v3*64 + $v4"/> <!-- v1 <<18 + v2 << 12 + v3 << 6 + v4 -->

    <xsl:variable name="r1" select="floor($res div 65536)"/>
    <xsl:variable name="b1" select="$r1 mod 255"/>

    <xsl:variable name="r2" select="floor(($res - $r1*65536) div 256)"/>
    <xsl:variable name="b2" select="$r2 mod 255"/>

    <xsl:variable name="r3" select="$res - $r1*65536 - $r2*256"/>
    <xsl:variable name="b3" select="$r3"/>

    <xsl:call-template name="decToHex">
      <xsl:with-param name="val" select="$b1"/>
    </xsl:call-template>
    <xsl:if test="$v3 != 64">
      <xsl:call-template name="decToHex">
        <xsl:with-param name="val" select="$b2"/>
      </xsl:call-template>
    </xsl:if>
    <xsl:if test="$v4 != 64">
      <xsl:call-template name="decToHex">
        <xsl:with-param name="val" select="$b3"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

   <!-- convert decimal value (less than 256) to hexadecimal -->
  <xsl:template name="decToHex">
    <xsl:param name="val"/>

    <xsl:variable name="digs" select="'0123456789ABCDEF'"/>

    <xsl:value-of select="substring($digs, floor($val div 16)+1, 1)"/>
    <xsl:value-of select="substring($digs, floor($val mod 16)+1, 1)"/>
  </xsl:template>


  <!-- substitutions for tag sequences {TAG count} -->
  <xsl:template name="substSequence">
    <xsl:param name="str"/>
    <xsl:param name="tag"/>
    <xsl:param name="by"/>
    <xsl:param name="caseSensitive" select="0"/>

    <xsl:variable name="openTag" select="'{'"/>
    <xsl:variable name="closeTag" select="'}'"/>

    <!-- convert tag for search obeying caseSensitive parameter -->
    <xsl:variable name="sTag">
      <xsl:choose>
        <xsl:when test="$caseSensitive = 1">
          <xsl:value-of select="concat($openTag, $tag)"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="concat($openTag, translate($tag,$loCase,$upCase))"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <!-- convert string for search obeying caseSensitive parameter -->
    <xsl:variable name="sStr">
      <xsl:choose>
        <xsl:when test="$caseSensitive = 1">
          <xsl:value-of select="$str"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="translate($str,$loCase,$upCase)"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    
    <xsl:choose>
      <xsl:when test="contains($sStr, $sTag)">
        <xsl:value-of select="substring($str, 1, string-length(substring-before($sStr, $sTag)))"/>
        <xsl:variable name="tail" select="substring($str, string-length(substring-before($sStr, $sTag))+string-length($sTag)+1)"/>
        <!-- get count (empty equal to one) -->
        <xsl:variable name="cnt" select="normalize-space(substring-before($tail, $closeTag))"/>
        <xsl:choose>
          <xsl:when test="$cnt = ''">
            <xsl:value-of select="$by"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="repeatString">
              <xsl:with-param name="str" select="$by"/>
              <xsl:with-param name="n" select="$cnt"/>
            </xsl:call-template>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:call-template name="substSequence">
          <xsl:with-param name="str" select="substring-after($tail, $closeTag)"/>
          <xsl:with-param name="tag" select="$tag"/>
          <xsl:with-param name="by" select="$by"/>
          <xsl:with-param name="caseSensitive" select="$caseSensitive"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$str"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- repeat given string n times -->
  <xsl:template name="repeatString">
    <xsl:param name="str"/>
    <xsl:param name="n"/>

    <xsl:if test="$n > 0">
      <xsl:value-of select="$str"/>
      <xsl:call-template name="repeatString">
        <xsl:with-param name="str" select="$str"/>
        <xsl:with-param name="n" select="$n - 1"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>
   
  <!-- substitutions for {DELAY X} and {DELAY=X} tag sequences -->
  <xsl:template name="substDelaySequence">
    <xsl:param name="str"/>
    <xsl:param name="caseSensitive" select="0"/>

    <xsl:variable name="tag" select="'DELAY'"/>
    <xsl:variable name="openTag" select="'{'"/>
    <xsl:variable name="closeTag" select="'}'"/>

    <!-- convert tag for search obeying caseSensitive parameter -->
    <xsl:variable name="sTag">
      <xsl:choose>
        <xsl:when test="$caseSensitive = 1">
          <xsl:value-of select="concat($openTag, $tag)"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="concat($openTag, translate($tag,$loCase,$upCase))"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <!-- convert string for search obeying caseSensitive parameter -->
    <xsl:variable name="sStr">
      <xsl:choose>
        <xsl:when test="$caseSensitive = 1">
          <xsl:value-of select="$str"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="translate($str,$loCase,$upCase)"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="contains($sStr, $sTag)">
        <xsl:value-of select="substring($str, 1, string-length(substring-before($sStr, $sTag)))"/>
        <xsl:variable name="tail" select="substring($str, string-length(substring-before($sStr, $sTag))+string-length($sTag)+1)"/>
        <!-- get count-->
        <xsl:variable name="cnt" select="normalize-space(substring-before($tail, $closeTag))"/>
        <xsl:choose>
          <xsl:when test="substring($cnt,1,1) = '='">
            <xsl:value-of select="concat('\d',substring($cnt,2))"/>
          </xsl:when>
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
          <xsl:with-param name="caseSensitive" select="$caseSensitive"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$str"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- substitute local string fields {S:FieldName} -->
  <xsl:template name="substField">
    <xsl:param name="str"/>
    
    <xsl:variable name="tag" select="'S:'" />
    <xsl:variable name="openTag" select="'{'" />
    <xsl:variable name="closeTag" select="'}'" />

    <xsl:variable name="sTag" select="concat($openTag,$tag)" />

    <xsl:choose>
      <xsl:when test="contains($str, $sTag)">
        <xsl:value-of select="substring-before($str, $sTag)"/>
        <xsl:variable name="tail" select="substring-after($str, $sTag)"/>
        <!-- get field name -->
        <xsl:variable name="fName" select="substring-before($tail, $closeTag)"/>
        <xsl:value-of select="String[Key=$fName]/Value"/>
        
        <xsl:call-template name="substField">
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
    <xsl:param name="caseSensitive" select="1"/>

    <!-- convert search string obeying caseSensitive parameter -->
    <xsl:variable name="sReplace">
      <xsl:choose>
        <xsl:when test="$caseSensitive = 1">
          <xsl:value-of select="$replace"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="translate($replace,$loCase,$upCase)"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <!-- convert string obeying caseSensitive parameter -->
    <xsl:variable name="sStr">
      <xsl:choose>
        <xsl:when test="$caseSensitive = 1">
          <xsl:value-of select="$str"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="translate($str,$loCase,$upCase)"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    
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

</xsl:stylesheet>
