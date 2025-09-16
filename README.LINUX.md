## Introduction
The Linux port of Password Safe is currently stable, although lacking some of the more advanced features of the Windows version.

## Supported Distributions
Packages for the versions of Debian, Ubuntu and Fedora that were
current at the time of release may be found on the Github (primary)
and SourceForge (secondary) sites under
https://github.com/pwsafe/pwsafe/releases and
https://sourceforge.net/projects/passwordsafe/files/Linux/,
respectively.
If you don't find the package for the distribution of your choice, you
can contact the developers via https://pwsafe.org/contact.php or build
it yourself according to the instuctions listed in
README.LINUX.DEVELOPERS.md.
Slackware is independently supported, see below.

## Installation on Debian or Ubuntu

Password Safe is available as package (https://packages.debian.org/stable/passwordsafe). To install it just use the following command.

```
$ sudo apt install passwordsafe
```

or 

1. Download the .deb file that corresponds to your distribution.
2. Install it using dpkg:
   ```
   $ sudo dpkg -i passwordsafe-*.deb
   ```
   Dpkg will complain if prerequisite packages are missing. To install
   the missing packages:
   ```
   $ sudo apt -f install
   ```

## Installation on Fedora
1. Download the .rpm file that corresponds to your distribution.
2. Install it using dnf:
   ```
   $ sudo dnf install passwordsafe-*.rpm
   ```

## Installation on Gentoo
As usual there are USE flags to control the features of the package. 
On Gentoo, suport for Yubi keys and QR is disabled by default.

```
$ sudo emerge app-admin/passwordsafe
```

## Slackware
Slackware users can download SlackBuild for Password Safe from
https://slackbuilds.org, courtesy of rfmae (search for passwordsafe).

## Installation on Arch
The package description file (PKGBUILD) can be dowloaded via
```
$ git clone https://aur.archlinux.org/passwordsafe.git
```
After downloading, cd to the passwordsafe directory and run
```
$ makepkg
```
to download the code, compile and package it.
Finally, run
```
$ sudo pacman -U passwordsafe-*.zst
```
to install the package on your machine. 

For more details on building and installing packages on Arch, see https://wiki.archlinux.org/title/Arch_User_Repository

## Flatpak
Finally, Password Safe may be installed as a flatpak from Flathub:
```
$ flatpak install flathub org.pwsafe.pwsafe
$ flatpak run org.pwsafe.pwsafe
```
See https://flathub.org/setup to get started using flatpak.

## Enabling System Tray support in Password Safe
Issue: Password Safe is running but not appearing in the System Tray.

System Tray/Task Bar support in Linux is problematic, and the implementations for it are inconsistent or missing in many desktop environments. 
Usually, this feature is not supported by the windowing system such as Wayland. You may also need to install a desktop environment extension to enable System Tray support.
If Password Safe detects that the feature is not enabled at the system level, it is disabled in the program's Options. 

Anyway, enabling this feature in many distros can be done by following these steps:

1. GNOME, KDE Plasma, and others: Verify that the org.kde.StatusNotifierWatcher D-Bus interface that provides the System Tray is enabled. 
   Note: some extensions don't register a D-Bus interface when installed.
   ```
   $ busctl --user list | grep StatusNotifierWatcher
   ```
   If the item is listed, jump to Step 3; otherwise go to Step 2.

2. Install/Enable a Desktop Environment extension to enable System Tray support. Examples:

   Mint with Cinnamon: Enable the "System Tray" and "XApp Status Applet" applets in System Settings > Applets.

   GNOME: Install a GNOME extension for System Tray - a system package such as "gnome-shell-extension-appindicator". Then restart your GNOME session by logging out and logging back in.

   GNOME: Or, install a GNOME extension via web browser at https://extensions.gnome.org/, such as "AppIndicator and KStatusNotifierItem Support" or "Tray Icons: Reloaded".

   GNOME: Use the Extensions app to verify that the GNOME extension is installed and enabled.

   If a GNOME extension cannot be installed via a web browser: 
   - Download it as a ZIP file.
   - Install the extension using this command:
   ```
   $ gnome-extensions install <downloaded extension>.zip
   ```
   - Restart your GNOME session by logging out and logging back in.
   - Find the extension's UUID:
   ```
   $ gnome-extensions list
   ```
   - Enable the extension:
   ```
   $ gnome-extensions enable <extension UUID>
   ```

3. Wayland display server only: Make Password Safe use the X11/Xorg backend through the XWayland compatibility layer.

   a) Package version: Make Password Safe launch with the GDK_BACKEND=x11 environment variable. Edit the application's .desktop file as root.
   ```
   # vi /usr/share/applications/pwsafe.desktop
   ```
   Replace the line:
   ```
   Exec=pwsafe %f
   ```
   with "Exec=env GDK_BACKEND=x11 pwsafe %f", or with:
   ```
   Exec=sh -c '[ "$XDG_SESSION_TYPE" = "wayland" ] && exec env GDK_BACKEND=x11 pwsafe %f || exec pwsafe %f'
   ```

   b) Flatpak version: Edit the application's launcher properties and add extra parameters to the command field (default: flatpak run org.pwsafe.pwsafe). This will grant the sandboxed application access to the X11 display server socket, for inter-process communication with the System Tray interface.
   ```
   flatpak run --nosocket=wayland --socket=x11 org.pwsafe.pwsafe
   ```

4. Start Password Safe and enable support for System Tray: in the main menu, go to Manage > Options > System tab, then check Put icon in System Tray.

## Reporting Bugs
Please submit bugs via https://sourceforge.net/p/passwordsafe/bugs/.
Set the Category field to Linux to help ensure timely response.
