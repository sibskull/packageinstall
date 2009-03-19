
Name:		packageinstall
Version:	1.0.0
Release:	alt1
Summary:	GUI frontend for install packages using apt-get

License:	GPL
Group:		System/Configuration/Packaging
URL:		http://www.altlinux.org/PackageInstall

Packager:   Andrey Cherepanov <cas@altlinux.org>

Source0:	%name-%version.tar.gz

BuildRequires: gcc-c++ libqt4-devel
Requires: apt

%description
This application is GUI frontend for install package(s) using apt-get.

%prep
%setup -q
subst "s,/usr/local,%buildroot,g" ./config.pri
qmake-qt4 %name.pro

%build
lrelease-qt4 %name.pro
%make_build

%install
%makeinstall

%files
%doc AUTHORS README
%_bindir/%name
%_datadir/apps/%name/

%changelog
* Thu Mar 19 2009 Andrey Cherepanov <cas@altlinux.org> 1.0.0-alt1
- Initial release


