#!/bin/sh

head -n 10 /root/rpmbuild/SPECS/vclient.spec
echo "vclient_version"
echo "VERSION_FRONWARE(方物)   VERSION_DCN(神州数码)   VERSION_NOLOGO()   VERSION_HXXY(华夏星云) \
      VERSION_SUGON(曙光)    VERSION_DHC(东华)    VERSION_ONLY(昂利)    VERSION_INSPUR(浪潮) \
	  VERSION_VSAP(启明星辰)     VERSION_DCITS(神州信息) VERSION_THINVIEW(华夏星云) \
	  VERSION_LENOVO_VCLASS(联想云教室) VERSION_JSJQ(江苏军区)"
#set param for var;
echo "need input like --release=67 --minrel=1 --version=VERSION_FRONWARE"
Param=`getopt -o l:m:v: --long release:,minrel:,version: -n 'Warn:' -- "$@"`
if [ $? != 0 ];then
    echo "getopt parm error"
    exit -1
fi
eval set -- "$Param"

while true ; do
    case "$1" in
	-l|--release) rel="$2" ; shift 2;;
	-m|--minrel)  min="$2" ; shift 2;;
	-v|--version) ver="$2" ; shift 2;;
	--) shift ; break ;;
	*) echo "Internal error" ; exit -1 ;;
    esac
done
echo "$rel" 
echo "$ver"
echo "$min"
if [ ! "$rel" ];then
    echo "release is null, please input --release= arg"
    echo "arg: eg: 55、56..."
    exit -1
fi 
if [ ! "$min" ];then
    echo "min release is null, please input --minrel= arg"
    echo "arg: eg: 55、56..."
    exit -1
fi 
if [ ! "$ver" ];then
    echo "version is null, please input --version=arg"
    echo "arg: eg: VERSION_DCN、VERSION_FRONWARE..."
    exit -1;
fi
#this init application trunk, trunk-make dir;
MyPath=`pwd`
WebsocketPath=${MyPath}"/websocket_service"
LibwebsocketPath=${WebsocketPath}"/libwebsocketpp.a"
UpdatePath=${MyPath}"/update_module"
UpdatePath2000=${MyPath}"/update_module-2000"
VclientConfigPath=${MyPath}"/vclientstart_config"
SrcImagePath=${MyPath}"/ui/resource/image"
TagsPath="/root/tags"
MakeFile=${MyPath}"/make.sh"
ConfigFile=${MyPath}"/config.h"
SpecFile=${MyPath}"/vclient.spec"
ImagePath=${MyPath}"/ui/resource/image"
spec_version="Version: 2.9.2."${rel}
spec_release="Release: "${min}"%{?dist}"
VersionFile=${MyPath}"/vclient_version"
vclient_version="2.9.2Build00"${rel}
MakePath="/root/trunk2-make-"${ver}
MakeWebsocketPath=${MakePath}"/websocket_service"
MakeUpdatePath=${MakePath}"/update_module"
MakeVclientConfigPath=${MakePath}"/vclientstart_config"
#modify the define release eg: Release and version eg: Version;
Release_temp="    #define     VER4                "
Version_temp="#define    "
Release=${Release_temp}"$rel"
Minrel="$min"
Version=${Version_temp}"$ver"
if [ ! -x "$MyPath" ];then
    echo ${MyPath}" is not exists"
    exit -1
fi
#check the source files;
cd "$MyPath"
echo $PWD
if [ $? != 0 ];then
    echo ${MyPath}" is not exists"
    exit -1;
fi
if [ ! -f "$MakeFile" ];then
    echo ${MakeFile}" is not exists"
    exit -1
fi
if [ ! -f "$ConfigFile" ];then
    echo ${ConfigFile}" is not exists"
    exit -1
fi
#this is change
#this is cover config.h the 12h and 14h line;
#this will change the edition of about ui; 
#VERSION_JSJQ need to modify the edition of cmessagebox.cpp 
sed -i "14c $Version" "$ConfigFile"
if [ $? -eq 0 ]; then
    echo "set version success"
else
    echo "set version failed"
    exit -1
fi
#this change the VERSION_XX
sed -i "12c $Release" "$ConfigFile"
if [ $? -eq 0 ]; then
    echo "set release success"
else
echo "set release failed"
    exit -1
