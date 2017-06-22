.PHONY: linux windows clean

linux:
	make -f Makefile.linux

windows:
	make -f Makefile.windows

clean:
	make -f Makefile.linux clean
	make -f Makefile.windows clean
	