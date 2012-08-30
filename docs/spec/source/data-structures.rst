###############
Data structures
###############


Data structures are first-class citizens in Saffire. 

Every data structure is handled the same way:

	<structure>[key:val1:val2,key:val1:val2,...]

A list can be defined as:

::

	list["foo", "bar", "baz"];

If a data structure has multiple arguments per element (like a key/value pair), you can separate them with a ``:``

::

	hash["foo":1, "bar":2, "baz":3];

More values are possible too, for instance, a priority-queue, which not only has key/value pairs, but also an additional
priority.

::

	priority["foo":1:10, "bar":2:10, "foo":3:20];


There are 3 internal data structures available:

- `List`_
- `Hash`_
- `Set`_

It's possible to create your own data structures. Be advised that by using custom data structures, you sacrifice
memory and speed compared to the native data structures. Your structure will never be as fast as the native ones.



List
----
Lists are data structures with values only. A value can be available multiple times inside the list 

::

	$a = list[ "foo", "bar", "baz" ];

this represents a list of three Strings, which can be accessed through the ``[index]`` notation:

::

	$a[0]  // "foo"
	$a[1]  // "baz"
	$a[2]  // "bar"



Hash
----
Hashes have the same properties as lists, but each value has a unique key.


::

	$a = hash[ 1:"foo", 2:"bar", "test":"baz" ];

	$a[1]       // "foo"
	$a["test"]  // "baz"



Set
---
Sets are like lists, but can only hold unique values. Every time a value is added that already exists, that value will
be ignored.


::

	$a = [[ "foo", "bar", "baz", "foo", "bar" ]];
	// $a holds "foo", "bar" and "baz"

Sets can be used to quickly add, subtract or check items.



Tuples
------
Tuples work a bit differently than the other data structures. A tuple is an internal structure that holds zero or more
other elements.

::

	$a = ( "foo", "bar" );

Tuples are read-only lists. They can be used to group values.

::

	$a, $b, $c = "foo", ("bar", "baz"), "qux";

	// $a = "foo"
	// $b = ("bar", "baz")
	// $c = "qux";



:Authors:
   Joshua Thijssen
   Caspar Dunant
