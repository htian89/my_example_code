Summary: The vClient is a application for managing virtual desktops' connections.
Name: vclient
Version: %{VERSION}
Release: %{RELEASE}%{?dist}
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

%build
./make.sh Release

%install
%{__rm} -rf %{buildroot}
mkdir -p %{buildroot}/opt/vclient
mkdir -p %{buildroot}/opt/vclient/resource/image
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/xfce4/backdrops
mkdir -p %{buildroot}/etc/xdg/autostart
mkdir -p %{buildroot}/usr/share/vclient/images

cp scripts/* %{buildroot}/opt/vclient
cp data/* %{buildroot}/opt/vclient
rm -f icons/vclientstart.desktop
cp icons/*.desktop %{buildroot}/usr/share/applications
cp icons/about_vCl_logo.png %{buildroot}/opt/vclient/resource/image
cp -r images/*  %{buildroot}/usr/share/vclient/images/
cp build.Release/vclient.Release/vClient %{buildroot}/opt/vclient
cp build.Release/vclientstart_config.Release/vclientstart_config %{buildroot}/opt/vclient
cp build.Release/websocket.Release/Binaries/WebsocketClient %{buildroot}/opt/vclient
cp build.Release/update_module.Release/update_module %{buildroot}/opt/vclient
%clean
#%{__rm} -rf %{buildroot}

%preun

%files
%defattr(-, root, root, 0755)
/opt/vclient/vClient
/opt/vclient/WebsocketClient
/opt/vclient/update_module
/opt/vclient/vclientstart_config
/opt/vclient/log.conf
/opt/vclient/resource/image/about_vCl_logo.png
/opt/vclient/start.sh
/opt/vclient/getmac.sh
/opt/vclient/vclient.sh
/opt/vclient/update.sh
/opt/vclient/qt_zh_CN.qm
/opt/vclient/vclient_zh.qm
/opt/vclient/update_zh.qm
/opt/vclient/vclientstartconfig_zh.qm
/opt/vclient/vclientnotautostart.sh
/opt/vclient/vclient_version
/opt/vclient/vclientstart_config.sh
/usr/share/applications/vclient.desktop
/usr/share/applications/vclientstart_config.desktop
/usr/share/applications/fronview-update.desktop
/usr/share/vclient
#/usr/share/xfce4/backdrops/background.png
#/etc/xdg/autostart/vclientstart.desktop

%changelog
* Thu Jun 20 2013 Guangyi Chen <chenguangyi@fronware.com> - 2.9.2.29
- Add some files to copy to installed directory. (based on redhat6.2)
