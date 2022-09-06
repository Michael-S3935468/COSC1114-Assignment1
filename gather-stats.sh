#!/usr/bin/env bash
###############################################################################
# gather-stats.sh
# ===============
#
# Description:           Gather performance stats for each task
# Author:                Michael De Pasquale
#
###############################################################################


echo "Task1.sh"
sudo perf stat -d ./Task1.sh wlist_all/wlist_match1.txt Task1out.sh.txt

echo "Task1"
sudo perf stat -d ./task1 wlist_all/wlist_match1.txt Task1out.txt

echo "Task2"
sudo perf stat -d ./task2 wlist_all/wlist_match1.txt Task2out.txt

echo "Task3"
sudo perf stat -d ./task3 wlist_all/wlist_match1.txt Task3out.txt

echo "Task4"
sudo perf stat -d ./task4 wlist_all/wlist_match1.txt Task4out.txt
