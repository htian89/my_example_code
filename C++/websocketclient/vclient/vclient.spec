Summary: The vClient is a application for managing virtual desktops' connections.
Name: vclient
Version: 2.9.2.68
Release: 2%{?dist}
License: Commercial
Group: Applications/System
URL: http://www.fronware.com/index.html

Packager: zhangyongliang <zhangyongliang@fronware.com>
Vendor:  Fronware Co.Ltd

Source0: http://www.fronware.com/download/vclient/vclient-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}%-{release}-root

BuildRequires: mxml
BuildRequires: qt
BuildRequires: openssl
Requires: bind-utils
Requires: traceroute
%description
vClient is a fronware software

%prep
%setup -q

#%build
#qmake-qt4 -makefile
#%{__make} %{?_smp_mflags}

#cd websocket_service
#qmake-qt4 -makefile
#%{__make} %{?_smp_mflags}

%install
%{__rm} -rf %{buildroot}
mkdir -p %{buildroot}/opt/vclient
mkdir -p %{buildroot}/opt/vclient/resource/image
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/xfce4/backdrops
mkdir -p %{buildroot}/etc/xdg/autostart

cp vclientnotautostart.sh %{buildroot}/opt/vclient
cp qt_zh_CN.qm %{buildroot}/opt/vclient
cp vclient_zh.qm %{buildroot}/opt/vclient
cp update_zh.qm %{buildroot}/opt/vclient
cp vclientstartconfig_zh.qm %{buildroot}/opt/vclient
cp about_vCl_logo.png %{buildroot}/opt/vclient/resource/image
cp background.png  %{buildroot}/usr/share/xfce4/backdrops
cp vClient %{buildroot}/opt/vclient
cp vclientstart_config %{buildroot}/opt/vclient
cp start.sh %{buildroot}/opt/vclient
cp getmac.sh %{buildroot}/opt/vclient
cp vclient.sh %{buildroot}/opt/vclient
cp vclientstart_config.sh %{buildroot}/opt/vclient
cp websocket_service %{buildroot}/opt/vclient
cp websocket_service_zh.qm %{buildroot}/opt/vclient
cp update_module %{buildroot}/opt/vclient
cp update.sh %{buildroot}/opt/vclient
cp vclient_version %{buildroot}/opt/vclient
cp vclient.desktop %{buildroot}/usr/share/applications
#cp vclientstart.desktop %{buildroot}/etc/xdg/autostart
cp fronview-update.desktop %{buildroot}/usr/share/applications
cp vclientstart_config.desktop %{buildroot}/usr/share/applications
%clean
#%{__rm} -rf %{buildroot}

%preun

%files
%defattr(-, root, root, 0755)

/opt/vclient/vClient
/opt/vclient/websocket_service
/opt/vclient/update_module
/opt/vclient/vclientstart_config
/opt/vclient/resource/image/about_vCl_logo.png
/opt/vclient/start.sh
/opt/vclient/getmac.sh
/opt/vclient/vclient.sh
/opt/vclient/update.sh
/opt/vclient/qt_zh_CN.qm
/opt/vclient/vclient_zh.qm
/opt/vclient/websocket_service_zh.qm
/opt/vclient/update_zh.qm
/opt/vclient/vclientstartconfig_zh.qm
/opt/vclient/vclientnotautostart.sh
/opt/vclient/vclient_version
/opt/vclient/vclientstart_config.sh
/usr/share/applications/vclient.desktop
/usr/share/applications/vclientstart_config.desktop
/usr/share/applications/fronview-update.desktop
/usr/share/xfce4/backdrops/background.png
#/etc/xdg/autostart/vclientstart.desktop

%changelog
* Thu Jun 20 2013 Guangyi Chen <chenguangyi@fronware.com> - 2.9.2.29
- Add some files to copy to installed directory. (based on redhat6.2)
