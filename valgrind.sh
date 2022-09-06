#!/usr/bin/env bash
###############################################################################
# valgrind.sh
# ===========
#
# Description:           Run valgrind
# Author:                Michael De Pasquale
#
###############################################################################


echo "Task1"
valgrind ./task1 wlist_all/wlist_match1.txt Task1out.txt

echo "Task2"
valgrind ./task2 wlist_all/wlist_match1.txt Task2out.txt

echo "Task3"
valgrind ./task3 wlist_all/wlist_match1.txt Task3out.txt

echo "Task4"
sudo valgrind ./task4 wlist_all/wlist_match1.txt Task4out.txt
