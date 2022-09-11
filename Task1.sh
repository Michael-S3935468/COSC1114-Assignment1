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
# 2. Remove words containing non-letters, or with length not within 3-15
#    letters (inclusive)
# 3. Remove duplicates (sorting required for uniq)
# 4. Shuffle randomly
# 5. Sort from third letter onwards
cat "$DIRTY_FILE" \
    | grep -oP '^[A-Za-z]{3,15}$' \
    | sort \
    | uniq \
    | shuf \
    | sort -b -k "1.3" > "$CLEAN_FILE"
