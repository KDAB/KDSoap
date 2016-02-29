Name:           kdsoap
Version:        1.4.99
Release:        1
Summary:        A Qt-based client-side and server-side SOAP component
Source:         %{name}-%{version}.tar.gz
Url:            http://github.com/KDAB/KDSoap
Group:          System Environment/Libraries
License:        LGPL-2.1+
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Vendor:         Klaralvdalens Datakonsult AB (KDAB)
Packager:       Klaralvdalens Datakonsult AB (KDAB) <info@kdab.com>

%if %{defined suse_version}
BuildRequires:  libqt4-devel
%endif

%if %{defined fedora}
BuildRequires:  gcc-c++ qt-devel desktop-file-utils
%endif

%if %{defined rhel}
BuildRequires:  gcc-c++ qt-devel desktop-file-utils
%endif

%description
KDSoap can be used to create client applications for web services
and also provides the means to create web services without the need
for any further component such as a dedicated web server.

Authors:
--------
      Klaralvdalens Datakonsult AB (KDAB) <info@kdab.com>

%package devel
Summary:        Development files for %{name}
Group:          Development/System
Requires:       %{name} = %{version}

%description devel
This package contains header files and associated tools and libraries to
develop programs which need to access web services using the SOAP protocol.

%prep
%setup -q

%build
touch .license.accepted
%if "%{_lib}"=="lib64"
QMAKE_ARGS="LIB_SUFFIX=64" ./configure.sh -shared -release -prefix %{buildroot}/usr
%else
./configure.sh -shared -release -prefix %{buildroot}/usr
%endif
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
%{_libdir}/libkdsoap.so
%{_libdir}/libkdsoap-server.so

%changelog
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
