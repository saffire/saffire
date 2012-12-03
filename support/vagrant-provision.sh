#!/bin/sh

# Install compile tools
apt-get install -y git-core make automake gcc pkg-config bison flex php5-cli

# Install mandatory libraries
apt-get install -y libpcre3-dev libfcgi-dev libaugeas-dev libedit-dev libbz2-dev

# Create new MOTD
cat << 'EOF' > /etc/motd.tail
Vagrant Development Box. This box runs on ubuntu64 bit server edition. 

All neccessary tools and libraries are installed to compile Saffire. Please run the following:

  $ cd /vagrant
  $ ./autogen.sh
  $ ./configure [--enable-debug]
  $ make
  $ sudo make install

This will compile and install a Saffire binary. You can use this string away by issueing:

  $ saffire help

Have fun!
The Saffire Group
EOF
