#!/bin/sh

#
# This is a simple provisioning script that will create a propper
#
#

apt-get update

# Install compile tools
apt-get install -y git-core make automake gcc pkg-config bison flex php5-cli

# Install mandatory libraries
apt-get install -y libpcre3-dev libfcgi-dev libedit-dev libbz2-dev libcunit1-dev libxml2-dev

# Symlink SFL library
mkdir -p /usr/share/saffire/modules
ln -s /vagrant/sfl /usr/share/saffire/sfl

# Create new MOTD
cat << 'EOF' > /etc/motd.tail
Vagrant Development Box. This box runs on ubuntu64 bit server edition. 

All neccessary tools and libraries are installed to compile Saffire. Please run the following:

  $ cd /vagrant
  $ ./autogen.sh
  $ ./configure [--enable-debug]
  $ make
  $ sudo make install

This will compile and install a Saffire binary in /usr/local/bin/saffire. From this point you can use 
saffire by issuing:

  $ saffire help

Or start executing your first saffire file with:

  $ saffire ./hello.sf

Have fun!
The Saffire Group
EOF
