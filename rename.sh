#!/bin/bash
# change program name
NAME="$1"

# Get title name (with spaces and capitalized)
NAME_MX="${NAME/#mx-/MX" "}" # for MX programs replaces mx- with MX
NAME_SPACES="${NAME_MX//-/" "}"
ARRAY=($NAME_SPACES)
TITLE_NAME="${ARRAY[@]^}"

# Rename files
rename "s/mx-samba-config/$NAME/" *
rename "s/mx-samba-config/$NAME/" translations/*

# Rename strings
find . -type f -exec sed -i "s/mx-samba-config/$NAME/g" {} +
find . -type f -exec sed -i "s/MX Samba Config/$TITLE_NAME/g" {} +

