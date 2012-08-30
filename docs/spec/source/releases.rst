########
Releases
########

All releases are in the form of ``major.minor.release``.

Major versions
==============
Every major release will increase the ``major`` number. There is no time frame for when new major releases will be
published. A release will be considered major when at least one of the following rules are true:

* There are backward compatibility breaks
* There are a certain number of new features that core-developers consider a major release worthy.


Minor versions
==============
Minor versions are available:

* When more smaller features are implemented.
* Bug fixes, security updates or features are implemented that modified the internal structures used by Saffire.
  For instance, a bug fix that adds a new field in the internal hashtable structure must be done in a minor update,
  not in a release update.

.. attention::
  There will be no backward compatibility breaks when dealing with minor versions.


releases
========
Release updates will be done when only small fixes are implemented. This includes hot fixes like (major) security
updates, bug fixes or small new functionality.



Backward compatibility
======================
Major releases **can** include backward compatibility breaks. It is not guaranteed that Saffire 1.x applications will
run on the Saffire 2.0 engine. Although backward compatibility breaks will be kept for a minimum as much as possible.
It's not guaranteed new versions will be backwards compatible. We have chosen for this setup to make sure that we can
guarantee backwards compatibility through minor releases, but still can move the language and engine forward without
being "held back" by backward compatibility.


:Authors:
   Joshua Thijssen
   Caspar Dunant
