Team #7, Lab 4
==============

Documentation
-------------
README.txt - This file. Contains information about all files in tarball.
docs       - Contains program documentation.
lab4.diff  - Contains a diff on the Linux 2.6.9 source containing our kernel
             level changes. Includes both the cp_range() and inc_cp_range()
             system calls compiled in.
tests      - Contains testing programs.

Documentation Files in docs
---------------------------
Lab4CheckpointDesign.pdf - Design document for incremental and basic schemes.

Testing Files in test
---------------------
test.c            - Mallocs 10 elements, cp_range call, memset(0xff), then cp_range
test10k.c         - Same as test.c but with 10k elements
testIncremental.c - Does multiple checkpoints with incremental scheme.
Makefile          - Compiles all testing files

How to run tests
----------------
$ cd tests
$ make
$ ./<testing program you want to run>
