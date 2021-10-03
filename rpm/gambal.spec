%define debug_package %{nil}


Name:           gambal
Version:        1.2
Release:        1%{?dist}
Summary:        Graphical system resource monitor
License:        GPLv2
URL:            https://github.com/ashtum/%{name}
BugURL:         https://github.com/ashtum/%{name}/issues
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  libX11-devel, gcc, gcc-c++, cmake
Requires:       libX11, xorg-x11-fonts-misc


%description
A tiny transparent window that provides an overview of network, CPU and memory usages.


%prep
%autosetup


%build
%cmake
%cmake_build


%install
rm -rf $RPM_BUILD_ROOT
%cmake_install


%files
/usr/bin/*
/usr/share/*
/etc/*

