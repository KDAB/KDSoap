Name:           kdsoap
Version:        1.3.98
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
BuildRequires:  libqt4-devel cmake
%endif

%if %{defined fedora}
BuildRequires:  gcc-c++ qt-devel cmake desktop-file-utils
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
%if %{defined fedora}
%cmake . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
%else
%if "%{_lib}"=="lib64"
cmake . -DLIB_SUFFIX=64 -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
%else
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
%endif
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
%{_libdir}/libkdsoap.so.%{version}
%{_libdir}/libkdsoap-server.so.%{version}

%files devel
%defattr(-,root,root)
%{_bindir}/kdwsdl2cpp
%{_prefix}/share/mkspecs
%{_prefix}/share/mkspecs/features
%{_prefix}/share/mkspecs/features/kdsoap.prf
%{_includedir}/KDSoapClient
%{_includedir}/KDSoapServer
%{_libdir}/libkdsoap.so
%{_libdir}/libkdsoap-server.so
%{_libdir}/cmake/KDSoap

%changelog
* Tue Aug 12 2014 Allen Winter <allen.winter@kdab.com> 1.3.98
  1.4.0 RC1
* Thu Aug 07 2014 Allen Winter <allen.winter@kdab.com> 1.3.1
  1.3.1 final
* Mon Aug 04 2014 Allen Winter <allen.winter@kdab.com> 1.3.0
  1.3.0 final