fi
#change the image before compile
if [ "$ver" = "VERSION_JSJQ" ];then
	cp ${MyPath}"/about_vCl_logo_jsjq.png" ${SrcImagePath}"/about_vCl_logo.png"
	cp ${MyPath}"/about_vCl_logo_jsjq.png" ${UpdatePath}"/update_moudle.png"
	cp ${MyPath}"/about_vCl_logo_jsjq.png" ${WebsocketPath}"/resource/about_vCl_logo.png"
	cp ${MyPath}"/about_vCl_logo_jsjq.png" ${VclientConfigPath}"/resource/image/about_vCl_logo.png"
else
	cp ${MyPath}"/about_vCl_logo_std.png" ${SrcImagePath}"/about_vCl_logo.png"
	cp ${MyPath}"/about_vCl_logo_std.png" ${UpdatePath}"/update_moudle.png"
	cp ${MyPath}"/about_vCl_logo_std.png" ${WebsocketPath}"/resource/about_vCl_logo.png"
	cp ${MyPath}"/about_vCl_logo_std.png" ${VclientConfigPath}"/resource/image/about_vCl_logo.png"
fi
#recover vclietn_version
sed -i "1c $vclient_version" ${VersionFile}
if [ $? != 0 ];then
	echo ${VersionFile}"change failed"
	exit -1;
fi
#this is vclient.spec change
sed -i "3c $spec_version" ${SpecFile}
if [ $? != 0 ];then
    echo "set vclient.spec version failed"
    exit -1
fi
sed -i "4c $spec_release" ${SpecFile}
if [ $? != 0 ];then
    echo "set vclient.spec release failed"
    exit -1
fi
#create the trunk-make dir and clear this dir;
if [ ! -x "$MakePath" ];then
   mkdir -p "$MakePath"
   echo ${MakePath}" is mkdir -p"
fi
cd "$MakePath"
if [ $? != 0 ];then
    echo ${MakePath}" is not exists"
    exit -1
else
    rm -rf *
fi
echo $PWD
#this environment qmake-qt4 need arg: QMAKE_CXXFLAGS 
qmake-qt4 QMAKE_CXXFLAGS=-fpermissive "$MyPath"
make -j3 
if [ $? != 0 ];then
    echo "make vclient failes"
    exit -1;
fi
#copy language file to test ui
cp ${MyPath}"/ui/vclient_zh.qm" "$MakePath"
if [ $? != 0 ]; then
    echo "cp vclient_zh.qm failed"
    exit -1
fi
#test the .exe 
#./vClient 
echo "test vclient finished"

#make the websocket_service
if [ ! -x "$MakeWebsocketPath" ];then
	mkdir -p "$MakeWebsocketPath"
	echo ${MakeWebsocketPath}" is mkdir -p"
fi
cd "$MakeWebsocketPath"
if [ $? != 0 ];then
	echo ${MakeWebsocketPath}" is not exist"
	exit -1;
fi
cd "$WebsocketPath"
qmake-qt4 QMAKE_CXXFLAGS=-fpermissive
make -j3
cp websocket_service "$MakeWebsocketPath"
if [ $? != 0 ];then
    cd -
	echo "make websocket error"
	exit -1;
fi
cd -
#make the update_module
if [ ! -x "$MakeUpdatePath" ];then
	mkdir -p "$MakeUpdatePath"
	echo ${MakeUpdatePath}" is mkdir -p"
fi
cd "$MakeUpdatePath"
if [ $? != 0 ]; then
	echo ${MakeUpdatePath}" is not exist"
	exit -1
fi
qmake-qt4 QMAKE_CXXFLAGS=-fpermissive "$UpdatePath"
make -j3
cp ${UpdatePath}"/update_zh.qm" "$MakeUpdatePath"
if [ $? != 0 ];then
	echo "cp update_zh.qm failed"
	exit -1
fi
#./update_module
echo "update_module test finished"
#make the vclient_config
if [ ! -x "$MakeVclientConfigPath" ];then
	mkdir -p "$MakeVclientConfigPath"
	echo ${MakeVclientConfigPath}" is mkdir -p"
fi
cd "$MakeVclientConfigPath"
if [ $? != 0 ];then
	echo ${MakeVclientConfigPath}" is note exist"
	exit -1
fi
qmake-qt4 QMAKE_CXXFLAGS=-fpermissive "$VclientConfigPath"
make -j3
cp ${VclientConfigPath}"/vclientstartconfig_zh.qm" "$MakeVclientConfigPath"
if [ $? != 0 ];then
	echo "cp vclientstartconfig_zh.qm failed"
	exit -1
fi
#./vclientstart_config
#test vclientstart_config finished;

