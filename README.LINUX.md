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

## Reporting Bugs
Please submit bugs via https://sourceforge.net/p/passwordsafe/bugs/.
Set the Category field to Linux to help ensure timely response.
