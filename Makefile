CC=gcc
CFLAGS=-std=c99 -g -Wall
LIBS=
TARGET=server
SRCDIR=src
OBJDIR=obj

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) $(OBJDIR)/hash.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) -pthread

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/hash.o: $(SRCDIR)/hash.c $(SRCDIR)/hash.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/hash.c

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(OBJS) $(OBJDIR)/hash.o $(TARGET)

.PHONY: clean
