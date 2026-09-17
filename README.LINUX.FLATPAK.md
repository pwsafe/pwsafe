## End-users
Flatpak is a universal installation package format for Linux. You can install Password Safe as a flatpak from Flathub: https://flathub.org/apps/org.pwsafe.pwsafe.

## Developers
This guide was tested on Ubuntu and Fedora, but it should also work for other Linux distributions.

If you want to build flatpak by yourself, then do the following:
1. Create a new directory, e.g. `pwsafe-flatpak`. This directory should not contain other files or subdirectories.
2. Download the flatpak manifest file from Github https://github.com/flathub/org.pwsafe.pwsafe/blob/master/org.pwsafe.pwsafe.yml and save it the directory created in the first step.
3. Get the `make-flatpak.sh` script from the Misc directory in pwsafe source tree and save it in the directory created in step 1. Make the script executable and run it. The process may take some time.

   NOTICE: Be careful, the script removes any existing Password Safe flatpak from your local machine, if you have installed the Password Safe flatpak from Flathub. If you'd like to keep the currently installed Password Safe flatpak from Flathub, then replace every `org.pwsafe.pwsafe` string in the manifest with a new name such as `org.pwsafe.pwsafe_new` and make sure to rename the `org.pwsafe.pwsafe.yml` file downloaded in step 2 to `org.pwsafe.pwsafe_new.yml`.
   
4. After flatpak builds and installs successfully, the script will start the Password Safe program for the first time. To run it again, type `flatpak run org.pwsafe.pwsafe &`.

5. (Optional) Make the Flatpak transferable as one standalone file by creating a Flatpak bundle:
   
   Create a local Flatpak repository called `pwsaferepo` from the build directory, type `flatpak build-export pwsaferepo build-dir master`.
   
   Create the standalone file, type `flatpak build-bundle pwsaferepo pwsafe.flatpak org.pwsafe.pwsafe master`.
   
   Copy pwsafe.flatpak to another machine and install the bundle using: `flatpak install -y pwsafe.flatpak`.
   
   If the Freedesktop runtime is not installed automatically with the bundle, install it from Flathub first (the current required version is 25.08): `flatpak install -y flathub org.freedesktop.Platform/x86_64/25.08`.

6. You can now safely remove directory created in step 1.
