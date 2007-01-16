#!/bin/sh

# -- pwsafe.sh, [see _version] --

# == History ===================================================================================


#    V0.99  31.10.2005  shw	started development and understanding
#    V1.00  31.10.2005  shw	took care for links; make us absolute; beautified pathes
#    V1.01  31.10.2005  shw	handle hotplug directories which contain ":"s: cd to the real
#				directory and address with .\___
#    V1.02  01.11.2005  shw	switched from GPL2 to Artistic License; _version introduced
#    V1.03  02.11.2005  shw	introduced environment check to make use of the GNU Utilities 
#				for Win32, added functionality for using their sh/zsh; added
#				"-d ..." for debugging and redirected stdout/err
#    V1.04  04.11.2005  shw	redesigned script; enhanced environment check; introduced -j 
#				and usage of "-?" or "/?" plus UPPER or lowercase

# == Short Description =========================================================================

#    + adapted from pwsafe.sh by Glen Smith <glen@bytecode.com.au>
#    + fire up PasswordSafeSWT with all it's JARs whether we are a (multiple) link, relatively 
#      or absolutely addressed
#    + tested with SuSE 9.2, WinXP.Prof, Win2003.Srvr; neither have a MAC nor any ideas about 
#      it; different WinXYZ showed different behaviour, so you are on your own to test; we need 
#      to be called "pwsafe.sh" due to Windows 
#    + might be used as is under the Artistic License which can be found at
#      http://www.opensource.org/licenses/artistic-license.php

#    Axel Schwarzer; Have a lot of fun...

# == Syntax for calling me =====================================================================

#    [{./|../|/path/}pwsafe.{cmd|sh} [{-|/}{d|D}] [{-|/}{j|J} java options]

#    pwsafe.cmd is needed for WinXYZ and pwsafe.sh should work for Linux and MAC

# == ===========================================================================================


# -- variables --
_classpath=""
_javalibpath=""
_javaopts=""
_new=""
_OSEnv=""; _OS=""; _OSPlatform=""; _OSString=""	# -- mostly hardcoded now --
_pathfilesep=""; _pathsep=""
_prefprop=""
_prg="${0}"
_vaultdir=""
_version="V1.04  04.11.2005, shw@schwarzer.d.uunet.de"


# -- functions --
f_UnlinkMe () {					# -- if we are a link, find our target --
   while [ -L "${_prg}" ]; do
       _link="$(ls -ld "${_prg}" | sed -e 's+^.*-> \(.*\)$+\1+')"

       if echo "${_link}" | grep ^${_pathfilesep} 1>/dev/null 2>&1; then
          _prg="${_link}"
       else
          _prg="$(dirname ${_prg})${_pathfilesep}${_link}"
       fi
   done

   return 0
}


# -- check environment --
case "$(uname)" in
   Darwin*)
      _OSEnv="carbon"; _OS="macosx"; _OSPlatform="ppc"

          _pathsep=':'
      _pathfilesep='/'
              HOME="$(echo ~)"
              TEMP="${TEMP:=/tmp}"

      f_UnlinkMe
   ;;
   Linux*)
      _OSEnv="gtk"; _OS="linux"; _OSPlatform="x86"

          _pathsep=':'
      _pathfilesep='/'
              HOME="$(echo ~)"
              TEMP="${TEMP:=/tmp}"

      f_UnlinkMe
   ;;
   Windows*)
      _OSEnv="win32"; _OS="win32"; _OSPlatform="x86"

          _pathsep=';'
      _pathfilesep='\'
              HOME="${HOMEDRIVE}${HOMEPATH}"
   ;;
   *)
      echo "(E) Unsupported OS '$(uname)' found." 1>&2

      exit 1
   ;;
esac


# -- now set some variables --
_OSString="${_OSEnv}.${_OS}.${_OSPlatform}"
_javalibpath=".${_pathfilesep}"
_vaultdir="${HOME}${_pathfilesep}.passwordsafe"
_prefprop="${_vaultdir}${_pathfilesep}preferences.properties"


