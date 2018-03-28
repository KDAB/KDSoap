Name:           qt5-kdsoap
Version:        1.7.0
Release:        1
Summary:        A Qt5-based client-side and server-side SOAP component
Source:         %{name}-%{version}.tar.gz
Url:            http://github.com/KDAB/KDSoap
Group:          System/Libraries
License:        GPL-2.0+
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Vendor:         Klaralvdalens Datakonsult AB (KDAB)
Packager:       Klaralvdalens Datakonsult AB (KDAB) <info@kdab.com>

BuildRequires: cmake
%if %{defined suse_version}
BuildRequires:  libqt5-qtbase-devel
%endif

%if %{defined fedora}
BuildRequires:  gcc-c++ qt5-qtbase-devel desktop-file-utils
%endif

%if %{defined rhel}
BuildRequires:  gcc-c++ qt5-qtbase-devel desktop-file-utils
%endif

%description
KDSoap can be used to create client applications for web services
and also provides the means to create web services without the need
for any further component such as a dedicated web server.

Authors:
--------
      Klaralvdalens Datakonsult AB (KDAB) <info@kdab.com>

%define debug_package %{nil}
%global __debug_install_post %{nil}

%package devel
Summary:        Development files for %{name}
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}

%description devel
This package contains header files and associated tools and libraries to
develop programs which need to access web services using the SOAP protocol.

%prep
%setup -q

%build
touch .license.accepted
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_SKIP_RPATH=True -DCMAKE_BUILD_TYPE=Release
%__make %{?_smp_mflags}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%install
%make_install

%clean
%__rm -rf "%{buildroot}"

%files
%defattr(-,root,root)
%{_prefix}/share/doc/KDSoap
%{_libdir}/libkdsoap.so.*
%{_libdir}/libkdsoap-server.so.*

%files devel
%defattr(-,root,root)
%{_bindir}/kdwsdl2cpp
%{_prefix}/share/mkspecs
%{_includedir}/KDSoapClient
%{_includedir}/KDSoapServer
%dir %{_libdir}/cmake/KDSoap
%{_libdir}/cmake/KDSoap/*
%{_libdir}/libkdsoap.so
%{_libdir}/libkdsoap-server.so

%changelog
* Mon Mar 01 2018 Allen Winter <allen.winter@kdab.com> 1.7.0
  1.7.0
* Mon May 01 2017 Allen Winter <allen.winter@kdab.com> 1.6.0
  1.6.0
* Tue Jun 07 2016 Allen Winter <allen.winter@kdab.com> 1.5.1
  1.5.1 bug fix
* Mon Feb 29 2016 Allen Winter <allen.winter@kdab.com> 1.4.99
  1.5.0 RC1
* Thu Jun 25 2015 Allen Winter <allen.winter@kdab.com> 1.4.0
  1.4.0 with the autogen buildsystem
* Sun Aug 24 2014 Allen Winter <allen.winter@kdab.com> 1.4.0
  1.4.0 Final
* Tue Aug 12 2014 Allen Winter <allen.winter@kdab.com> 1.3.98
  1.4.0 RC1
* Thu Aug 07 2014 Allen Winter <allen.winter@kdab.com> 1.3.1
  1.3.1 final
* Mon Aug 04 2014 Allen Winter <allen.winter@kdab.com> 1.3.0
  1.3.0 final
