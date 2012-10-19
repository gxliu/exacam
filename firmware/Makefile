
SOURCES:=main.c
A51_SOURCES:=vectors.a51 dscr.a51 startup.a51 # vectors.a51 must ALWAYS go first! (TODO: check why)
EXTRA_CFLAGS:=-Ifx2
LIBS:=fx2/fx2.lib
EXTRA_LDFLAGS:=-L fx2 -I fx2
#EXTRA_CFLAGS:=

ifneq ($(RELEASE),1)
  EXTRA_CFLAGS += -DFIRMWARE_GIT_REVISION=\"$(shell git rev-parse HEAD)\"
endif

include fx2/fx2.mk

all: build

build: fx2/fx2.lib exacam.iic

fx2/fx2.lib:
	$(MAKE) -C fx2 fx2.lib

clean:
	$(MAKE) -C fx2 clean
	rm -f $(JUNK_FILES)
	
load: build
	sudo fx2load -v 0x04b4 -p 0x8613 exacam.iic

info:
	lsusb -d fffe:0100 -v 
