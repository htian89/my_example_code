Summary: The tcp-usbip
Name: tcp-usbip 
Version: 2.0
Release: 19%{?dist}
License: BSD
Group: System Environment/Libraries
URL: http://www.fronware.com/index.html

Packager: Tian Hao <tianhao@fronware.com>
Vendor:  Repository

Source: http://www.fronware.com/download.tcp-usbip/tcp-usbip-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
tcp-usbip is a fronware software

%prep
%setup -q

%build
make

%install
%{__rm} -rf %{buildroot}
mkdir -p %{buildroot}/opt/tcp-usbip
mkdir -p %{buildroot}%{_bindir}
cp usbip-driver/usbip-core.ko %{buildroot}/opt/tcp-usbip
cp usbip-driver/usbip.ko %{buildroot}/opt/tcp-usbip
cp tcp-usbip/tcp-usbip %{buildroot}%{_bindir}

%clean
%{__rm} -rf %{buildroot}

%files 
%defattr(-, root, root, 0755)
/opt/tcp-usbip/usbip.ko
/opt/tcp-usbip/usbip-core.ko
%{_bindir}/tcp-usbip
