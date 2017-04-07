#%define dy_version %(python -c "from datetime import datetime;print(datetime.utcnow().strftime('%Y%m%d_%H%M%S'));")

%define dy_version %(date +%Y%m%d_%H%M%%S)

Summary:The DLF SDK example shows how to use of the DMD
Name:	dlf_setup
Version:1.9
Release:%{dy_version}
License:GPL
Group:	Development/Tools
Url:	http://www.wintechdigital.com
Vendor:Wintech Digital System Technology Corp.
AutoReqProv: no
Prefix:%{_prefix}/local
Requires(rpmlib): rpmlib(CompressedFileNames) <= 3.0.4-1 rpmlib(FileDigests) <= 4.6.0-1 rpmlib(PayloadFilesHavePrefix) <= 4.0-1
Requires:OpenEXR-devel libtiff-devel jasper-devel ilmbase-devel freetype-devel jbigkit-devel xz-devel spice-glib-devel mesa-libGL-devel mesa-libGLU-devel python-devel libusbx-devel libXrender qt-devel fontconfig-devel
Provides: dlf = 1.9-1 dlf(x86-64) = 1.9-1 libcbs.so()(64bit) libhal.so()(64bit) libstl2bmp.so()(64bit)
Conflicts:libpng12-devel
BuildRoot:%{_tmppath}/%{name}-%{version}-%{release}-root

%description
The DLF SDK example shows how to use of the DMD

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT/%{_prefix}/local/wintech
cp -a $RPM_BUILD_DIR/* $RPM_BUILD_ROOT/%{_prefix}/local/wintech

%pre
#RPM_INSTALL_PREFIX=/home/$USER						just for test
#prefix=/home/$USER							just for test
#echo This is pre for %{version}-%{release}: arg=$RPM_INSTALL_PREFIX	just for test
#echo This is pre for %{version}-%{release}: prefix arg=%{prefix}	just for test

%post
#echo This is post for %{version}-%{release}: arg=$RPM_INSTALL_PREFIX	just for test
if [ $1 = "1" ]
then
if [ ! -f /etc/ld.so.conf.d/dlf-x86_64.conf ]
then
touch /etc/ld.so.conf.d/dlf-x86_64.conf
echo $RPM_INSTALL_PREFIX/wintech/lib >> /etc/ld.so.conf.d/dlf-x86_64.conf
rm -rf /usr/lib64/libXrender.so
ln -s /usr/lib64/libXrender.so.1 /usr/lib64/libXrender.so
ldconfig


sed -i '/wdreg_wt/'d /etc/rc.d/rc.local
echo $RPM_INSTALL_PREFIX/wintech/bin/windriver/wdreg_wt $RPM_INSTALL_PREFIX/wintech/bin/windriver/wtdrvr6.ko auto >> /etc/rc.d/rc.local
chmod +x /etc/rc.d/rc.local
cd $RPM_INSTALL_PREFIX/wintech
chmod +x build.sh

cd $RPM_INSTALL_PREFIX/wintech/bin/windriver
chmod 777 ./wdreg_wt
./wdreg_wt wtdrvr6.ko auto


fi
fi

%preun
#echo This is preun for %{version}-%{release}: arg=$RPM_INSTALL_PREFIX	just for test
if [ $1 = "0" ]
then
rm -rf /etc/ld.so.conf.d/dlf-x86_64.conf
ldconfig
sed -i '/wdreg_wt/'d /etc/rc.d/rc.local
fi

%postun
#echo This is postun for %{version}-%{release}: arg=$RPM_INSTALL_PREFIX	just for test
if [ $1 = "0" ]
then
rm -rf $RPM_INSTALL_PREFIX/wintech
fi

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/*

%files
%defattr(-,root,root)
%attr(775,root,root) %{_prefix}/local/wintech/bin/dls_console
%attr(775,root,root) %{_prefix}/local/wintech/bin/dls_gui2
%{_prefix}/local/wintech


#%changelog
#* Mon Nov 03 2014 Wintech <wintech@dspchina.com> 1.0
#-build the program