#######
Objects
#######

Everything in Saffire is an object. Even constant strings like "foo" are represented as `string`, which makes it possible to do:

::

	"foo".length()   // returns 3

Objects are all based from a special base class (object) that has some basic functionality for each class.


Defining objects
================

::

	class Foo {
	}

Even though this object does not extends anything, it still extends our base calls object. This means it could have been written like:

::

	class Foo extends object {
	}



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

	Foo.Bar("this", "works");	// $arg1 = "this", $arg2 = "works"


An argument can also have a default value, in case nothing has been specified by the caller.

::

	class Foo {
		public method Bar($arg1 = "var", $arg2 = "default") {
		}
	}

	Foo.Bar();                 // $arg1 = "var", $arg2 = "default"
	Foo.Bar("this");           // $arg1 = "this", $arg2 = "default"
	Foo.Bar("this", "works");  // $arg1 = "this", $arg2 = "works"


It's also possible to use type hinting to make sure the arguments are from a certain class or interface:

::

	class Foo {
		public method Bar(String $arg1, Foo $arg2) {
		}
	}

	class Bar extends Foo {
	}

	Foo.Bar("test", Foo());     // Works, arg1 is a string, Foo() is of class Foo
	Foo.Bar("test", Bar());     // Works, arg1 is a string, Bar() extends from class Foo

	Foo.Bar(1, Bar());          // Error: 1 is a Numerical, not a string.



Constructing and destructing objects
====================================
Whenever an object is instantiated, Saffire will automatically call the ctor() method from that class. This is called the constructor method. It's possible to add multiple arguments to a class, which automatically gets passed to the constructor.


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


Destructing an object is done whenever there are no references to that object. It is called automatically by Saffire during the cleanup.

.. note::
	It is not possible to call the ctor() or dtor() methods directly. This will result in an error.


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
	Saffire implements through its base object class the refcount() method that returns the number of reference the current object holds. 

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

Abstract classes can hold abstract methods. These methods only have a method definition, but no body. It's up to the class that extends this class to implement the body. 

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



:Authors:
   Joshua Thijssen
