##########
Interfaces
##########

Saffire interfaces are boilerplate classes that a class can implement. They are similar to abstract classes, however:

* A class can implement multiple interfaces, but only extend one (abstract) class
* An interface can only describe methods, not implement method bodies.
* An interfaces cannot have properties or constants
* An abstract class can have abstract methods, but also methods with bodies.

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
   Caspar Dunant
