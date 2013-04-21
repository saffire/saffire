Unittests
---------
Saffire tries to do as much unittesting as possible. There are two different ways to unittest:

  - Through .sfu files
  - Through CUnit

With CUnit, we test internal core code like hashtables, dlls, memory management etc. The SFU files are highlevel
unittests where the tests have been writting in Saffire.

Running CUnit unittests
-----------------------
You can run the CUnit tests through the following command:

    make check

Running SFU tests
-----------------
You can run the SFU testsuite through the following command:

    sh unittest.sh