#need to copy the old rpm dir to this version and cover the vClient.exe and vclient_zh.qm  
Prerel=`expr $rel - 1`
echo "PreRelease=$Prerel"
RpmbuildPath="/root/rpmbuild"
if [ ! -x "$RpmbuildPath" ]; then
    echo ${RpmbuildPath}" is not exists"
    exit -1
fi
#prepare the rpmbuild environment 
PreBuildVclient="vclient-2.9.2."${Prerel}
CurBuildVclient="vclient-2.9.2."${rel}
vclient_tags="v2.9.2Build00"${rel}"."${min}"-"${ver}
SOURCESPath=${RpmbuildPath}"/SOURCES"
SPECSPath=${RpmbuildPath}"/SPECS"
RPMSPath=${RpmbuildPath}"/RPMS/i686"
if [ ! -x "$SOURCESPath" ];then
    echo ${SOURCESPath}" is not exists"
    exit -1
fi
cd "$SOURCESPath"
if [ $? != 0 ]; then
    echo ${SOURCESPath}" is not exists"
    exit -1
fi
#if [ -x "$CurBuildVclient" ];then
#    echo ${CurBuildVclient}" is exists"
#    rm -rf "$CurBuildVclient"
#fi
#if [ ! -x "$PreBuildVclient" ];then
#    echo ${PreBuildVclient}" is not exists"
#    exit -1
#else
#    cp -r "$PreBuildVclient" "$CurBuildVclient"
#fi
#if [ ! -x "$CurBuildVclient" ];then
#    echo ${CurBuildVclient}" is not exists"
#    exit -1
#fi

#create the build dir

if [ -x "$CurBuildVclient" ];then
	echo ${CurBuildVclient}" is exists"  
	rm -rf $CurBuildVclient
	mkdir $CurBuildVclient
else
	mkdir ${CurBuildVclient}
fi
if [ $? != 0 ];then
	echo  ${CurBuildVclient}" is not exist"
	exit -1
fi


#this way to copy the source files to rpmbuilg dir
#copy exe
cp ${MakePath}"/vClient" ${CurBuildVclient}
cp ${MakeWebsocketPath}"/websocket_service" ${CurBuildVclient}
cp ${MakeUpdatePath}"/update_module" ${CurBuildVclient}
cp ${MakeVclientConfigPath}"/vclientstart_config" ${CurBuildVclient}
#copyt language file
cp ${MyPath}"/ui/vclient_zh.qm" ${CurBuildVclient}
cp ${MyPath}"/ui/qt_zh_CN.qm" ${CurBuildVclient}
cp ${WebsocketPath}"/websocket_service_zh.qm" ${CurBuildVclient}
cp ${UpdatePath}"/update_zh.qm" ${CurBuildVclient}
cp ${VclientConfigPath}"/vclientstartconfig_zh.qm" ${CurBuildVclient}
#copy .sh files
cp ${MyPath}"/fronview-update.desktop" ${CurBuildVclient}
cp ${MyPath}"/getmac.sh" ${CurBuildVclient}
cp ${MyPath}"/start.sh" ${CurBuildVclient}
cp ${MyPath}"/update.sh" ${CurBuildVclient}
cp ${MyPath}"/vclient.desktop" ${CurBuildVclient}
cp ${MyPath}"/vclientnotautostart.sh" ${CurBuildVclient}
cp ${MyPath}"/vclient.sh" ${CurBuildVclient}
cp ${MyPath}"/vclientstart_config.desktop" ${CurBuildVclient}
cp ${MyPath}"/vclientstart_config.sh" ${CurBuildVclient}
cp ${MyPath}"/vclientstart.desktop" ${CurBuildVclient}
cp ${MyPath}"/vclient_version" ${CurBuildVclient}
cp ${MyPath}"/vclient.spec" ${SPECSPath}

#some oem .rpm need recover the background to its;

#copy the image files
if [ "$ver" = "VERSION_JSJQ" ];then
	cp ${MyPath}"/about_vCl_logo_jsjq.png" ${CurBuildVclient}"/about_vCl_logo.png"
else
	cp ${MyPath}"/about_vCl_logo_std.png" ${CurBuildVclient}"/about_vCl_logo.png"
fi
if [ "$ver" = "VERSION_FRONWARE" ];then
	cp ${ImagePath}"/background_FRONWARE.png" ${CurBuildVclient}"/background.png"
elif [ "$ver" = "VERSION_FRONWARE_VCLASS" ];then
	cp ${ImagePath}"/background_FRONWARE_vclass.png" ${CurBuildVclient}"/background.png"
