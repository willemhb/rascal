#!/bin/bash

# SCRIPT_DIR_REL=$(dirname ${BASH_SOURCE[0]})
# SCRIPT_DIR_ABS=$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
# CURRENT_DIR=$(pwd)

# echo "Current directory: $CURRENT_DIR"
# echo "Relative script dir: $SCRIPT_DIR_REL"
# echo "Absolute script dir: $SCRIPT_DIR_ABS"

cat "/home/willem/Programming/C/include/tpl/module.c.tpl"
cat "/home/willem/Programming/C/include/tpl/module.h.tpl"
