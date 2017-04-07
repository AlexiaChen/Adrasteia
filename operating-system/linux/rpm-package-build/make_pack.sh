#rm -rf bmk

mkdir bmk 

cd bmk

#check and creat the working directory for packaging
if [ -d  ~/rpmbuild ]
then
 if [ ! -d  ~/rpmbuild/BUILD ]
 then
	mkdir -p ~/rpmbuild/BUILD
 fi
 if [ ! -d  ~/rpmbuild/RPMS ]     
 then
        mkdir -p ~/rpmbuild/RPMS
 fi
 if [ ! -d  ~/rpmbuild/SOURCES ]     
 then
        mkdir -p ~/rpmbuild/SOURCES
 fi
 if [ ! -d  ~/rpmbuild/SPECS ]    
 then
        mkdir -p ~/rpmbuild/SPECS
 fi
 if [ ! -d  ~/rpmbuild/SRPMS ]     
 then
        mkdir -p ~/rpmbuild/SRPMS
 fi
else
     mkdir -p ~/rpmbuild/{BUILD,RPMS,S{OURCE,PEC,RPM}S}
fi


#remove the old files for packaging
rm -rf ~/rpmbuild/BUILD/*
rm -rf ~/rpmbuild/SPECS/*

cmake  -DTEST=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=~/rpmbuild/BUILD ../
make install


#packaging
cp -a ../install.spec ~/rpmbuild/SPECS
rpmbuild -bb ~/rpmbuild/SPECS/install.spec

mv ~/rpmbuild/RPMS/x86_64/* ../


