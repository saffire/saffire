###########
Overloading
###########


Operator overloading
====================

It's possible in Saffire to overload operators. This means that whenever an operator (+, -, /, *, [], <<, >>) is used in
combination with a class, it will automatically call the correct operator overloading method. Operator overloading
methods are different from regular methods, since they are in the format

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


Operator overloading *can* make it easier to work with objects. Just like you can add numerical values like 1 + 3, it
sometimes makes sense to do this with objects as well.

Suppose we have a class Color, which holds a specific color. It would make sense that whever we have a Color("yellow")
and add a Color("blue"), we end up with a Color("green"), since the colors have been mixed.

::
	
	$c = Color("yellow") + Color("blue");   // $c = Color("green")

Without operator overloading, you would have to specify another method for this:

::

	$c = Color("yellow")->mix(Color("blue"));	// Does the same as the example above, but less readable.


.. warning::
	It's quite possible to overload operators that makes no sense, like subtracting values through the + operator, and
	adding them with. Always make sure that whenver you overload operators, they make sense in the context you use them.



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

Depending on which arguments you pass, it will call the correct "Bar" method.

::

	Foo.Bar("foo");         // calls method 1
	Foo.Bar(1);             // calls method 2
	Foo.Bar("foo", "bar");  // calls method 3 (could have called method 4)
	Foo.bar("foo", 3);      // calls method 4

If the actual class is not found, parent objects and interfaces will be checked. Since all objects extend from the
Object method, having no typehinting is the same as specifing Object as the typehint. Extended classes have a higher
priority than base classes:

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

When deciding which method Saffire needs to call, it will generate a "path" for every class. In this example, the class
path would be:

::

    higherclass -> baseclass -> object


But sometimes two different candidates are possible, with the same base path's:

::

    class Foo {
        method Bar(String $arg1, $arg2);
        method Bar($arg1, String $arg2);
    }

Calling `Foo.Bar("foo", 1);` is an easy call, method 1 will be called, since that is the only candidate available to
handle arguments "String, Object". But calling `Foo.Bar("foo", "bar");` isn't easy. Both methods support "String,
Object" and "Object, String", and there isn't a more specific argumentlist we can match.

In this case, ambiguity arrises. Saffire will resolve this by using the most specific first fit. Since a string matches
a string better than an object, the first method will be called, since the first argument with the best fit is actually
the first argument, while the second method has the first best fit in the second argument.


Saffire has multiple candidates to call. In this case, Foo.Bar("foo", "bar") could either call method 3 or 4.
In those situations, Saffire uses the method with the most **specific** argumentlist. In this case it's method 3, since
we specify 2 strings while method 4 specifies 1 string and any object.


However, sometimes complex hierarchies can appear with mixed extends and interfaces:

::

    interface A { }
    interface B implements A { }
    class Foo implements B;
    class Bar extends Foo implements A;

    class Qux {
        public method test(A $arg1, B $arg2) { ... }
        public method test(B $arg1, A $arg2) { ... }
    }

    Qux.test(Foo(), Bar());
    Qux.test(Bar(), Foo());

So which method should be called in the first call?

The path would be:

::

    Bar -> Foo
     |      |
     A      B
            |
            A

In this situation the first call could either call the first or the second method, since both combinations result in
the same base paths.

::

	class Foo {
		method Bar(String $a, $b);
		method Bar($a, String $b);
	}

	Foo.Bar("a", "b");	// method 1 wins, arg 1 from method 1 (string) is stricter than arg 1 from method 2 (object).



:Authors:
   Joshua Thijssen