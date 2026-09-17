#!/bin/bash
FREEDESKTOP_VERSION=25.08

# --------------------------
# Exit script on first error
# --------------------------
set -e


# -----------------------
# Install flatpak-builder
# -----------------------
flatpak install -y flathub org.flatpak.Builder


# -----------------------
# Install flatpak SDK
# -----------------------
flatpak install -y flathub org.freedesktop.Platform//${FREEDESKTOP_VERSION} org.freedesktop.Sdk//${FREEDESKTOP_VERSION}


# -------------
# Build flatpak
# -------------
flatpak run org.flatpak.Builder --force-clean build-dir org.pwsafe.pwsafe.yml


# --------------------------------------------------
# Uninstall Password Safe flatpak from local machine
# --------------------------------------------------
# If you have installed Password Safe flatpak from e.g. Flathub, it will be uninstalled.
is_pwsafe_installed=$(flatpak list | grep "org.pwsafe.pwsafe" | wc -l)
if [[ $is_pwsafe_installed -eq 1 ]]; then
    flatpak uninstall -y org.pwsafe.pwsafe
fi


# --------------------------------
# Install flatpak on local machine
# --------------------------------
flatpak run org.flatpak.Builder --user --install --force-clean build-dir org.pwsafe.pwsafe.yml


# ----------------------
# List installed flatpak
# ----------------------
flatpak list | grep "org.pwsafe.pwsafe"


# -----------
# Run flatpak
# -----------
flatpak run org.pwsafe.pwsafe &
