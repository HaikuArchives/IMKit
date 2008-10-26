#!/bin/env python

"""GimICQ2IM v2003-12-05 - Convert a Gim-ICQ user list file to people files w/ UINs. 
 
Usage: GimICQ2PIM.py ~/config/settings/GimICQ/user-UIN Folder
Replace UIN for your UIN.

Creates People files named after the nick names in the user list -
attributes set for the IM Kit - in the directory Folder

2003-12-05: Setting IM Status to offline upon creation.
2003-11-25: Initial version

Copyright (C) 2003 by Mikael Jansson <mikael at dundermusen dot net>"""

def extract(line):
	"""Return a tuple (uin, nick) from 'O11111111	nickname'"""
	
	line = line.replace("\n", "")

	uin = line[1:line.find("\t")]
	
	# fix uin
	uin2 = ""
	for c in uin:
		if c.isdigit():
			uin2 += c
	uin = uin2
	
	nick = line[1+line.find("\t"):]
	nick = nick.replace("/", "_")
	nick = nick.replace(":", "_")
	return (uin, nick)

import os
def writePerson(path, (uin, nick)):
	#fsattr.write_attr(filename, attr_name, attr_type, attr_data, flags)
	fname = os.path.normpath(os.path.join(path, nick))
	file = open(fname, "w+")
	file.close()
	connattr = "ICQ:"+uin
	os.system("addattr -t mime 'BEOS:TYPE' '%s' '%s'" % ('application/x-person', fname))
	os.system("addattr -t string 'IM:connections' '%s' '%s'" % (connattr, fname))
	os.system("addattr -t string 'IM:status' '%s' '%s'" % ('offline', fname))

from xreadlines import xreadlines

def main():
	import sys
	
	if len(sys.argv) < 3:
		print __doc__

	if not os.path.isfile(sys.argv[1]):
		print __doc__
	
	settings = sys.argv[1]
	folder = sys.argv[2]
	
	if not os.path.isdir(folder):
		os.makedirs(folder)

	nicklist = map(extract, xreadlines(open(settings)))

	for nick in nicklist:
		writePerson(folder, nick)	
	
if __name__ == '__main__':
	main()
