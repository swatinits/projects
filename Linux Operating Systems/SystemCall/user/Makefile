#
# Makefile for sample
#

CFLAGS = -O3
LDFLAGS = -static -s

BINDIR = /bin
MANDIR = /usr/man/man1


OBJS = sample.o 


ourshell:	$(OBJS)
	cc $(LDFLAGS) -o sample $(OBJS)

clean:
	rm -f $(OBJS) sample

install: test
	cp sample $(BINDIR)/sample
	cp sample.1 $(MANDIR)/sample.1

$(OBJS):	prinfo.h
