#!/bin/sh

if test $# '<' 2; then
	printf "Usage:\n\t%s ~/config/settings/GimICQ/user-UIN outputfolder\n" $0
	printf "\twhere UIN is the UIN of the user you want to import.\n"
	exit 1;
fi

settingfile="$1"
outputfolder="$2"


uin=0;
nick="";
createpeepsfile()
{
	connection="ICQ:$uin"
	peepfile="$outputfolder/gimimport-$nick"

	echo Creating $peepfile
	touch "$peepfile"
	addattr -t mime 'BEOS:TYPE' 'application/x-person' "$peepfile"
	addattr -t string 'IM:connections' "$connection" "$peepfile"
	addattr -t string 'IM:status' 'Offline' "$peepfile"
	addattr -t string 'META:nickname' "$nick" "$peepfile"
}

{
	while read line; do
		uin=`echo -n $line | awk '{print $1}' | sed s/O//`
		nick=`echo -n $line | awk '{print $2}'`
		createpeepsfile;
	done;
} < $settingfile
