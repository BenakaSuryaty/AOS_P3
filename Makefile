CC=gcc
CFLAGS=-std=c99 -g -Wall -D_GNU_SOURCE $(shell pkg-config --cflags glib-2.0)
LIBS=$(shell pkg-config --libs glib-2.0)
TARGET=server
SRCDIR=src
OBJDIR=obj

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean