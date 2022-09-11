# README

## Requirements

* gcc
* make

## Compilation

Run 'make' to build each of the task binaries.

## Usage

* gather-stats.sh will run the task 1 script and each binary using
  wlist_match1.txt, recording performance data and writing the output for each
  to a corresponding file.

* After running any binary, you MUST run cleanup.sh to remove temporary files.
  If you don't do this, task3 and task4 will fail when attempting to create
  the named pipes.

* valgrind.sh will run valgrind for each binary.

* The scripts and binaries can be run individually. Each takes two arguments,
  first is the input file and second is the output file.
  For example:

    ./task3 wlist_all/wlist_match1.txt Task3out.txt

* task4 requires elevated permissions to function:

    sudo ./task4 wlist_all/wlist_match1.txt Task4out.txt
