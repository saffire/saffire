###########
Overloading
###########


Operator overloading
====================

It's possible in Saffire to overload operators. This means that whenever an operator (+, -, /, *, [], <<, >>) is used in combination with a class, it will automatically call the correct operator overloading method. Operator overloading methods are different from regular methods, since they are in the format 

	::<operator>


For instance:

::

	class Foo {
		public method ::+(String $s) {
		}
	}

	$a = Foo();
	$b = $a + "test";	// This will call the ::+ method, since we are adding a string
	$b = $a + 1;		// This result in an error, since there is no + overloading for a Numerical


Operator overloading *can* make it easier to work with objects. Just like you can add numerical values like 1 + 3, it sometimes makes sense to do this with objects as well.

Suppose we have a class Color, which holds a specific color. It would make sense that whever we have a Color("yellow") and add a Color("blue"), we end up with a Color("green"), since the colors have been mixed.

::
	
	$c = Color("yellow") + Color("blue");   // $c = Color("green")

Without operator overloading, you would have to specify another method for this:

::

	$c = Color("yellow")->mix(Color("blue"));	// Does the same as the example above, but less readable.


.. warning::
	It's quite possible to overload operators that makes no sense, like subtracting values through the + operator, and adding them with. Always make sure that whenver you overload operators, they make sense in the context you use them.



Method overloading
==================

Method overloading allows us to have the same method names, but with different arguments:

::

	class Foo {
		method Bar(String $arg) { }
		method Bar(Numeric $arg) { }
		method Bar(String $arg1, String $arg2) { }
		method Bar(String $arg1, $arg2) { }
	}

Depending on which arguments you pass, it will call the correct "bar" method.

::

	Foo.Bar("foo");         // calls method 1
	Foo.Bar(1);             // calls method 2
	Foo.Bar("foo", "bar");  // calls method 3 (could have called method 4)
	Foo.bar("foo", 3);      // calls method 4

Sometimes, saffire has multiple candidates to call. In this case, Foo.Bar("foo", "bar") could either call method 3 or 4. In those situations, Saffire uses the most specifiec method. In this case it's method 3, since we specify 2 strings, while method 4 specifies 1 string and any object. 

Extended classes have a higher priority than base classes:

::

	class baseclass {
	}

	class higherclass extends baseclass {
	}

	class Foo {
		method Bar(baseclass $foo) {}
		method Bar(higherclass $foo) {}
	}

	Foo.Bar(higherclass());  // Class method 2


In situations where there is a tie, the first strictest parameter will win:

::

	class Foo {
		method Bar(String $a, $b);
		method Bar($a, String $b);
	}

	Foo.Bar("a", "b");	// method 1 wins, param 1 from method 1 (string) is stricter than param 1 from method 2 (object).



:Authors:
   Joshua Thijssen