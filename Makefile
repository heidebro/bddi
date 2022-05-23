CFLAGS  = -I /usr/include/json-c/ -I include/
LDFLAGS = -ljson-c
BINDIR = /usr/bin/
CONFDIR = $(HOME)/.config/bddi
CC = gcc
SOURCES = $(wildcard src/*.c) $(wildcard include/*.h)

bddi: $(SOURCES)
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@

icons.list:
	mkdir -p $(CONFDIR)
	touch $(CONFDIR)/$@ 

clean: 
	-rm -rf bddi *.gch

install: bddi icons.list
	install -m 557 $? $(BINDIR)
