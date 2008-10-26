# makefile for all sample code

SUBDIRS = \
	AIM \
	MSN \
	Jabber \
	GoogleTalk \
	ICQ \
	Yahoo

default .DEFAULT :
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile $@ || exit -1; \
	done

clean:
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile clean || exit -1; \
	done
