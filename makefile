# makefile for all sample code

VERSION:=0.9.1-alpha
DIST:=imkit-$(VERSION)

SUBDIRS = \
	libim \
	server \
	sample_clients \
	protocols \
	utils \
	Preflet

IMKIT_HEADERS=$(addprefix /boot/home/config/include/, $(wildcard libim/*.h))

.PHONY: default clean install dist common

default .DEFAULT : /boot/home/config/include/libim $(IMKIT_HEADERS)
	# ensure libsigc++ was properly installed
	-if test ! -e ~/config/include/sigc++-1.0/sigc++config.h && test -e ~/config/lib/sigc++-1.0/include/sigc++config.h; then \
		ln -s ~/config/lib/sigc++-1.0/include/sigc++config.h ~/config/include/sigc++-1.0/sigc++config.h; \
	fi

	#ensure OpenSSL has symlinks at necessary places
	-if test ! -e ~/config/lib/libssl.so && test -e ~/config/lib/libssl.so.0.9.7; then \
		ln -s ~/config/lib/libssl.so.0.9.7 ~/config/lib/libssl.so; \
		ln -s ~/config/lib/libcrypto.so.0.9.7 ~/config/lib/libcrypto.so; \
	fi
	-if test ! -e /boot/develop/lib/x86/libssl.so && test -e ~/config/lib/libssl.so; then \
		ln -s ~/config/lib/libssl.so /boot/develop/lib/x86/libssl.so; \
		ln -s ~/config/lib/libcrypto.so /boot/develop/lib/x86/libcrypto.so; \
	fi
	-if test ! -e /boot/develop/headers/openssl && test -e ~/config/include/openssl; then \
		ln -s ~/config/include/openssl/ /boot/develop/headers/openssl; \
	fi

	# ensure libbsvg.so is where im_emoclient needs it.
	-if test ! -e ~/config/lib/libbsvg.so; then \
		cp $(shell pwd)/sample_clients/im_emoclient/lib/libbsvg.so ~/config/lib; \
	fi
	-if test ! -e /boot/develop/lib/x86/libbsvg.so; then \
		ln -s ~/config/lib/libbsvg.so /boot/develop/lib/x86/libbsvg.so; \
	fi
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile $@ || exit -1; \
	done

clean:
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile clean || exit -1; \
	done

ICONDIR=/boot/home/config/settings/im_kit/icons
COMMON_LIB:=$(shell finddir B_COMMON_LIB_DIRECTORY)
COMMON_SERVERS:=$(shell finddir B_COMMON_SERVERS_DIRECTORY)
COMMON_ADDONS:=$(shell finddir B_COMMON_ADDONS_DIRECTORY)
BUILD:=$(shell pwd)/build

symlinks: common $(ICONDIR)
	ln -sf "$(BUILD)/lib/libim.so" "$(COMMON_LIB)"
	
	ln -sf "$(BUILD)/im_server"  "$(COMMON_SERVERS)"
	ln -sf "$(BUILD)/protocols" "$(COMMON_ADDONS)/im_kit"
	ln -sf "$(BUILD)/tracker-addons/IM_Merge_contacts" "$(COMMON_ADDONS)/Tracker"
	ln -sf "$(BUILD)/clients/im_client" "$(COMMON_ADDONS)/Tracker"
#does this create symlinks for the rest of $(BUILD)/clients/* ?
	ln -sf "$(BUILD)/settings/InstantMessaging" /boot/home/config/be/Preferences

install: common $(ICONDIR)
	copyattr --data --move "$(BUILD)/lib/libim.so" "$(COMMON_LIB)"
	rmdir "$(BUILD)/lib"

	copyattr --data --move "$(BUILD)/im_server"  "$(COMMON_SERVERS)"
	copyattr --data --recursive --move "$(BUILD)/protocols" "$(COMMON_ADDONS)/im_kit"
	copyattr --data --move "$(BUILD)/tracker-addons/IM_Merge_contacts" "$(COMMON_ADDONS)/Tracker"

	rmdir "$(BUILD)/tracker-addons"
	mkdir -p /boot/apps/im_kit

	copyattr --data --recursive --move "$(BUILD)/clients" /boot/apps/im_kit
	ln -sf "/boot/apps/im_kit/clients/im_client" "$(COMMON_ADDONS)/Tracker"
	copyattr --data --recursive --move "$(BUILD)/settings" /boot/apps/im_kit
	ln -sf "/boot/apps/im_kit/settings/InstantMessaging" /boot/home/config/be/Preferences
	copyattr --data --recursive --move "$(BUILD)/utils" /boot/apps/im_kit


common:
	# create indexes
	-mkindex -t string IM:connections
	-mkindex -t string IM:status	
	
	# add attributes to application/x-person
	-$(BUILD)/utils/mimetype_attribute --mime application/x-person --internal-name "IM:connections" --public-name "IM Connections" --type string --width 80 --viewable --public
	-$(BUILD)/utils/mimetype_attribute --mime application/x-person --internal-name "IM:status" --public-name "IM Status" --type string --width 80 --viewable --public --not-editable
	ln -sf "/boot/home/config/lib/libim.so" "/boot/develop/lib/x86/libim.so"

	-if [ ! -d "$(COMMON_SERVERS)" ]; then \
		mkdir -p "$(COMMON_SERVERS)"; \
	fi 

	rm -Rf "$(COMMON_ADDONS)/im_kit"
	mkdir -p "$(COMMON_ADDONS)/im_kit"


$(ICONDIR): server/Icons.zip
	# Unpack the Server icons.
	-mkdir -p "$(ICONDIR)" 2>/dev/null
	-unzip -o server/Icons.zip -d "$(ICONDIR)"
	touch "$(ICONDIR)"
	# Unpack im_emoclient icons.
	-mkdir -p  ~/config/settings/im_kit
	-unzip -o "$(shell pwd)/sample_clients/im_emoclient/smileys.zip" -d ~/config/settings/im_kit

dist: all
	mkdir -p "$(DIST)"
	copyattr --data --recursive build/* "$(DIST)"
	mkdir -p "$(DIST)/doc"
	copyattr --data --recursive build/* "$(DIST)/doc"
	zip -r "$(DIST).zip" "$(DIST)/"
	rm -Rf "$(DIST)"
	
/boot/home/config/include/libim:
	-mkdir -p $@

/boot/home/config/include/libim/%.h: libim/%.h
	cp -f $< $@	
