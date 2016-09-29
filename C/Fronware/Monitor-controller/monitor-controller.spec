Summary: The monitor-controller
Name: monitor-controller
Version: 1.0
Release: 1%{?dist}
License: BSD
Group: System Environment/Libraries
URL: http://www.fronware.com/index.html

Packager: Tian Hao <tianhao@fronware.com>
Vendor:  Repository

Source: http://www.fronware.com/download.tcp-usbip/monitor-controller.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
monitor-controller is a fronware software

%prep
%setup -q -n monitor-controller -c

%build
cd monitor-controller
make
cd ..

%install
cd monitor-controller
%{__rm} -rf %{buildroot}
mkdir -p %{buildroot}/opt/vncviewer
mkdir -p %{buildroot}/opt/monitor-controller
mkdir -p %{buildroot}/etc/xdg/autostart
mkdir -p %{buildroot}%{_bindir}
install -p passwd %{buildroot}/opt/vncviewer
install -p monitor-controller.desktop %{buildroot}/etc/xdg/autostart
install -p monitor-controller %{buildroot}%{_bindir}
cd ..

%clean
%{__rm} -rf %{buildroot}

%files 
%defattr(-, root, root, 0755)
/opt/vncviewer/passwd
/opt/monitor-controller
/etc/xdg/autostart/monitor-controller.desktop
%{_bindir}/monitor-controller
