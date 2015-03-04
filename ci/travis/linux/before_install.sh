#!/bin/bash

sudo apt-get update -qq
sudo apt-get install -qq libpcre3-dev libfcgi-dev libedit-dev libbz2-dev libxml2-dev
sudo apt-get install -qq php5-cli php-pear
sudo apt-get install -qq libcunit1-dev
printf "\n" | sudo pecl install yaml
sudo sh -c 'echo "extension = yaml.so" >> /etc/php5/cli/php.ini'
