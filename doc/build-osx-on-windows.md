Mac OS X Build Instructions and Notes on Windows WSL
====================================

Notes
-----

* Quick how to compile OS X on Windows WSL


First, install the general dependencies:

    sudo apt update
    sudo apt upgrade
    sudo apt-get install -y \
        git ruby apt-cacher-ng qemu-utils debootstrap lxc python-cheetah parted kpartx bridge-utils \
        make curl firewalld ca-certificates curl g++ git-core pkg-config autoconf librsvg2-bin libtiff-tools \
        libtool automake faketime bsdmainutils cmake imagemagick libcap-dev libz-dev libbz2-dev python python-dev \
        python-setuptools fonts-tuffy


Acquire the source in the usual way:

    git clone https://github.com/thebitradio/Bitradio.git
    cd Bitradio


After that install more dependencies:

    cd depends
    wget https://github.com/MZaf/MacOSX10.11.sdk/raw/master/MacOSX10.11.sdk.tar.gz
    mkdir SDKs
    tar -xzvf MacOSX10.11.sdk.tar.gz
    mv MacOSX10.11.sdk SDKs/MacOSX10.11.sdk
    make HOST="x86_64-apple-darwin11" DARWIN_SDK_PATH=$PWD/depends/SDKs/MacOSX10.11.sdk -j<threads>

This will take a lot of time. Wait and do not interrupt the compile.

    cd ..
    ./autogen.sh
    ./configure --prefix=`pwd`/depends/x86_64-apple-darwin11 --disable-tests
    make deploy -j<threads>
    mv BitRadio-Core.dmg /mnt/c/<path>/<you>/<want>/BitRadio-Core.dmg
