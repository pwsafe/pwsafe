#!/bin/sh

# -- pwsafe, [see _version] --

#    adapted from pwsafe.sh by Glen Smith <glen@bytecode.com.au>
#    fire up PasswordSafeSWT with all it's JARs whether we are a (multiple) link,
#    relatively or absolutely addressed
#    tested with SuSE 9.2
#    might be used as is under the Artistic License which can be found at
#    http://www.opensource.org/licenses/artistic-license.php

#    V0.99  31.10.2005  shw	started development and understanding
#    V1.00  31.10.2005  shw	took care for links; make us absolute; beautified
#				pathes
#    V1.01  31.10.2005  shw	handle hotplug directories which contain ":"s
#    V1.02  01.11.2005  shw	switched from GPL2 to Artistic License; _version
#				introduced

#    Axel Schwarzer; Have a lot of fun...


# -- if we are a link, find our target --
_prg="${0}"

while [ -L "${_prg}" ]; do
    _link="$(ls -ld "${_prg}" | sed -e 's+^.*-> \(.*\)$+\1+')"
    if echo "${_link}" | grep '^/' 1>/dev/null 2>&1; then
        _prg="${_link}"
    else
        _prg="$(dirname ${_prg})/${_link}"
    fi
done


# -- make us absolute --
if [ "${_prg}" = "$(basename "${_prg}")" ]; then
   _prg="${PWD}/${_prg}"
else
   if [ "$(echo "${_prg}" | grep "^\.")" ]; then
      cd "$(dirname "${_prg}")"

      _prg="${PWD}/$(basename "${_prg}")"
   else
      _prg="${_prg}"
   fi
fi


# -- remove 'path and one level up' and multiple /es (often found in AIX) --
_prg="$(echo "${_prg}" | sed -e 's+/[^/]*/[.][.]/++g' -e 's|/\{2,9\}|/|g')"


# -- set variables --
HOME="$(echo ~)"
JAR_CLASSPATH=""
JAR_LIBPATH="$(dirname ${_prg})"
VAULT_DIR="${HOME}/.passwordsafe"
_version="V1.02  31.10.2005, shw@schwarzer.d.uunet.de"


# -- handle hotplug directories (make sure they are mounted with -o exec) --
if echo "${JAR_LIBPATH}" | grep ':' 1>/dev/null 2>&1; then
   echo "(N) ${JAR_LIBPATH} contains ':'; perhaps hotplug directory."

   cd "${JAR_LIBPATH}"

   JAR_LIBPATH="."
fi


# -- collect all jars absolute --
for _jar in $(ls ${JAR_LIBPATH}/*.jar); do
   JAR_CLASSPATH="${JAR_CLASSPATH}:${_jar}"
done

JAR_CLASSPATH="$(echo "${JAR_CLASSPATH}" | sed -e 's+^:++')"


# -- cd to private config dir (perhaps we need to make it) --
if [ -d "${VAULT_DIR}" ]; then
   cd "${VAULT_DIR}"

   _new=""
else
   echo "(N) Initializing private directory."
   echo "(+) Don't forget to create the new data store (safe)!"

   mkdir "${VAULT_DIR}"

   chmod 700 "${VAULT_DIR}"

   touch "${VAULT_DIR}/MyVault" 

   # -- try to set some reasonable defaults --
   cat << E-O-D >"${VAULT_DIR}/preferences.properties"
# -- User Preferences for PasswordSafeSWT; $(date) --
mru.1=${HOME}/.passwordsafe/MyVault
show.password.in.list=false
use.symbols=true
clear.clipboard.on.minimize=true
save.immediately.on.edit=true
use.uppercase.letters=true
default.password.length=8
use.digits=true
use.lowercase.letters=true
E-O-D

   chmod 600 "${VAULT_DIR}"/*

   cd "${VAULT_DIR}"

   _new="»new«"
fi


# -- handle hotplug directories --
if [ "${JAR_LIBPATH}" = "." ]; then
   cd -
fi


# -- be verbose if not requested to be "q"uiet --
if [ "${1}" != "q" ]; then
   cat << E-O-D
------------- - ----------------------------------------------------------------
            0 = ${0}
         _prg = ${_prg}
     _version = ${_version}

          PWD = ${PWD}
  JAR_LIBPATH = ${JAR_LIBPATH}
JAR_CLASSPATH = ${JAR_CLASSPATH}
         HOME = ${HOME}
    VAULT_DIR = ${VAULT_DIR} ${_new}
        VAULT = ${VAULT_DIR}/MyVault
------------- - ----------------------------------------------------------------
E-O-D
fi


# -- start java with main class --
java -Djava.library.path="${JAR_LIBPATH}" -cp "${JAR_CLASSPATH}" org.pwsafe.passwordsafeswt.PasswordSafeJFace

chmod 600 "${VAULT_DIR}"/*


# -- end of pwsafe --