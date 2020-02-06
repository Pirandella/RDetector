CC := gcc
TARGET := RDetector
CFLAGS = -std=c99 -I $(IDIR) -Wall -g3

ODIR := ./obj
SDIR := ./src
IDIR := ./inc

_DEPS := argParser.h panTompkins.h hcChen.h struct.h
DEPS := $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ := main.o argParser.o panTompkins.o hcChen.o
OBJ := $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(TARGET)
