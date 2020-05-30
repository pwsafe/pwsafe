#!/usr/bin/busybox sh
# Created by Jacob Hrbek <kreyren@rixotstudio.cz> under GPLv3 license <https://www.gnu.org/licenses/gpl-3.0.en.html> in 30/05/2020 for github.com/ronys at https://github.com/pwsafe/pwsafe or at github.com/ronys's preferred license

# shellcheck shell=sh

###! Script to provide dependencies to work on this repository
###! Requires:
###! - DNM: TBD
###! Platforms:
###! - [ ] Linux
###!  - [?] Debian
###!  - [ ] Ubuntu
###! - [ ] FreeBSD
###! - [ ] Darwin
###! - [ ] Redox
###! - [ ] ReactOS
###! - [ ] Windows
###! - [ ] Windows/Cygwin

# Command overrides
[ -z "$PRINTF" ] && PRINTF="printf"
[ -z "$WGET" ] && WGET="wget"
[ -z "$CURL" ] && CURL="curl"
[ -z "$ARIA2C" ] && ARIA2C="aria2c"
[ -z "$CHMOD" ] && CHMOD="chmod"

# DNM: Used for debugging
unset SUDO

# Exit on anything unexpected
set -e

# NOTICE(Krey): By default busybox outputs a full path in '$0' this is used to strip it
myName="${0##*/}"

# Used to prefix logs with timestemps, uses ISO 8601 by default
logPrefix="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
# Path to which we will save logs
logPath="$HOME/.$myName.log"

# inicialize the script in logs
printf '%s\n' "Started $myName on $(uname -s) at $(date -u +"%Y-%m-%dT%H:%M:%SZ")" >> "$logPath"

# NOTICE(Krey): Aliases are required for posix-compatible line output (https://gist.github.com/Kreyren/4fc76d929efbea1bc874760e7f78c810)
die() { funcname=die
	case "$2" in
		38|fixme) # FIXME
			if [ "$DEBUG" = 0 ] || [ -z "$DEBUG" ]; then
				printf 'FATAL: %s, fixme?\n' "$3"
				printf "${logPrefix}FATAL($myName:$1): %s, fixme?\\n" "$3" >> "$logPath"
				unset funcname
			elif [ "$DEBUG" = 1 ]; then
				printf "FATAL($0:$1): %s, fixme?\n" "$3"
				printf "${logPrefix}FATAL($myName:$1): %s, fixme?\\n" "$3" >> "$logPath"
				unset funcname
			else
				# NOTICE(Krey): Do not use die() here
				printf 'FATAL: %s\n' "Unexpected happend while processing variable DEBUG with value '$DEBUG' in $funcname"
			fi

			exit 38
		;;
		255) # Unexpected trap
			if [ "$DEBUG" = 0 ] || [ -z "$DEBUG" ]; then
				printf 'FATAL: Unexpected happend while %s\n' "$3"
				printf "${logPrefix}FATAL($myName:$1): Unexpected happend while %s\\n" "$3" >> "$logPath"
				unset funcname
			elif [ "$DEBUG" = 1 ]; then
				printf "FATAL($0:$1): Unexpected happend while %s\n" "$3"
				printf "${logPrefix}FATAL($myName:$1): Unexpected happend while %s\\n" "$3" >> "$logPath"
				unset funcname
			else
				# NOTICE(Krey): Do not use die() here
				printf 'FATAL: %s\n' "Unexpected happend while processing variable DEBUG with value '$DEBUG' in $funcname"
			fi
		;;
		*)
			if [ "$DEBUG" = 0 ] || [ -z "$DEBUG" ]; then
				printf 'FATAL: %s\n' "$3"
				printf "${logPrefix}FATAL($myName:$1): %s\\n" "$3" >> "$logPath"
				unset funcname
			elif [ "$DEBUG" = 1 ]; then
				printf "FATAL($0:$1): %s\n" "$3"
				printf "${logPrefix}FATAL($myName:$1): %s\\n" "$3" >> "$logPath"
				unset funcname
			else
				# NOTICE(Krey): Do not use die() here
				printf 'FATAL: %s\n' "Unexpected happend while processing variable DEBUG with value '$DEBUG' in $funcname"
			fi
	esac

	exit "$2"

	# In case invalid argument has been parsed in $2
	exit 255
}; alias die='die "$LINENO"'

einfo() { funcname=einfo
	if [ "$DEBUG" = 0 ] || [ -z "$DEBUG" ]; then
		printf 'INFO: %s\n' "$2"
		printf "${logPrefix}INFO($myName:$1): %s\\n" "$2" >> "$logPath"
		unset funcname
		return 0
	elif [ "$DEBUG" = 1 ]; then
		printf "INFO($0:$1): %s\n" "$2"
		printf "${logPrefix}INFO($myName:$1): %s\\n" "$2" >> "$logPath"
		unset funcname
		return 0
	else
		die 255 "processing variable DEBUG with value '$DEBUG' in $funcname"
	fi
}; alias einfo='einfo "$LINENO"'

