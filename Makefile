
# For any target, just recurse into each subdirectory
.DEFAULT:
	$(MAKE) -C man $(MFLAGS) $@
	$(MAKE) -C src $(MFLAGS) $@

all: .DEFAULT

# docs
doc: cppcheck doxygen flawfinder scan-build

cppcheck:
	cppcheck --std=c89 --enable=all --inline-suppr src 2> doc/cppcheck/results.txt

doxygen:
	doxygen doc/doxygen.conf

flawfinder:
	flawfinder src > doc/flawfinder/results.txt

scan-build:
	make clean
	scan-build -o doc/scan-build/ make
	figlet "Update scan-build link in README.md!"
