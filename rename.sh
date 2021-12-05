#!/bin/bash
# change program name
NAME="$1"

# Get title name (with spaces and capitalized)
NAME_MX="${NAME/#mx-/MX" "}" # for MX programs replaces mx- with MX
NAME_SPACES="${NAME_MX//-/" "}"
ARRAY=($NAME_SPACES)
TITLE_NAME="${ARRAY[@]^}"

# Rename files
rename "s/CUSTOMPROGRAMNAME/$NAME/" *
rename "s/CUSTOMPROGRAMNAME/$NAME/" translations/*

# Rename strings
find . -type f -exec sed -i "s/CUSTOMPROGRAMNAME/$NAME/g" {} +
find . -type f -exec sed -i "s/Custom_Program_Name/$TITLE_NAME/g" {} +

