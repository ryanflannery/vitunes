
# For any target, just recurse into each subdirectory
.DEFAULT:
	$(MAKE) -C man $(MFLAGS) $@
	$(MAKE) -C src $(MFLAGS) $@

all: .DEFAULT
