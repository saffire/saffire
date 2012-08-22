#######
Objects
#######

Everything in Saffire is an object. Even constant strings like "foo" are represented as `string`, which makes it possible to do:

::

	"foo".length()   // returns 3

Objects are all based from a special base class (object) that has some basic functionality for each class.


Classes vs Objects
==================
There is a subtle difference between objects and classes. A class is a "contract" which tells Saffire how objects should look like.
There can be only one class for each name. Objects, are instantiated classes. They are mostly mapped to variables so you can
actually use these classes.


Defining objects
================

::

	class Foo {
	}

Even though this class does not extends anything, it still extends our base calls object. This means it could have been written like:

::

	class Foo extends object {
	}

For more info about extending classes, see `extending classes`_.


Instantiating
=============
The processes of creating objects from classes is called instantiating. To instantiate a class into an object use the following syntax:

::

    class Foo {
    }

    $a = Foo();

This will create an object $a, which has been instantiated from class Foo();

.. note::
	It is not possible to instantiate the `object` class. This will result in an exception.



Extending classes:
==================
A class can extend another class. Note that saffire only support single inheritance model, not multiple inheritance.

::

	class Foo {
	}

	class Bar extends Foo {
	}


Class body
==========
There are three things that can be defined inside a class body:

- `constants`_
- `properties`_
- `methods`_



Constants
---------
Constants are like readonly properties. They are usually written in uppercase.

::
	
	class Foo {
		const Foo = "BAR";
	}


Properties
----------
Properties *must* implement a `Visiblity`_. There is no assumed visibilty.

::

	class Foo {
		public property $foo = "bar";
		protected property $foo = "bar";
		private  property $foo = "bar";
	}

Properties can either have a default assigned value, or none. In that case, it will automatically be filled with the Null object.


::

	class Foo {
		public property $foo = Null;
		public property $bar;

		// $bar and $foo are both Null
	}




Methods
-------
Just like properties, methods also must implement a 'Visibility'.

::

	class Foo {
		public method Bar() {
		}
	}


A method can have an optional argument list:

::

	class Foo {
		public method Bar($arg1, $arg2) {
		}
	}

	Foo().Bar("this", "works");	// $arg1 = "this", $arg2 = "works"


An argument can also have a default value, in case nothing has been specified by the caller.

::

	class Foo {
		public method Bar($arg1 = "var", $arg2 = "default") {
		}
	}

	Foo().Bar();                 // $arg1 = "var", $arg2 = "default"
	Foo().Bar("this");           // $arg1 = "this", $arg2 = "default"
	Foo().Bar("this", "works");  // $arg1 = "this", $arg2 = "works"


It's also possible to use type hinting to make sure the arguments are from a certain class or interface:

::

	class Foo {
		public method Bar(String $arg1, Foo $arg2) {
		}
	}

	class Bar extends Foo {
	}

	Foo().Bar("test", Foo());     // Works, arg1 is a string, Foo() is of class Foo
	Foo().Bar("test", Bar());     // Works, arg1 is a string, Bar() extends from class Foo

	Foo().Bar(1, Bar());          // Error: 1 is a Numerical, not a string.


Every method will return at least one value. You can return an explicit value by using 'return', otherwise the result
of the last used action will be returned.

::

    class Foo {
        public method Bar() {
            $a = 5;
        }

        public method Baz() {
            $a = 5;
            return 2;
        }
    }

    $b = Foo().Bar();     // returns 5
    $b = Foo().Baz();     // returns 2;


Self
====
You probably want to reference class properties inside the class methods. This is done by the special keyword `self`.

::

    class Foo {
        protected property $prop;

        public method Bar($arg) {
            self.$prop = $arg;
        }

        public method getProp() {
            return self.$prop;
        }
    }


.. note::
	The `self` keyword can only be used inside class methods. Outside class methods the self keyword will throw an
	exception.


Parent
======
If classes are extended from other classes, sometimes you want to call those methods.

::

    class Foo {
        public method Baz($arg) { }
    }

    class Bar extends Foo {
        public method Baz($arg) {
            parent.Baz($arg);       // Calls the Baz method from the Foo class
        }
    }



Constructing and destructing objects
====================================
Whenever an object is instantiated, Saffire will automatically call the ctor() method from that class. This is called
the constructor method. It's possible to add multiple arguments to a class, which automatically gets passed to the
constructor.

::

	class Foo {
		protected property $foo;

		public method ctor() {
		}
	}

	$a = Foo();

::

	class Foo {
		public method ctor($arg) {
		}
	}

	$a = Foo();             // Not possible, must pass an argument, since we don't have a default value
	$a = Foo("something");  // Automatically calls ctor("something")


