1. Introduction:
The Linux port of PasswordSafe is currently in BETA.
This means that (1) you should take care to keep copies of the
database to protect against possible loss of data due to bugs, and (2)
there are several unimplemented features. Nonetheless, we feel that
this is good enough to release as an early beta to gather feedback
from a wider audience.

2. Supported Distributions:
Packages for the versions of Debian, Ubuntu and Fedora that were
current at the time of release may be found on the Github (primary)
and SourceForge (secondary) sites under
https://github.com/pwsafe/pwsafe/releases and
https://sourceforge.net/projects/passwordsafe/files/Linux-BETA/,
respectively.
If you don't find the package for the distribution of your choice, you
can contact the developers via https://pwsafe.org/contact.php or build
it yourself according to the instuctions listed in
README.LINUX.DEVELOPERS.md.
Slackware is independently supported, see below.

3. Installation on Debian or Ubuntu:
3.1 Download the .deb file that corresponds to your distribution.
3.2 Install it using dpkg:
$ sudo dpkg -i passwordsafe-*.deb
Dpkg will complain if prerequisite packages are missing. To install
the missing packages:
$ sudo aptitude install missing-pkgX missing-pkgY

4. Installation on Fedora:
3.1 Download the .rpm file that corresponds to your distribution.
3.2 Install it using yum:
$ sudo yum install passwordsafe-*.rpm

5. Slackware users can download SlackBuild for PasswordSafe from
https://slackbuilds.org, courtesy of rfmae (search for passwordsafe).

6. Reporting bugs:
Please submit bugs via https://sourceforge.net/p/passwordsafe/bugs/
Set the Category field to Linux to help ensure timely response.
