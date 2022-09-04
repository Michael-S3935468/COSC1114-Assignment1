#!/usr/bin/env bash
###############################################################################
# Task1.sh
# ========
#
# Description:           Task 1
# Author:                Michael De Pasquale
#
###############################################################################

if [ "$#" != '2' ]; then
    echo "Usage: Task1.sh {input_path} {output_path}" >&2
    exit 1
fi

DIRTY_FILE="$1"
CLEAN_FILE="$2"

# 1. Read file
# 2. Shuffle randomly
# 3. Remove punctuation
# 4. Select words of length 3 to 15 letters (inclusive)
# 5. Sort and remove duplicates
cat "$DIRTY_FILE" \
    | sed 's/[^A-Za-z]//g' \
    | grep -oP '^.{3,15}$' \
    | sort \
    | uniq \
    | shuf > "$CLEAN_FILE"
