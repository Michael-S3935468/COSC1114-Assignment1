#!/usr/bin/env bash
###############################################################################
# Task1.sh
# ========
#
# Description:           Task 1
# Author:                Michael De Pasquale
# Creation Date:         2022-08-19
# Modification Date:     2022-08-19
#
###############################################################################

WORD_FILE="wlist_all/wlist_match1.txt"

# 1. Read file
# 2. Shuffle randomly
# 3. Remove punctuation
# 4. Select words of length 3 to 15 letters (inclusive)
# 5. Sort and remove duplicates
cat "$WORD_FILE" \
    | shuf \
    | sed 's/[^A-Za-z]//' \
    | grep -oP '^.{3,15}$' \
    | sort \
    | uniq
