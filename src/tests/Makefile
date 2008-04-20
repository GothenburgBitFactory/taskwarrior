PROJECT = t.t tdb.t
CFLAGS  = -I.. -I../../../library/include -Wall -pedantic -ggdb3 -fno-rtti
LFLAGS  = -L/usr/local/lib -L../../../library/lib -lcompany -lpcre -lncurses -lcurl

all: $(PROJECT)

install: $(PROJECT)
	@echo unimplemented

test: $(PROJECT)
	@echo unimplemented

clean:
	-rm *.o $(PROJECT)

.cpp.o:
	g++ -c $(CFLAGS) $<

t.t: t.t.o ../T.o ../parse.o ../../../library/lib/libcompany.a
	g++ t.t.o ../T.o ../parse.o $(LFLAGS) -o t.t

tdb.t: tdb.t.o ../TDB.o ../T.o ../parse.o ../../../library/lib/libcompany.a
	g++ tdb.t.o ../TDB.o ../T.o ../parse.o $(LFLAGS) -o tdb.t

