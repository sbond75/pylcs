prefix     = /usr/local
includedir = $(prefix)/include
libdir     = $(prefix)/lib
mandir     = $(prefix)/man
CC         = gcc
LIBNAME    = mba
MAJVERSION = 0.8
MINVERSION = 0.8.10
ARNAME     = lib$(LIBNAME).a
SONAME     = lib$(LIBNAME).so.$(MINVERSION)
SOVERSION  = lib$(LIBNAME).so.$(MAJVERSION)
DISTRO     = lib$(LIBNAME)-$(MINVERSION)
RPM_OPT_FLAGS = -O2

# For wide character support compile with -DUSE_WCHAR -D_XOPEN_SOURCE=500

CFLAGS     = -Wall -W -ansi -DMSGNO -D_XOPEN_SOURCE=500 -Isrc -I$(includedir) -L$(libdir) $(RPM_OPT_FLAGS)
#CFLAGS     = -Wall -W -g -ansi -pedantic -Wcast-qual -Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes -Wno-parentheses -DMSGNO -D_XOPEN_SOURCE=500 -Isrc -I$(includedir) -L$(libdir) $(RPM_OPT_FLAGS)
OBJS       = src/stack.o src/linkedlist.o src/hashmap.o src/hexdump.o src/msgno.o src/cfg.o src/pool.o src/varray.o src/shellout.o src/csv.o src/path.o src/misc.o src/text.o src/eval.o src/svsem.o src/allocator.o src/suba.o src/time.o src/bitset.o src/svcond.o src/daemon.o src/diff.o
HDRS       = src/mba/msgno.h src/mba/stack.h src/mba/linkedlist.h src/mba/hashmap.h src/mba/hexdump.h src/mba/cfg.h src/mba/pool.h src/mba/varray.h src/mba/shellout.h src/mba/csv.h src/mba/iterator.h src/mba/text.h src/mba/path.h src/mba/eval.h src/mba/svsem.h src/mba/allocator.h src/mba/suba.h src/mba/time.h src/mba/bitset.h src/mba/svcond.h src/mba/daemon.h src/mba/diff.h src/mba/misc.h
MAN        = diff.3m.gz bitset.3m.gz allocator.3m.gz cfg.3m.gz pool.3m.gz varray.3m.gz csv.3m.gz text.3m.gz path.3m.gz suba.3m.gz msgno.3m.gz stack.3m.gz linkedlist.3m.gz hashmap.3m.gz hexdump.3m.gz shellout.3m.gz eval.3m.gz svsem.3m.gz svcond.3m.gz time.3m.gz

all: $(ARNAME)($(OBJS)) $(SONAME)
so: $(SONAME)

$(SONAME): $(OBJS)
	$(CC) -shared $(OBJS) -L$(libdir) -lc -lutil -Wl,-h,$(SOVERSION) -o $(SONAME)

.c.a:
	$(CC) $(CFLAGS) -c $< -o $*.o
	ar rv $@ $*.o
	rm $*.o

.c.o:
	$(CC) $(CFLAGS) -fpic -c -o $*.o $<

install: $(SONAME)
	install -d $(libdir)
	install -d $(includedir)/mba
	install -d $(mandir)/man3
	-install -m 644 $(ARNAME) $(libdir)
	install -m 755 $(SONAME) $(libdir)
	cd $(libdir) && ln -sf $(SONAME) $(SOVERSION) && ln -sf $(SONAME) lib$(LIBNAME).so
	install -m 444 $(HDRS) $(includedir)/mba
	-install -m 444 docs/man/*.3m.gz $(mandir)/man3
	-/sbin/ldconfig $(libdir)

zipold:
	-rm docs/www/dl
	cd .. && zip -lr $(DISTRO)/.$(DISTRO).zip $(DISTRO) -x $(DISTRO)/.* $(DISTRO)/docs/man/* $(DISTRO)/tcase/tests/*.dat $(DISTRO)/libmba.lib $(DISTRO)/libmba.dll $(DISTRO)/libmba_s.lib
	cd .. && zip -ur $(DISTRO)/.$(DISTRO).zip $(DISTRO) -x $(DISTRO)/.* $(DISTRO)/docs/man/*
	mv .$(DISTRO).zip $(DISTRO).zip

zip:
	rm -rf /tmp/$(DISTRO) /tmp/$(DISTRO).zip
	cp -a . /tmp/$(DISTRO)
	cd /tmp/$(DISTRO) && crlf * docs/* docs/xml/* docs/www/* docs/ref/* examples/* examples/msgno/* examples/diff/* src/* src/mba/* tcase/* tcase/tests/* tcase/tests/data/*
	cd /tmp && zip -r $(DISTRO).zip $(DISTRO) -x "*/.*"

clean:
	rm -rf $(includedir)/mba
	rm -f $(OBJS) $(ARNAME) $(SONAME) $(libdir)/$(ARNAME) $(libdir)/$(SONAME) $(libdir)/$(SOVERSION) $(libdir)/lib$(LIBNAME).so $(DISTRO).zip
	cd $(mandir)/man3 && rm -f $(MAN)

sho: src/shellout.c src/mba/shellout.h
	gcc -Wall -W -DTEST -DMSGNO -lmba -lutil -o sho src/shellout.c

print:
	a2ps -l110 -o code.ps code.txt && gv code.ps

