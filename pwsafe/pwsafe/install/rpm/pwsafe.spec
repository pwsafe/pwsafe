Summary: Password Safe is a password database utility.
Name: pwsafe
Version: 0.8BETA
Release: 1
License: See /usr/share/doc/%{name}-%{version}/LICENSE
Group: Applications/Utils
Vendor: Rony Shapiro
URL: http://pwsafe.org/
Packager: David Dreggors <dadreggors@gmail.com>
Source:	pwsafe-0.8BETA-src.tgz
BuildRequires: gcc-c++, libXt-devel, libXtst-devel, libuuid-devel, xerces-c-devel, wxGTK-devel, make

%description
Password Safe is a password database utility. Like many other such
products, commercial and otherwise, it stores your passwords in an
encrypted file, allowing you to remember only one password (the "safe
combination"), instead of all the username/password combinations that
you use.

%prep
%setup -q

%build
make dist RPMBUILD="true"

%install
rm -rf ${RPM_BUILD_ROOT}
install -d -m 755 ${RPM_BUILD_ROOT}
install -d -m 755 ${RPM_BUILD_ROOT}/usr/bin
install -d -m 755 ${RPM_BUILD_ROOT}/usr/share/applications
install -d -m 755 ${RPM_BUILD_ROOT}/usr/share/icons/hicolor/48x48/apps
install -m 755 ${RPM_BUILD_DIR}/%{name}-%{version}/install/rpm/redhat/usr/bin/pwsafe ${RPM_BUILD_ROOT}/usr/bin
cp -r ${RPM_BUILD_DIR}/%{name}-%{version}/install/rpm/redhat/usr/share/* ${RPM_BUILD_ROOT}/usr/share
install -m 755 ${RPM_BUILD_DIR}/%{name}-%{version}/install/rpm/redhat/tmp/fedora-pwsafe.desktop ${RPM_BUILD_ROOT}/usr/share/applications
install -m 755 ${RPM_BUILD_DIR}/%{name}-%{version}/install/rpm/redhat/tmp/pwsafe.png ${RPM_BUILD_ROOT}/usr/share/icons/hicolor/48x48/apps

%clean
rm -rf ${RPM_BUILD_ROOT}

%post 

%postun

%files
%attr(755,root,root) %doc LICENSE docs
%attr(755,root,root) /usr/share/pwsafe
%attr(755,root,root) /usr/share/doc/passwordsafe
%attr(644,root,root) /usr/share/locale/de/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/locale/dk/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/locale/es/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/locale/fr/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/locale/it/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/locale/kr/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/locale/nl/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/locale/pl/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/locale/ru/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/locale/sv/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/locale/zh/LC_MESSAGES/pwsafe.mo
%attr(644,root,root) /usr/share/man/man1/pwsafe.1.gz
%attr(644,root,root) /usr/share/icons/hicolor/48x48/apps/pwsafe.png
%attr(644,root,root) /usr/share/applications/fedora-pwsafe.desktop
%attr(755, root, root) /usr/bin/pwsafe

%changelog
* Thu Nov 08 2012 David Dregors <dadreggors@gmail.com> - 0.8BETA-1
- Spec file created for Fedora rpm builds
- First rpm build for Fedora 17
