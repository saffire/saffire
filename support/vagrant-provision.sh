#!/bin/sh

#
# This is a simple provisioning script that will create a proper saffire compilation setup
#

apt-get update

# Install compile tools
apt-get install -y git-core make automake gcc pkg-config bison flex colormake
apt-get install -y cmake build-essential

# Install mandatory libraries
apt-get install -y libpcre3-dev libfcgi-dev libedit-dev libbz2-dev libcunit1-dev libxml2-dev libicu-dev

# Mandatory PHP scripts, needed for running unittests
apt-get install -y php5-cli

# Symlink SFL library
mkdir -p /usr/share/saffire/modules
ln -s /vagrant/sfl /usr/share/saffire/modules/sfl

# Create new MOTD
cat << 'EOST' > /etc/update-motd.d/99-saffire-welcome
#!/bin/sh

echo "\033[1;33m"

cat << "EOT"
                    __  __ _
         ___  __ _ / _|/ _(_)_ __ ___
        / __|/ _` | |_| |_| | '__/ _ \
        \__ \ (_| |  _|  _| | | |  __/
        |___/\__,_|_| |_| |_|_|  \___|
EOT

echo "\033[0m"

echo "
 All necessary tools and libraries are installed to compile Saffire. Please run the following:
\033[1;37m
   $ cd /vagrant
   $ sh build.sh
\033[0m

 This will compile and install a Saffire binary in /usr/local/bin/saffire. From this point you
 can use saffire by issuing:
\033[1;37m
   $ saffire help
\033[0m

 Or start executing your first saffire file with:
\033[1;37m
   $ saffire ./hello.sf
\033[0m

 Have fun!
 The Saffire Group
";
EOST

chmod 755 /etc/update-motd.d/99-saffire-welcome
