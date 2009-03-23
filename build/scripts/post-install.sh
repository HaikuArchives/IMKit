#!/bin/sh

MKINDEX=mkindex
MIMEATTR=mimetype_attribute

# Create indexes
echo "Creating indexes..."
$MKINDEX -t string IM:connections
$MKINDEX -t string IM:status

# Add attributes to application/x-person
echo "Adding attributes to Person files..."
$MIMEATTR --mime application/x-person --internal-name "IM:status" \
	--public-name "IM Status" --type string --width 80 --viewable --public --not-editable
$MIMEATTR --mime application/x-person --internal-name "IM:connections" \
	--public-name "IM Connections" --type string --width 80 --viewable --public
