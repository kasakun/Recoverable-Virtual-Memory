# Recovery-Virtual-Memory

## How to compile

In the root directory, run
```
$ make

```
to build the RVM library librvm.a

Users can compile each test, i.e. compiling abort test

```
$ make abort
```

Or, users can compile all test cases using

```
$ make all
```

To make sure the code is compiled correctly, before each make, clean is recommended

```
$ make clean
```

All executable files are located in the specified directories in the bin directory.


## How to run
In each directory, simply run

```
./[test_name]  #i.e. abort
```

## Contributions of Each Team Member
Zeyu Chen: Design appropriate data structures. Implement initialization and
mapping operations of RVM library. Also implement library output operation of
RVM library. Design 1 additional test case.

Yaohong Wu: Design appropriate data structures. Implement transactional
operations of RVM library. Design 2 additional test cases.