ewarn() { funcname=ewarn
	if [ "$DEBUG" = 0 ] || [ -z "$DEBUG" ]; then
		printf 'WARN: %s\n' "$2"
		printf "${logPrefix}WARN($myName:$1): %s\\n" "$2" >> "$logPath"
		unset funcname
		return 0
	elif [ "$DEBUG" = 1 ]; then
		printf "WARN($0:$1): %s\n" "$2"
		printf "${logPrefix}WARN($myName:$1): %s\\n" "$2" >> "$logPath"
		unset funcname
		return 0
	else
		die 255 "processing variable DEBUG with value '$DEBUG' in $funcname"
	fi
}; alias ewarn='ewarn "$LINENO"'

eerror() { funcname=eerror
	if [ "$DEBUG" = 0 ] || [ -z "$DEBUG" ]; then
		printf 'ERROR: %s\n' "$2"
		printf "${logPrefix}ERROR($myName:$1): %s\\n" "$2" >> "$logPath"
		unset funcname
		return 0
	elif [ "$DEBUG" = 1 ]; then
		printf "ERROR($0:$1): %s\n" "$2"
		printf "${logPrefix}ERROR($myName:$1): %s\\n" "$2" >> "$logPath"
		unset funcname
		return 0
	else
		die 255 "processing variable DEBUG with value '$DEBUG' in $funcname"
	fi
}; alias eerror='eerror "$LINENO"'

edebug() { funcname=edebug
	if [ "$DEBUG" = 0 ] || [ -z "$DEBUG" ]; then
		printf 'DEBUG: %s\n' "$2"
		printf "${logPrefix}ERROR($myName:$1): %s\\n" "$2" >> "$logPath"
		unset funcname
		return 0
	elif [ "$DEBUG" = 1 ]; then
		printf "DEBUG($0:$1): %s\n" "$2"
		printf "${logPrefix}DEBUG($myName:$1): %s\\n" "$2" >> "$logPath"
		unset funcname
		return 0
	else
		die 255 "processing variable DEBUG with value '$DEBUG' in $funcname"
	fi
}; alias eerror='eerror "$LINENO"'

efixme() { funcname=efixme
	if [ "$IGNORE_FIXME" = 1 ]; then
		true
	elif [ "$IGNORE_FIXME" = 0 ] || [ -z "$IGNORE_FIXME" ]; then
		if [ "$DEBUG" = 0 ] || [ -z "$DEBUG" ]; then
			printf 'FIXME: %s\n' "$2"
			printf "${logPrefix}FIXME($myName:$1): %s\\n" "$2" >> "$logPath"
			unset funcname
			return 0
		elif [ "$DEBUG" = 1 ]; then
			printf "FIXME($0:$1): %s\n" "$2"
			printf "${logPrefix}FIXME($myName:$1): %s\\n" "$2" >> "$logPath"
			unset funcname
			return 0
		else
			die 255 "processing DEBUG variable with value '$DEBUG' in $funcname"
		fi
	else
		die 255 "processing variable IGNORE_FIXME with value '$IGNORE_FIXME' in $0"
	fi
}; alias efixme='efixme "$LINENO"'

KERNEL="$(uname -s)"

# Identify system and core
if command -v uname 1>/dev/null; then
	case "$KERNEL" in
		Linux)
			# Identify distribution
			if command -v lsb_release 1>/dev/null; then
				# NOTICE(Krey): Command 'lsb_release -si' returns values alike 'Debian' where logic expects 'debian' -> piped in tr to standardize
				DISTRO="$(lsb_release -si | tr '[:upper:]' '[:lower:]')"
			elif [ -f /etc/os-release ]; then
				DISTRO="$(grep -o "^ID\=.*" /etc/os-release | sed 's#^ID\=##gm')"
			elif ! command -v lsb_release 1>/dev/null && [ ! -f /etc/os-release ]; then
				die 1 "Unable to identify distribution since command 'lsb_release' and file /etc/os-release are not present"
			else
				die 255 "identifying distro in $myName running logic for Linux"
			fi

			# FIXME(Krey): Better logic needed
			RELEASE="$(lsb_release -cs)"

			# Install dependencies
			case "$DISTRO/$RELEASE" in
				debian/*)
					efixme "Implement logic for dependencies"
					$SUDO apt-get install -qy \
						cmake \
						gettext \
						libwxgtk3.0-dev \
						shellcheck
				;;
				ubuntu/*)
					efixme "Implement logic for dependencies"
					$SUDO apt-get install -qy \
						shellcheck
				;;
				archlinux/*)
					efixme "Implement logic for dependencies" 
				;;
				alpine/*)
					efixme "Implement logic for dependencies" 
				;;
				nixos/*)
					efixme "Implement logic for dependencies" 
				;;
				*) die fixme "Unsupported distribution '$DISTRO' has been parsed in $myName"
			esac
		;;
		FreeBSD)
			efixme "Implement logic for dependencies"
		;;
		OpenBSD)
			efixme "Implement logic for dependencies"
		;;
		Redox)
			efixme "Implement logic for dependencies"
		;;
		Windows)
			efixme "Implement logic for dependencies"
		;;
		*) die fixme "Platform '$(uname -s)' is not supported by this script"
	esac
elif ! command -v uname 1>/dev/null; then
	# FIXME(Krey): Logic implementation?
	die 1 "Required command uname is not available on this environment, unable to identify kernel"
else
	die 255 "processing uname with value $(uname -s)"
fi