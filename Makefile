CFLAGS  = -I /usr/include/json-c/ -I include/
LDFLAGS = -ljson-c
BINDIR = /usr/bin/
CONFDIR = $(HOME)/.config/bddi
CONFFILE = icons.list
CC = gcc
SOURCES = $(wildcard src/*.c) $(wildcard include/*.h)

bddi: $(SOURCES)
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@

clean: 
	-rm -rf bddi *.gch

install: bddi 
	install -m 557 $? $(BINDIR)
	mkdir -p $(CONFDIR)
	touch $(CONFDIR)/$(CONFFILE)
