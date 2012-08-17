##################
Control structures
##################

if
--
An if-statement let you execute code only when a certain condition have been met.

::

	if ($a == 1) {
		// only done when $a equals 1
	}

Off course, when you only have one statement, you can omit the brackets:

::

	if ($a == 1) dothis();


It's also possible to have an else-statement, which gets executed when the condition is NOT met:

::

	if ($a == 1) {
		// done when $a == 1
	} else {
		// done when $a not equals 1
	}

It's also possible to chain if/else statements:

	if ($a == 1) {
		// done when $a == 1
	} else if ($a == 2) {
		// done when $a == 2
	} else {
		// done when $a not equals 1 or 2
	}

while
-----
A while loop will iterate over the statements until the evaluation will be false.

::

	while ($a < 10) {
		$a++;
	}

The while-structure also accepts an else-statement. This else statement is executed if the **initial** evaluation is NOT true **or** when breakelse is called from inside the loop.

::

	while ($a < 10) {
		$a++;
	} else {
		// Called when $a was 10 or more
	}



dowhile
-------
A do-while loop is similar to a while loop except the loop will **at least** be executed once, since the evaluation will be done at the end of the loop. This means that do-while does not support the else statement.

::

	$a = 1;
	do {
		$a++;
	} while ($a < 10);



for
---
**for** loops can be completely controlled by the three expressions given in the header:

	for (epxr1 ; expr2 ; expr3) {}

At the beginning of the loop, the expr1 will be executed.  On every iteration, expr2 is evaluated. If this expression returns False, the for-loop will be ended, otherwise the loop will be executed. At the end of each iteration, expr3 is called.

A classis way to loop over 1 to 10:

::
	
	for ($i=1; $i<=10; $i++) {
		// $1 will be 1 to 10
	}

All expression can be empty. If expr2 is empty, it will be considered True. The following statement will be a endless loop:

::

	for (;;) {
		// do this endlessly, until we hit a break statement
	}



foreach
-------
The **foreach** operator allows you to iterator over **any** object that implements the .iter() method. 

::

	foreach ($object as $value) {
		// Do something with value
	}

The .iter() method returns a tuple of 3 variables:

- value
- key
- metadata

The value is the actual value that is returned. For instance, a list datastructure will only return the elements, since there aren't any keys.

::

	foreach (list["foo", "bar"] as $value, $key) {
			// $value = "foo" or "bar"
			// $key will always be Null
	}

Data structures like hashes, which contain keys, will return keys as well.

::

	foreach (hash[2 : "foo", 4: "bar"] as $value, $key) {
			// $value = "foo" or "bar"
			// $key will be 2 or 4
	}

There is an additional field called $meta, which is filled with foreach metadata:

.index
	The current index, 0-based
.first
	True when the element is the first element of the loop (False otherwise)
.last
	True when the element is the last element of the loop (False otherwise)
.count
	The number of elements, if available and known
.key
	The key, as returned by $key
.val
	The value, as returned by $value


.. note:: 
	.iter() methods can add additional information to the meta data, for instance, with a priority-queue, the actual priority of an element can be stored in $meta.priority

::

	foreach (String.range('a','z') as $value, $key, $meta) {
		if (meta.first) {
			// First element ($value = 'a')
		}
		if (meta.last) {
			// Last element ($value = 'z')
		}
		// meta.count == 26
		// meta.index == 0..25
	}	

.. note:: 
	A datastructure (an object that implemented "datastructure", like the list, hash etc), already have implemented the .iter() method and thus are iterable.


Foreach can be accompanied by an else. This is called whenever the .iter() is not implemented, or when there is an empty list, or when `breakelse`_ is issued inside the foreach{} block.



break
-----
**Break** can be used to end the execution of `for`_, `foreach`_, `while`_, `dowhile`_ and `switch`_ statements. See the corresponding statements for more information.



breakelse
---------
The **breakelse** statement is used in the `for`_ and `while`_ statements. They act the same way as a normal break statement, but when a for and while statement are accompanied with an else statement, it will automatically execute that as well.

::

	while ($a) {
		$a--;
		if ($a == 5) breakelse;	// Break the loop, but execute else
		if ($a == 3) break;	// Break the loop, but don't execute else
	} else {
		// Do something else when $a initially was false, or when breakelse was triggered
	}

.. note::
	When no else statement has been given, the breakelse behaves the same was as a break statement.



switch
------
**switch** can be seen as a multi-if statement. Instead of evaluating one expression, it evaluates many expressions and directly executes those statements

::

	if ($a == 1) {
		...
	} else if ($a == 2) {
		...		
	} else if ($a == 3) {
		...	
	} else {
		...
	}

With **switch** this can be rewritten as:

::

	switch ($a) {
		case 1 :
			...
			break;
		case 2 :
			...
			break;
		case 3 :
			...
			break;
		default:
			...
			break;
	}

.. warning::
	When omitting the `break`_ in a case statement, it will automatically fall through the next statement:

::

	switch ($a) {
		case 1 :
			$a += 1;
			// Will continue with the next statement
		case 2 :
			$a += 1;
			// Will continue with the next statement
		case 3:
			$a += 1;
	}



continue
--------
**Continue** is used to end a current loop and continue with the next iteration. 

::

	for ($i=0; $i!=10; $i++) {
		if ($i % 2 == 0) continue;
		// $i is an odd number.
	}



return
------
**Return** will exit a method and return to the caller. Calling return from the global scope will stop execution of the script.

::

	// Global scope, can only return a Numerical exit code
	return 1;


::
	
	class Foo {
		method Bar() {
			return "Baz";
			// Will not be called
		}
	}

	$a = Foo.Bar();   // $a = "Baz"

Note that return can include zero or more arguments. Those arguments are directly returned to the caller. If at the end of a method no return statement is given, the result of the last expression will be the return value.

::

	class Foo {
		method Bar() {
			$a = "Baz";
		}
	}

	$b = Foo.Bar();   // $b = "Baz", since that is the last result in the method Bar. 



goto
----
**Goto** can be used to jump directly to a label. These labels are defined as <name>: at the beginning of a line. Note that you cannot jump outside a code block. This means that you can only **goto** a labael inside the same method for instance. 

::

	class Foo {
		method Bar() {
			goto label1;
			// This part is skipped
		label1:

		}
	}


::

	class Foo {
		method Bar() {
			goto label1;	// This does not work
		}

		method Baz() {
		label1:
		}
	}

Also, you cannot jump inside a loop.

::

	goto loop;
	while ($a < 10) {
	loop:
		$a++;
	}

.. warning::
	Even though **goto** might be seen as **evil** by many programmers, it does have its purpose. But not many. If you are not sure wether or not you should use **goto**, you probably are "doing it wrong"(tm).



:Authors:
   Joshua Thijssen
