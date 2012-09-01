#########
Tutorials
#########

Hello world
-----------

::

    use io;

    io.print("hello world!");

In order to use methods like print, you must include the actual module that defines this functionality. The ``io`` class
defines input and output functionality, including methods like ``print`` and ``printf``.


Simple calculations
-------------------

::

    use io;

    $a = 1;
    $a += 1;

    io.print("The value of $a: ", $a , "\n");

Variables do not have to be declared, but instantiated. Using uninstantiated variables will result in an error.
Variables inside strings aren't automatically converted to their values. Using ``$a`` inside a string will result in a
**literal** ``$a``. If you want to use the **value** of ``$a``, you must add them separately. The print-method allows
multiple arguments so you can easily concatenate the string and values.


Classes and Objects
-------------------

::

    use io;

    class Foo() {
        public method ctor() {
        }

        public method foo() {
            return $a;
        }
    }

:Authors:
   Joshua Thijssen
   Caspar Dunant
