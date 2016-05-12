SUBDIRS := zk_lib utility

all:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $$d all; \
	done

clean:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $$d clean; \
	done
