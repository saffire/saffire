Read the following twice. Three times if it still confuses you:
---------------------------------------------------------------
Saffire is nowhere near production ready. You can compile saffire, run some example code but don't expect to be
programming websites or applications in it. We are still a long way from a v0.1 release. Compile Saffire if you
like to see how the


Installation through vagrant:
-----------------------------
If you don't want to clutter up your local, or just want to test out Saffire, it's possible to use vagrant for
development. Make sure you have vagrant and virtualbox installed on your local system. After that, just clone the
Saffire repository onto your local system, go inside the directory and type:

    vagrant up

It should download a ubuntu 64bit basebox and automatically install all the packages you need in order to compile
Saffire. Once the box has started, issue the following:

    vagrant ssh
    cd /vagrant
    sh autogen.sh
    ./configure [--enable-debug] [--enable-parsedebug]
    make
    sudo make install

This will compile and install Saffire in your vagrant box, where you can experiment all you like.


Installation on your local system:
----------------------------------
This will only work on Linux systems. We do not support Windows, OSX or any other unix-platform currently. Plans in
doing so will be there, we just haven't got the time to support them all right now.

Make sure you have the following tools installed:
- automake
- pkg-config
- make
- flex
- bison
- gcc

Plus, you will need to following libraries and header files:
- libedit
- libfcgi
- libbz2
- libpcre3
- libcunit11

To install these packages on Ubuntu or Debian, please use the following commands:

    sudo apt-get install automake make flex bison gcc pkg-config
    sudo apt-get install libedit-dev libfcgi-dev libbz2-dev libpcre3-dev

On RedHat/CentOS systems, the library development packages are probably ends with -devel.

After installation of the packages, clone the github repository (either your own, or from the Saffire github repo):
    
    git clone https://github.com/saffire/saffire.git

After this, you can compile and install Saffire:

    cd saffire
    sh autogen.sh
    ./configure [--enable-debug] [--enable-parsedebug]
    make
    sudo make install

One more thing: in the current development settings, we need to have Saffire modules located in a fixed path:

    cd saffire
    mkdir -p /usr/share/saffire/modules
    ln -s sfl /usr/share/saffire/modules/sfl

If everything goes according to plan, you will have a /usr/local/bin/saffire binary up and running. You can test this
with the following command:

    saffire

This should display an initial help file and have saffire configured.
