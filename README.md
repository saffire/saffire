Saffire
=======

[![Build Status](https://travis-ci.org/saffire/saffire.png)](https://travis-ci.org/saffire/saffire)

A new OO programming scripting language, based on primarily Python, PHP and Ruby. Its primary features:

- interpreted language
- dynamically, strong typed
- everything is an object
- full unicode support
- method + operator overloading

More information can be found at our website: http://www.saffire-lang.org and we're on IRC (freenode) as well, join us
in channel \#saffire.


Contributing:
-------------
We LOVE new contributors. Please join the \#saffire channel at IRC (freenode) for more information about contributing
to the project. There is no need have deep knowledge of C or knowing lots about compilers and stuff. There are lots of
other things that needs to be done and all the help on every level is welcome! Also, we love to meet new people, so
come and say hi to us.


Installing Saffire
------------------
There are two ways to install Saffire. The easy way, and the hard way :) The easy way consists of setting up a
virtualbox environment that will automatically install everything you need through the help of vagrant. The hard way,
well, you do everything yourself. Please read the information in the INSTALL.md file on how to install Saffire.




Generating a configuration file
-------------------------------
It's easy to get a default configuration.

    saffire config generate > saffire.ini

Copy this saffire.ini to /etc/saffire/saffire.ini. For most users, this configuration should be pretty ok. If you like
to modify this configuration, either manually edit the file, or use the 'saffire config' options:

    saffire config set global.timezone "Europe/Amsterdam"

This will automatically save a "timezone = Europe/Amsterdam" line under the "global" section of your ini file.
