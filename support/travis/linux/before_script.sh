#!/bin/bash

# These are needed since the current libicu-dev pacakges on ubuntu does not have pkg-config information in them :(
wget http://saffire-lang.org/travis/libicu-dev_4.8.1.1-3_amd64.deb
wget http://saffire-lang.org/travis/libicu48_4.8.1.1-3_amd64.deb
sudo dpkg -i libicu48_4.8.1.1-3_amd64.deb
sudo dpkg -i libicu-dev_4.8.1.1-3_amd64.deb

# Link saffire modules
sudo mkdir -p /usr/share/saffire/modules
sudo ln -s `pwd`/sfl /usr/share/saffire/modules/sfl
