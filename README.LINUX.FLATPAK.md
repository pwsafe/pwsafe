## Introduction
Flatpak is a universal installation package format for Linux. You can install Password Safe as a flatpak from Flathub: https://flathub.org/apps/org.pwsafe.pwsafe

## Short Guide
This guide was tested on Ubuntu and Fedora, but should also works for other Linux distributions.

If you want to build flatpak by yourself, then do the following:
1. Create a new directory. This directory should not contain other files or subdirectories.
2. Download the flatpak manifest file from Github https://github.com/flathub/org.pwsafe.pwsafe/blob/master/org.pwsafe.pwsafe.yml and save it the directory created in first step.
3. Execute the following commands in the same directory:

   NOTICE: Be careful, these commands remove any existing Password Safe flatpak from your local machine, if you have installed the Password Safe flatpak from Flathub. If you'd like to keep the currently installed Password Safe flatpak from Flathub, then replace every `org.pwsafe.pwsafe` string in the manifest with a new name such as `org.pwsafe.pwsafe_new` and make sure to rename the `org.pwsafe.pwsafe.yml` file downloaded in step two to `org.pwsafe.pwsafe_new.yml`
   
   The following commands can take some time. Copy them commands to a `make.sh` file and save it in the directory created in step one, set execute permission `chmod u+x make.sh` and execute it with `./make.sh`

```
#!/bin/bash

# --------------------------
# Exit script on first error
# --------------------------
set -e


# ---------------
# Note start time
# ---------------
date +'%Y-%m-%d %H:%M:%S'


# -----------------------
# Install flatpak-builder
# -----------------------
flatpak install -y flathub org.flatpak.Builder


# -----------------------
# Install flatpak SDK
# -----------------------
flatpak install -y flathub org.freedesktop.Platform//23.08 org.freedesktop.Sdk//23.08


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
flatpak list | grep pwsafe


# -----------
# Run flatpak
# -----------
flatpak run org.pwsafe.pwsafe &


# -------------
# Note end time
# -------------
date +'%Y-%m-%d %H:%M:%S'
```

5. After flatpak builds and installs successfully, the script will start the Password Safe program is for the first time. To run it again, type `flatpak run org.pwsafe.pwsafe &`

6. You can now safely remove directory created in first step.
