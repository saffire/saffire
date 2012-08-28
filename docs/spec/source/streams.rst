=======
Streams
=======

Most, if not all, languages support some sort of I/O. This I/O allows you to communicate with the outside world, mostly
by user input, screen output or through files.

File I/O
========

Without any context, the default context will be io://. This context is used for reading and writing files, including
to the screen and from the user (through the stdin, stdout and stderr handles).


Http context
============
Reads (or writes) directory from a HTTP stream:

::

    $file = io.open("http://www.saffire-lang.org:80/index.html");
    $contents = $file.read(1024);
    $file.close();


