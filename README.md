Saffire
=======
A new OO programming scripting language, based on primarily Python, PHP and Ruby. It's primary features:

- interpreted language
- dynamically, strong typed
- everything is an object
- full unicode support
- method + operator overloading

More information can be found at our website: http://www.saffire-lang.org and we're on IRC (freenode) as well, join us
in channel \#saffire.


Contributing:
-------------
We LOVE new contributers. Please join the \#saffire channel at IRC (freenode) for more information about contributing
to the project. There is no need have deep knowledge of C or knowing lots about compilers and stuff. There are lots of
other things that needs to be done, and all the help on every level is welcome! Also, we love to meet new people, so
come and say hi to us.


Installation on vagrant:
------------------------
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
Make sure you have the following tools installed:
- automake
- make
- flex
- bison
- gcc

Plus, you will need to following libraries and header files:
- libedit
- libfcgi
- libbz2
- libaugeas
- libpcre3

To install these packages on Ubuntu or Debian, please use the following commands:
  sudo apt-get install automake make flex bison gcc
  sudo apt-get install libedit-dev libfcgi-dev libbz2-dev libaugeas-dev libpcre3-dev

On RedHat/CentOS systems, the library development packages are probably ends with -devel.

After installation of the packages, clone the github repository (either your own, or from the Saffire github repo):
  git clone https://github.com/saffire/saffire.git

  cd saffire
  sh autogen.sh
  ./configure [--enable-debug] [--enable-parsedebug]
  make
  sudo make install

If everything goes according to plan, you will have a /usr/local/bin/saffire binary up and running. You can test this
with the following command:

  saffire

This should display an initial help file.


Generating a configuration file
-------------------------------
It's easy to get a default configuration.

  saffire config generate > saffire.ini

Copy this saffire.ini to /etc/saffire/saffire.ini. For most users, this configuration should be pretty ok. If you like
to modify this configuration, either manually edit the file, or use the 'saffire config' options:

  saffire config set global.timezone "Europe/Amsterdam"

This will automatically save a "timezone = Europe/Amsterdam" line under the "global" section of your ini file.