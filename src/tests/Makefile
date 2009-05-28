PROJECT = t.t tdb.t date.t duration.t t.benchmark.t text.t autocomplete.t      \
          parse.t seq.t att.t mod.t stringtable.t # record.t
CFLAGS  = -I. -I.. -Wall -pedantic -ggdb3 -fno-rtti
LFLAGS  = -L/usr/local/lib
OBJECTS = ../TDB.o ../T.o ../parse.o ../text.o ../Date.o ../Duration.o         \
          ../util.o ../Config.o ../Sequence.o ../Att.o ../Record.o ../Mod.o    \
          ../StringTable.o

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

seq.t: seq.t.o $(OBJECTS) test.o
	g++ seq.t.o $(OBJECTS) test.o $(LFLAGS) -o seq.t

record.t: record.t.o $(OBJECTS) test.o
	g++ record.t.o $(OBJECTS) test.o $(LFLAGS) -o record.t

mod.t: mod.t.o $(OBJECTS) test.o
	g++ mod.t.o $(OBJECTS) test.o $(LFLAGS) -o mod.t

att.t: att.t.o $(OBJECTS) test.o
	g++ att.t.o $(OBJECTS) test.o $(LFLAGS) -o att.t

stringtable.t: stringtable.t.o $(OBJECTS) test.o
	g++ stringtable.t.o $(OBJECTS) test.o $(LFLAGS) -o stringtable.t

