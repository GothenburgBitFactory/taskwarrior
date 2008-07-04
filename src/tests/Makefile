PROJECT = t.t tdb.t date.t duration.t
CFLAGS  = -I. -I.. -Wall -pedantic -ggdb3 -fno-rtti
LFLAGS  = -L/usr/local/lib
OBJECTS = ../TDB.o ../T.o ../parse.o ../text.o ../Date.o ../util.o ../Config.o

all: $(PROJECT)

install: $(PROJECT)
	@echo unimplemented

test: $(PROJECT)
	@echo unimplemented

clean:
	-rm *.o $(PROJECT)

.cpp.o:
	g++ -c $(CFLAGS) $<

t.t: t.t.o $(OBJECTS) test.o
	g++ t.t.o $(OBJECTS) test.o $(LFLAGS) -o t.t

tdb.t: tdb.t.o $(OBJECTS) test.o
	g++ tdb.t.o $(OBJECTS) test.o $(LFLAGS) -o tdb.t

date.t: date.t.o $(OBJECTS) test.o
	g++ date.t.o $(OBJECTS) test.o $(LFLAGS) -o date.t

duration.t: duration.t.o $(OBJECTS) test.o
	g++ duration.t.o $(OBJECTS) test.o $(LFLAGS) -o duration.t

