#############
Using Saffire
#############

Command line usage
------------------
Saffire can be run easily from the commandline.

Usage: saffire [options] [script [args]]

**-h or --help**
Return usage information

**-v or --version**
Show the version information

**-c or --cli**
Start an interactive command line version of Saffire.

**-l or --lint**
Execute a lint-check on the source code.

**--dot**
Generate a AST tree in DOT format.

Interactive mode
----------------
When starting Saffire with the **-c** or **--cli**, it will start an interactive mode. In this mode you can insert your
code line by line and will execute it straight away.

FastCGI usage
-------------

:Authors:
   Joshua Thijssen
   Caspar Dunant
