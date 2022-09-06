#!/usr/bin/env bash
###############################################################################
# cleanup.sh
# ==========
#
# Description:           Remove word list files and named pipes created by
#                        task1-task4 binaries.
# Author:                Michael De Pasquale
#
###############################################################################

# Output
rm -f "Task1out.txt"
rm -f "Task1out.sh.txt"
rm -f "Task2out.txt"
rm -f "Task3out.txt"
rm -f "Task4out.txt"

# Word sub-lists
for IDX in 3 4 5 6 7 8 9 10 11 12 13 14 15
do
    # Task 2
    rm -f ".tmp.task2.words$IDX.txt"
    rm -f ".task2.words$IDX.txt"

    # Task 3
    rm -f ".task3.words$IDX.txt"

    # Task 4
    rm -f ".task4.words$IDX.txt"
done
