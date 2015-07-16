#!/bin/bash

#
# This is a simple provisioning script that will create a proper Saffire compilation environment
#

apt-get update

# Install compile tools
apt-get install -y git-core make automake gcc pkg-config bison flex colormake
apt-get install -y cmake build-essential

# Install mandatory libraries
apt-get install -y libpcre3-dev libfcgi-dev libedit-dev libbz2-dev libcunit1-dev libxml2-dev libicu-dev
apt-get autoremove

# Needed as cunit pkgconfig does not supply a version line itself
grep "Version" /usr/lib/pkgconfig/cunit.pc
if [[ $? == 1 ]] ; then
    echo "Version: 2.1.0" >> /usr/lib/pkgconfig/cunit.pc
fi

# Mandatory PHP scripts, needed for running unittests
apt-get install -y php5-cli


# Install nginx
apt-get install -y nginx
ln -s /vagrant/support/nginx/config/saffire /etc/nginx/sites-enabled/saffire
/etc/init.d/nginx reload

# Symlink SFL library
mkdir -p /usr/share/saffire/modules
ln -s /vagrant/sfl /usr/share/saffire/modules/sfl


# Create new MOTD partial
cat << 'EOST' > /etc/update-motd.d/99-saffire-welcome
#!/bin/bash

echo -e "\033[1;31m                    __  __ _"
echo -e "\033[1;32m         ___  __ _ / _|/ _(_)_ __ ___"
echo -e "\033[1;33m        / __|/ _\` | |_| |_| | '__/ _ \\"
echo -e "\033[1;34m        \\__ \\ (_| |  _|  _| | | |  __/"
echo -e "\033[1;35m        |___/\\__,_|_| |_| |_|_|  \\___|"

echo -e "\033[0m"

echo -e "
 All necessary tools and libraries are installed to compile Saffire. Please run the following:
\033[1;37m
   $ cd /vagrant
   $ sh build.sh
\033[0m

 This will compile and install a Saffire binary in \033[1;37m/usr/local/bin/saffire\033[0m. From this point you
 can use saffire by issuing:
\033[1;37m
   $ saffire help
\033[0m

 Or start executing your first saffire file with:
\033[1;37m
   $ saffire hello.sf
\033[0m

 Have fun!
 The \033[1;31mS\033[1;32ma\033[1;33mf\033[1;34mf\033[1;35mi\033[1;36mr\033[1;37me\033[0m Group
";
EOST

chmod 755 /etc/update-motd.d/99-saffire-welcome

update-motd
