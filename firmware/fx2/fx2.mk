JUNK_FILES:=*.sym *.asm *.rel *.lib *.hex *.bix *.iic *.map *.mem *.lst *.lk *.rst

COMPAT_FLAGS:=-Dxdata=__xdata -D_asm=__asm -D_endasm=__endasm -Dbit=__bit -Dinterrupt=__interrupt -Dat=__at -Dsbit=__sbit
CFLAGS:=-mmcs51 --no-xinit-opt $(EXTRA_CFLAGS) $(COMPAT_FLAGS) 

CC=sdcc
AS=sdas8051
ASFLAGS+=-plosgff

CODE_SIZES:=--code-loc 0x0000 --code-size 0x1800 --xram-loc 0x1800 --xram-size 0x0800 
LDFLAGS:=$(CODE_SIZES) -Wl '-b USBDESCSEG = 0xe100'  $(EXTRA_LDFLAGS)

REL_SOURCES:=$(A51_SOURCES:.a51=.rel) $(SOURCES:.c=.rel) 

.PRECIOUS: %.rel
%.rel : %.a51
	$(AS) $(ASFLAGS) $<
	
%.rel : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.iic : %.hex
	objcopy -I ihex -O binary $< $@

%.lib: $(REL_SOURCES)
	rm -f $@
	touch $@
	for obj in $^ ; do basename $$obj .rel >> $@ ; done
	
%.hex: $(REL_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(LIBS) $+ 
	packihx $@ > .tmp.hex
	rm $@
	mv .tmp.hex $@
	
#%.hex: $(REL_SOURCES) $(LIBS)
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $+
# TODO: necessary?
#	packihx $@ > .tmp.hex
#	rm $@
#	mv .tmp.hex $@
