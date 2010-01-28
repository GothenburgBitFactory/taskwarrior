PROJECT = t.t tdb.t date.t duration.t t.benchmark.t text.t autocomplete.t      \
          config.t seq.t att.t stringtable.t record.t nibbler.t subst.t filt.t \
          cmd.t util.t color.t list.t path.t file.t directory.t
CFLAGS  = -I. -I.. -I../.. -Wall -pedantic -ggdb3 -fno-rtti
LFLAGS  = -L/usr/local/lib -lncurses
OBJECTS = ../t-TDB.o ../t-Task.o ../t-text.o ../t-Date.o ../t-Table.o          \
          ../t-Duration.o ../t-util.o ../t-Config.o ../t-Sequence.o ../t-Att.o \
          ../t-Cmd.o ../t-Record.o ../t-StringTable.o ../t-Subst.o             \
          ../t-Nibbler.o ../t-Location.o ../t-Filter.o ../t-Context.o          \
          ../t-Keymap.o ../t-command.o ../t-interactive.o ../t-report.o        \
          ../t-Grid.o ../t-Color.o ../t-rules.o ../t-recur.o ../t-custom.o     \
          ../t-import.o ../t-edit.o ../t-Timer.o ../t-Permission.o ../t-Path.o \
          ../t-File.o ../t-Directory.o ../t-Hooks.o ../t-API.o

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

seq.t: seq.t.o $(OBJECTS) test.o
	g++ seq.t.o $(OBJECTS) test.o $(LFLAGS) -o seq.t

record.t: record.t.o $(OBJECTS) test.o
	g++ record.t.o $(OBJECTS) test.o $(LFLAGS) -o record.t

att.t: att.t.o $(OBJECTS) test.o
	g++ att.t.o $(OBJECTS) test.o $(LFLAGS) -o att.t

stringtable.t: stringtable.t.o $(OBJECTS) test.o
	g++ stringtable.t.o $(OBJECTS) test.o $(LFLAGS) -o stringtable.t

subst.t: subst.t.o $(OBJECTS) test.o
	g++ subst.t.o $(OBJECTS) test.o $(LFLAGS) -o subst.t

nibbler.t: nibbler.t.o $(OBJECTS) test.o
	g++ nibbler.t.o $(OBJECTS) test.o $(LFLAGS) -o nibbler.t

filt.t: filt.t.o $(OBJECTS) test.o
	g++ filt.t.o $(OBJECTS) test.o $(LFLAGS) -o filt.t

cmd.t: cmd.t.o $(OBJECTS) test.o
	g++ cmd.t.o $(OBJECTS) test.o $(LFLAGS) -o cmd.t

config.t: config.t.o $(OBJECTS) test.o
	g++ config.t.o $(OBJECTS) test.o $(LFLAGS) -o config.t

util.t: util.t.o $(OBJECTS) test.o
	g++ util.t.o $(OBJECTS) test.o $(LFLAGS) -o util.t

color.t: color.t.o $(OBJECTS) test.o
	g++ color.t.o $(OBJECTS) test.o $(LFLAGS) -o color.t

list.t: list.t.o $(OBJECTS) test.o
	g++ list.t.o $(OBJECTS) test.o $(LFLAGS) -o list.t

path.t: path.t.o $(OBJECTS) test.o
	g++ path.t.o $(OBJECTS) test.o $(LFLAGS) -o path.t

file.t: file.t.o $(OBJECTS) test.o
	g++ file.t.o $(OBJECTS) test.o $(LFLAGS) -o file.t

directory.t: directory.t.o $(OBJECTS) test.o
	g++ directory.t.o $(OBJECTS) test.o $(LFLAGS) -o directory.t