elif [ "$ver" = "VERSION_HXXY" ];then
	cp ${ImagePath}"/background_HXXY.png" ${CurBuildVclient}"/background.png"
elif [ "$ver" = "VERSION_DCN" ];then
	cp ${ImagePath}"/background_DCN.png" ${CurBuildVclient}"/background.png"
elif [ "$ver" = "VERSION_DCN_VCLASS" ];then
	cp ${ImagePath}"/background_DCN_vclass.png" ${CurBuildVclient}"/background.png"
elif [ "$ver" = "VERSION_LENOVO" ];then
	cp ${ImagePath}"/background_LENOVO.png" ${CurBuildVclient}"/background.png"
elif [ "$ver" = "VERSION_LENOVO_VCLASS" ];then
	cp ${ImagePath}"/lenovo_vClass/vclass_background_1440.png" ${CurBuildVclient}"/background.png"
	cp ${ImagePath}"/lenovo_vClass/title.png" ${CurBuildVclient}"/title.png"
	cp ${ImagePath}"/lenovo_vClass/title_center.png" ${CurBuildVclient}"/title_center.png"
	cp ${ImagePath}"/lenovo_vClass/login_vClass.png" ${CurBuildVclient}"/"
	cp ${ImagePath}"/lenovo_vClass/vClass_admin_welcome.png" ${CurBuildVclient}"/"
	cp ${ImagePath}"/lenovo_vClass/vClass_center_login.png" ${CurBuildVclient}"/"
	cp ${ImagePath}"/lenovo_vClass/vClass_start.png" ${CurBuildVclient}"/"
elif [ "$ver" = "VERSION_SUGON_2000" ];then
	cp ${ImagePath}"/background_SUGON_2000.png" ${CurBuildVclient}"/background.png"
elif [ "$ver" = "VERSION_JSJQ" ];then
	cp ${ImagePath}"/background_JSJQ.png" ${CurBuildVclient}"/background.png"
else
	cp ${ImagePath}"/background_NOLOGO.png" ${CurBuildVclient}"/background.png"
fi
#compress the SOURCES dir;
tar -zcvf ${CurBuildVclient}".tar.gz" "$CurBuildVclient"
#clear the *.rpm int RPMSPath
if [ ! -x "$RPMSPath" ];then
    echo ${RPMSPath}" is not exists"
    exit -1
fi
cd "$RPMSPath"
if [ $? != 0 ];then
    echo ${RPMSPath}" is not exists"
    exit -1
fi
rm -rf *
#make .rpm
if [ ! -x "$SPECSPath" ];then
    echo ${SPECSPath}" is not exists"
    exit -1
else
    cd "$SPECSPath"
fi
#modify the vclient.spec to this release;
if [ ! -f "vclient.spec" ];then
    echo ${SPECSPath}"/vclient.spec is not exits"
    exit -1
fi
rpmbuild -bb "vclient.spec"
if [ ! -x ${MyPath}"/RPM" ];then
    echo ${MyPath}"/RPM is not exists"
    exit -1
fi
cd ${MyPath}"/RPM"
if [ $? != 0 ];then
    echo ${MyPath}"/RPM failed to cd"
    exit -1
fi 
exit 0
#clear trunk/RPM/*.rpm and svn ci the modify 
prestr=`find . -name "*.rpm"`
if [ "$prestr" ];then
    find . -name "*.rpm" | xargs svn delete 
fi
find ${RPMSPath} -name "*.rpm" | xargs -i cp {} ${MyPath}"/RPM"
if [ $? != 0 ]; then 
   echo "failed to copy vclient.rpm to RPMS"
   exit -1
fi
curstr=`find . -name "*.rpm"`
if [ "$curstr" ];then
    find . -name "*.rpm" | xargs svn add  
fi
cd "$MyPath"
if [ $? != 0 ]; then
   exit -1;
fi
svn up
svn st 
svn commit -m ""
cd 
svn copy "$MyPath" ${TagsPath}"/"${vclient_tags}
if [ ! -x "$TagsPath" ];then
    echo "svn copy trunk2 failed"
    exit -1
else
    cd "$TagsPath"
fi
svn commit ${vclient_tags} -m ""
svn up ${vclient_tags}
find ${vclient_tags}"/RPM" -name "*.rpm" | xargs svn delete 
find ${MyPath}"/RPM" -name "*.rpm" | xargs -i cp {} ${TagsPath}"/"${vclient_tags}"/RPM"
find ${vclient_tags}"/RPM" -name "*.rpm" | xargs svn add
svn commit ${vclient_tags} -m ""
svn up ${vclient_tags}
