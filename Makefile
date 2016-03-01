
# For any target, just recurse into each subdirectory
.DEFAULT:
	$(MAKE) -C man $(MFLAGS) $@
	$(MAKE) -C src $(MFLAGS) $@

all: .DEFAULT

# docs

doxygen:
	doxygen doxygen.conf

report.scan-build:
	make clean
	scan-build -o $@ make
	figlet "Update scan-build link in README.md!"
