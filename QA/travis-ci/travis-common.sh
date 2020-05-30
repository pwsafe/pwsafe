#!/bin/sh
# Simplified error handling
die() {
	printf 'FATAL: %s\n' "$2"
	exit "$1"
}
# Simplified QA handling
fixme() {
	printf 'FIXME: %s\n' "$1"
}
# Simplified info messages
info() {
	printf 'INFO: %s\n' "$1"
}
warn() {
    printf 'WARN: %s\n' "$1"
}