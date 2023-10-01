IDIR = ../st7796s
CC = gcc
CFLAGS = -I$(IDIR)

ODIR = .
LDIR = ../st7796s

#LIBS = -lncurses -lbcm2835

_DEPS = st7796s_define.h rpi_dma_utils.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o rpi_dma_test.o rpi_dma_utils.o st7796s.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 