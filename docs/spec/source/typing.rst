======
Typing
======

Dynamically typed
=================
Saffire is dynamically typed language instead of static typed. This means that the type of a variable is known at
runtime, but often not at compile time. It means that it is not necessary to define your variables before using like in
C, Java or Pascal.

Strong typed
============
Saffire is strong typed instead of being weak typed. This means that Saffire will never cast variables or values
automatically.

::

    $a = "foo";
    $b = 1;
    $c = $a + $b;

In this case, Saffire would throw an exception. But you can cast explicitly:

::

    $a = "foo";
    $b = 1;
    $c = $a + $b.string();  // $c == "foo1"


coercion
========



