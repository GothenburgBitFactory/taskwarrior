PROJECT = t.t tdb.t date.t duration.t t.benchmark.t text.t autocomplete.t \
          parse.t
CFLAGS  = -I. -I.. -Wall -pedantic -ggdb3 -fno-rtti
LFLAGS  = -L/usr/local/lib
OBJECTS = ../TDB.o ../T.o ../parse.o ../text.o ../Date.o ../Duration.o \
          ../util.o ../Config.o

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

t.benchmark.t: t.benchmark.t.o $(OBJECTS) test.o
	g++ t.benchmark.t.o $(OBJECTS) test.o $(LFLAGS) -o t.benchmark.t

text.t: text.t.o $(OBJECTS) test.o
	g++ text.t.o $(OBJECTS) test.o $(LFLAGS) -o text.t

autocomplete.t: autocomplete.t.o $(OBJECTS) test.o
	g++ autocomplete.t.o $(OBJECTS) test.o $(LFLAGS) -o autocomplete.t

parse.t: parse.t.o $(OBJECTS) test.o
	g++ parse.t.o $(OBJECTS) test.o $(LFLAGS) -o parse.t

