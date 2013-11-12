#!/bin/sh

#
# This is a simple provisioning script that will create a proper saffire compilation setup
#

apt-get update

# Install compile tools
apt-get install -y git-core make automake gcc pkg-config bison flex colormake

# Install mandatory libraries
apt-get install -y libpcre3-dev libfcgi-dev libedit-dev libbz2-dev libcunit1-dev libxml2-dev

# Mandatory PHP scripts, needed for generating some files
apt-get install -y php5-cli php5-dev libyaml-dev php-pear
printf "\n" | pecl install yaml
echo "extension=yaml.so" >> /etc/php5/cli/php.ini

# Symlink SFL library
mkdir -p /usr/share/saffire/modules
ln -s /vagrant/sfl /usr/share/saffire/modules/sfl

# Create new MOTD
cat << 'EOF' > /etc/motd.tail
Vagrant Development Box. This box runs on ubuntu64 bit server edition. 

All necessary tools and libraries are installed to compile Saffire. Please run the following:

  $ cd /vagrant
  $ ./autogen.sh
  $ ./configure [--enable-debug]
  $ colormake
  $ sudo make install

This will compile and install a Saffire binary in /usr/local/bin/saffire. From this point you can use 
saffire by issuing:

  $ saffire help

Or start executing your first saffire file with:

  $ saffire ./hello.sf

Have fun!
The Saffire Group
EOF
