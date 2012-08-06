###############
Data structures
###############


List
----
Lists are data structures with values only.

$a = [ "foo", "bar", "baz" ];

this represents a list of 4 Strings, which can be accessed through the [] notation.

$a[0]  // "foo"
$a[1]  // "baz"
$a[2]  // "bar"

Hash
----
Hashes have the same properties as lists, but each value has a (unique) key.

$a = { 1 : "foo", 2 : "bar", "test" : "baz" };

$a[1]       // "foo"
$a["test"]  // "baz"


Sets
----
Sets are lists, but can only hold unique values.

$a = [[ "foo", "bar", "baz", "foo", "bar" ]];

// $a hols "foo", "bar" and "baz"

Sets can be used to quickly add, subtract or check items.


Tuples
------

$a = ( "foo", "bar" );

Tuples are read-only lists. They can be used to group values.

$a, $b, $c = "foo", ("bar", "baz"), "qux";

// $a = "foo"
// $b = ("bar", "baz")
// $c = "qux";
