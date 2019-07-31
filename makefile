IDIR =./include
CC=arm-linux-gnueabihf-gcc
CFLAGS=-I$(IDIR) -O3 -pthread

ODIR=obj
SRCDIR=./src
LDIR =../lib

_DEPS = circular_buffer.h definitions.h ipScan.h messageTools.h randString.h threads.h saveTools.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = router.o ipScan.o randString.o messageTools.o circular_buffer.o threads.o saveTools.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

routerPI.out: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
