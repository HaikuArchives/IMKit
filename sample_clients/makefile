# makefile for all sample code

SUBDIRS = \
	logger \
	tracker_addons \
	im_infopopper \
	im_binlogger \
	im_binlog_viewer \
	im_emoclient \
	im_client

default .DEFAULT :
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile $@ || exit -1; \
	done

clean:
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile clean || exit -1; \
	done
