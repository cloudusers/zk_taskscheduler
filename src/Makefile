SUBDIRS := sche base

all:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $$d all; \
	done

clean:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $$d clean; \
	done