A constructor is the only method that will have a different default return value. Not the result of the last expression
is returned, but a reference to itself. This means that you can use a fluid interface right from the constructor.

::

    $a = Foo().test();

Note that in this case: the result of test() gets saves in $a and the actual Foo() object isn't saved at all.

 ::

     $a = Foo().bar();
     $a = Foo().baz();

This would actually instantiate the Foo() method twice.


Destructing an object is done whenever there are no references to that object. It is called automatically by Saffire
during the cleanup.

.. note::
	It is not possible to call the ctor() or dtor() methods directly. This will result in an exception.

::

	class Foo {
		public method ctor() { }
		public method dtor() { }
	}

	$a = Foo();   // Calls ctor()
	$a = Null;    // calls dtor(), since there are no references

::

	$a = Foo();   // Calls ctor()
	$b = $a;      // $b is a reference to the object $a
	$a = Null;    // Foo.dtor is not called, since there is still a reference
	$b = Null;    // calls dtor(), since there are no references

.. hint::
	Saffire implements through its base object class the refcount() method that returns the number of reference the
	current object holds.



Fluent interface
================
A fluent interface means that (almost) every method will return the actual method once the method has been completed.

::

    class Foo {
        protected property $_bar;
        protected property $_baz;

        public method ctor($arg) {
            self.$_bar = 0;
            self.$_baz = 0;
        }

        public method bar() {
            self.$_bar++;
            return self;
        }

        public method baz() {
            self.$_baz++;
            return self;
        }
    }

    $a = Foo().baz().baz().bar(); // Will return a foo() object with _baz = 2, and _bar = 1;


Final
=====
Classes and/or methods can be finalized. This means that it cannot be extended by another class

::

	Final class Foo {
	}

	class Bar extends Foo {  
	}  // This is not possible

Or finalized methods:

::

	class Foo {
		final public method Baz() {
		}
	}

	class Bar extends Foo {  
		public method Baz() {
			// This is not possible
		}
	}  


Abstract classes
================
Abstract classes are classes that by itself cannot be instantiated, but can be extended.

::

	Abstract class Foo {
	}

	class Bar extends Foo {
	}

	$a = Foo();   // Cannot instantiate an abstract class
	$b = Bar();   // Works correctly.

Abstract classes can hold abstract methods. These methods only have a method definition, but no body. It's up to the
class that extends this class to implement the body.

::

	Abstract class Foo {
		abstract public method Bar(String $a, Numerical $b);
	}

	class Bar {
		public method Bar(String $a, Numerical $b) {
			// Body of the method
		}
	}


Visibility
==========

There are three kind of visibilities in Saffire:

- `public visibility`_
- `protected visibility`_
- `private visibility`_

Public visibility
-----------------
A public property or method can be called directly from any other class.

::

	class Foo {
		public property $bar = "baz";
	}

	$a = Foo.bar;


Protected visibility
--------------------
A protected property or method can only be called from its own class *or* any classes that have extended the class

::

	class Foo {
		protected property $bar = "baz";

		public method test() {
			self.bar = "qux";	// This is allowed
		}
	}

	class Bar extends Foo {
		public method test2() {
			self.bar = "quxx";	// This is allowed, since we extend from Foo
		}
	}

	$a = Foo.bar;  // This is not allowed


Private visibility
------------------
A private property or method can *only* be called from its own class. Any classes that extend this class *cannot* access.

::

	class Foo {
		private property $bar = "baz";

		public method test() {
			self.bar = "qux";	// This is allowed
		}
	}

	class Bar extends Foo {
		public method test2() {
			self.bar = "quxx";	// This is not allowed
		}
	}

	$a = Foo.bar;  // This is not allowed


Static methods
==============
It's not always necessary to create an object when you just want to call a class method or property. For instance, the
 print method of the io class can be called without instantiating an io object.

::

    class Foo {
        static public method test() {
            io.printf("The test method is called statically");
        }
    }

It's possible to call the `test` method directly, by referencing this method directly from the object.

::

    Foo.test();

You can call both static methods and properties. Note however, that the *self* and *parent* keyword are not cannot
available inside static methods. To reference a static method or property, use the *static* keyword;

::

    class Foo {
        static protected property $a = 1;
        protected property $b = 2;

        static public method Bar() {
            static.$a = 5;        // Allowed
            static.Baz();         // Allowed
        }

        static public method Baz() {
            static.Quk();         // Not allowed. Calling a non-static method
            return static.$a;     // Return static property
        }

        public method Quk() {
            self.Bar();         // We CAN reference static methods like this
            return self.$a;     // Static property, but allowed.
        }
    }



:Authors:
   Joshua Thijssen
