## Introduction
Flatpak is universal installation package format for Linux. You can install Password Safe as flatpak from Flathub: https://flathub.org/apps/org.pwsafe.pwsafe

## Short Guide
Bellow guide was tested on Ubuntu and Fedora, but probably also works fine on other Linux distributions.

If you want to build flatpak by yourself, then do the following:
1. Create new directory. Newly created directory should not contain other files or subdirectories.
2. Download manifest file from Github https://github.com/flathub/org.pwsafe.pwsafe/blob/master/org.pwsafe.pwsafe.yml and save it to directory created in first step.
3. Execute commands:

   NOTICE: Be careful, bellow commands removes Password Safe flatpak from your local machine, if you have installed Password Safe flatpak from Flathub. If you like to retain the current installed Password Safe flatpak from Flathub, then replace every `org.pwsafe.pwsafe` string with new name like `org.pwsafe.pwsafe_new` and make sure to rename the `org.pwsafe.pwsafe.yml` file downloaded in step two to `org.pwsafe.pwsafe_new.yml`
   
   Bellow commands can take pretty long time. Copy bellow commands to new `make.sh` file and save it to directory created in step one, assign execute permission `chmod 744 make.sh` and execute file with `./make.sh`

```
#!/bin/bash

# -------------------------------
# On first error exit bash script
# -------------------------------
set -e


# ----------
# Start time
# ----------
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


# ----------
# End time
# ----------
date +'%Y-%m-%d %H:%M:%S'
```

5. After successful flatpak build and install, Password Safe program is automatically started for the first time. If you want to manually start it later, start it with: `flatpak run org.pwsafe.pwsafe &`

6. You can safely remove directory created in first step.
