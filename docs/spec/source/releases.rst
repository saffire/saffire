########
Releases
########

All releases are in the form of ``major.minor.release``.

Major versions
==============
Every major release will increase the ``major`` number. There is no timeframe for when new major releases will be
published. A release will be considered major when at least one of the following rules are true:

* There are backward compatibility breaks
* There are a certain number of new features that core-developers consider a major release worthy.


Minor versions
==============
Minor versions are available:

* When more smaller features are implemented.
* Bugfixes, security updates or features are implemented that modified the internal structures used by Saffire.
  For instance, a bugfix that adds a new field in the internal hashtable structure must be done in a minor update,
  not in a release update.

.. attention::
  There will be no backward compatibility breaks when dealing with minor versions.


releases
========
Release updates will be done when only small fixes are implemented. This includes hotfixes like (major) security
updates, bugfixes or small new functionality.



Backward compatibility
======================
Major releases **can** include backward compatibility breaks. It is not guaranteed that Saffire 1.x applications will
run on the Saffire 2.0 engine. All though BC breaks will tried to be kept for a minimum, it's not guaranteed they will
not happen. We have choosen for this setup to make sure that we can guarantuee BC through minor releases, but still can
move the language and engine forward without being "held back" by BC.


:Authors:
   Joshua Thijssen
