############
Scalar types
############

Saffire is an object-only language. This means that everything, including strings, numerical values etc, are objects.

**String**
This represents an (UTF-8) array of characters. The ``length()`` of a string will always return the number of CHARACTERS,
not the number of bytes. Make sure that when dealing with binary data, you are using ``String.bytes()`` instead of
``String`` in order to retrieve the actual bytes.

**Numeric**
Numeric integer ranging from ``Numeric.MIN`` to ``Numeric.MAX``. Longer numerical values can use a different class
(LongNumeric) instead.

**Double**
Any fixed point number. Note that Saffire does not have floating point numbers.

**Boolean**
Representing either a true or a false value.


Standard objects
----------------

**Null**
Represents a null value. This object cannot be extended or changed.

**True**
A boolean representing a true value. This object cannot be extended or changed.

**False**
A boolean representing a false value. This object cannot be extended or changed.


:Authors:
   Joshua Thijssen
