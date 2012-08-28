##########
Interfaces
##########

Saffire interfaces is a boilerplate class that a class that uses the interface must implement. They are similar to
abstract classes, however:

* A class can only extend one (abstract) class
* An abstract class can have abstract methods, but also methods with bodies.
* Interfaces cannot have properties or constants 
* A class can implement multiple interfaces

::

	interface Foo {
		public method Bar(String $a);
		public method Baz(String $b);
	}

	class Bar implements Foo {
		public method Bar(String $a) {
			// Body of method
		}

		public method Baz(String $b) {
			// Body of method
		}
	}


:Authors:
   Joshua Thijssen