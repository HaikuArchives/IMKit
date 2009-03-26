#!/bin/sh
#
# Copyright 2008-2009, IM Kit Team.
# Distributed under the terms of the MIT License.
#
# Authors:
#		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#

# Replacement for which to make it work in Zeta
function which_replacement()
{
 	location=`which $1 2>/dev/null`
 
 	if [ -z "$location" ]; then
 		for p in `echo $PATH | awk '$1=$1' RS=: OFS="\n"`; do 
 			if [ -x $p/$1 ]; then 
 				location=$p/$1
 				break
 			fi 
 		done 
 	fi

	echo $location
 	return
}

MKINDEX=`which_replacement mkindex`
MIMEATTR=`which_replacement mimetype_attribute`

if [ -z "$MKINDEX" ]; then
	echo "mkindex could not be found, aborting..."
	exit 1
fi

if [ -z "$MIMEATTR" ]; then
	echo "mimetype_attribute could not be found, aborting..."
	exit 1
fi

# Create indexes
echo "==> Creating indexes..."
$MKINDEX -t string BEOS:APP_SIG
$MKINDEX -t string IM:connections
$MKINDEX -t string IM:status

# Add attributes to application/x-person
echo "==> Adding attributes to Person files..."
$MIMEATTR --mime application/x-person --internal-name "IM:status" \
	--public-name "IM Status" --type string --width 80 --viewable --public --not-editable
$MIMEATTR --mime application/x-person --internal-name "IM:connections" \
	--public-name "IM Connections" --type string --width 80 --viewable --public
