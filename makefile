IDIR =./include
CC=arm-linux-gnueabihf-gcc
CFLAGS=-I$(IDIR) -O3 -pthread

ODIR=obj
LDIR =../lib

_DEPS = circular_buffer.h definitions.h ipScan.h messageTools.h randString.h threads.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = router.o ipScan.o randString.o messageTools.o circular_buffer.o threads.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

routerPI: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
