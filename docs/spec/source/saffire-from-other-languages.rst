############################
Saffire from other languages
############################

Saffire for PHP programmers
---------------------------
There are some differences between PHP and saffire

- There are no (global) functions.
- Scalar values like strings and integers are objects too.
- Saffire is strong typed. It will not convert variables from integers to strings or vice versa.
- Saffire strings are always UTF-8. The returned length of a string is the number of characters, not the actual number
  of bytes.
- __constructor and __destructor methods are called ctor() and dtor() respectively.
- Array's are hashmaps, used as the ``hash[]`` data structure. Numerical arrays are used as ``list[]`` structures.
- Saffire supports method and operator overloading
- Methods and properties visibilities must be explicitly stated (they are not implied public)

Saffire for Python programmers
------------------------------
-
-
-
-
-
-

Saffire for Ruby programmers
----------------------------
-
-
-
-
-
-

:Authors:
   Joshua Thijssen
