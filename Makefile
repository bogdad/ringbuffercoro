buildall:
	echo building
	cd build && ninja && ctest --verbose