# -- save stdout and stderr to a file in temp storage --
exec 1>"${TEMP}${_pathfilesep}pws_start.log" 2>&1

echo "(I) Environment used: ${_OSString}."


# -- check for debug switch --
if echo "${1}" | grep ^[-/][dD] 1>/dev/null 2>&1; then
   shift

   echo "(I) Running in debug mode."

   _javaopts="-showversion -verbose"

   echo "(I) Your environment consists of:"
   set
   echo "----+----1----+----2----+----3----+----4----+----5----+----6----+----7-!--+----8----+----9----+!----10---+----11---+----12---+----132"

   set -xv
fi


# -- check for java options by user --
if echo "${1}" | grep ^[-/][jJ] 1>/dev/null 2>&1; then
   shift

   echo "(I) Running java with options."

   _javaopts="${_javaopts} ${1}"
fi


# -- make us absolute, PWD is **IX var, contains always slash --
if [ "${_prg}" = "$(basename "${_prg}")" ]; then
   _prg="${PWD}/${_prg}"
else
   if [ "$(echo "${_prg}" | grep ^\.)" ]; then
      cd "$(dirname "${_prg}")"

      _prg="${PWD}/$(basename "${_prg}")"
   else
      _prg="${_prg}"
   fi
fi


# -- remove 'path and one level up' and multiple /es (often found in AIX) --
_prg="$(echo "${_prg}" | sed -e 's+/[^/]*/[.][.]/++g' -e 's|/\{2,9\}|/|g')"

cd "$(dirname ${_prg})"


# -- collect all jars absolute --
for _jar in $(ls .${_pathfilesep}*.jar ${_javalibpath}${_pathfilesep}*.jar); do
   _classpath="${_classpath}${_pathsep}${_jar}"
done

_classpath="$(echo "${_classpath}" | sed -e 's+^[:;]++')"


# -- cd to private config dir (perhaps we need to make it) --
if [ -d "${_vaultdir}" ]; then
   _new="(already prepared)"
else
   echo "(N) Initializing private directory; Don't forget to create the new data store (safe)!"

   mkdir "${_vaultdir}"

   [ "${_OS}" = "win32" ] || chmod 700 "${_vaultdir}"

   touch "${_prefprop}"

   # -- try to set some reasonable defaults --
   echo "# -- User Preferences for PasswordSafeSWT; $(date) --"			>> "${_prefprop}"
   echo "show.password.in.list=false"						>> "${_prefprop}"
   echo "use.symbols=true"							>> "${_prefprop}"
   echo "clear.clipboard.on.minimize=true"					>> "${_prefprop}"
   echo "save.immediately.on.edit=true"						>> "${_prefprop}"
   echo "use.uppercase.letters=true"						>> "${_prefprop}"
   echo "default.password.length=8"						>> "${_prefprop}"
   echo "use.digits=true"							>> "${_prefprop}"
   echo "use.lowercase.letters=true"						>> "${_prefprop}"

   [ "${_OS}" = "win32" ] || chmod 600 "${_vaultdir}"${_pathfilesep}*

   _new="(new)"
fi


# -- be verbose before start --
echo ""
echo "------------- - ----------------------------------------------------------------"
echo "            0 = ${0}"
echo "         _prg = ${_prg}"
echo "     _version = ${_version}"
echo ""
echo "          PWD = ${PWD}"
echo " _javalibpath = ${_javalibpath}"
echo "   _classpath = ${_classpath}"
echo "    _vaultdir = ${_vaultdir} ${_new}"
echo ""
echo "         HOME = ${HOME}"
echo "         TEMP = ${TEMP}"
echo "         PATH = ${PATH}"
echo "------------- - ----------------------------------------------------------------"
echo ""


# -- start java with main class --
java ${_javaopts} -Djava.library.path="${_javalibpath}" -cp "${_classpath}" org.pwsafe.passwordsafeswt.PasswordSafeJFace

[ "${_OS}" = "win32" ] || chmod 600 "${_vaultdir}"${_pathfilesep}*


# -- end of pwsafe.sh --
