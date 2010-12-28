PROJECT = t.t tdb.t date.t duration.t t.benchmark.t text.t autocomplete.t      \
          config.t seq.t att.t stringtable.t record.t nibbler.t subst.t filt.t \
          cmd.t util.t color.t list.t path.t file.t directory.t grid.t rx.t    \
          taskmod.t sensor.t rectangle.t tree.t tree2.t lisp.t uri.t
CFLAGS  = -I. -I.. -I../src -Wall -pedantic -ggdb3 -fno-rtti
LFLAGS  = -L/usr/local/lib -lpthread -llua
OBJECTS = ../src/t-TDB.o ../src/t-Task.o ../src/t-text.o ../src/t-Date.o ../src/t-Table.o          \
          ../src/t-Duration.o ../src/t-util.o ../src/t-Config.o ../src/t-Sequence.o ../src/t-Att.o \
          ../src/t-Cmd.o ../src/t-Record.o ../src/t-StringTable.o ../src/t-Subst.o             \
          ../src/t-Nibbler.o ../src/t-Location.o ../src/t-Filter.o ../src/t-Context.o          \
          ../src/t-Keymap.o ../src/t-command.o ../src/t-interactive.o ../src/t-report.o        \
          ../src/t-Grid.o ../src/t-Color.o ../src/t-rules.o ../src/t-recur.o ../src/t-custom.o     \
          ../src/t-export.o ../src/t-import.o ../src/t-edit.o ../src/t-Timer.o                 \
          ../src/t-Permission.o ../src/t-Path.o ../src/t-File.o ../src/t-Directory.o           \
          ../src/t-Hooks.o ../src/t-API.o ../src/t-rx.o ../src/t-Taskmod.o ../src/t-dependency.o   \
          ../src/t-Transport.o ../src/t-TransportSSH.o ../src/t-Sensor.o ../src/t-Thread.o     \
          ../src/t-Lisp.o ../src/t-Rectangle.o ../src/t-Tree.o ../src/t-TransportRSYNC.o       \
          ../src/t-TransportCurl.o ../src/t-Uri.o ../src/t-diag.o ../src/t-burndown.o          \
          ../src/t-history.o

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

grid.t: grid.t.o $(OBJECTS) test.o
	g++ grid.t.o $(OBJECTS) test.o $(LFLAGS) -o grid.t

rx.t: rx.t.o $(OBJECTS) test.o
	g++ rx.t.o $(OBJECTS) test.o $(LFLAGS) -o rx.t

taskmod.t: taskmod.t.o $(OBJECTS) test.o
	g++ taskmod.t.o $(OBJECTS) test.o $(LFLAGS) -o taskmod.t

lisp.t: lisp.t.o $(OBJECTS) test.o
	g++ lisp.t.o $(OBJECTS) test.o $(LFLAGS) -o lisp.t

rectangle.t: rectangle.t.o $(OBJECTS) test.o
	g++ rectangle.t.o $(OBJECTS) test.o $(LFLAGS) -o rectangle.t

sensor.t: sensor.t.o $(OBJECTS) test.o
	g++ sensor.t.o $(OBJECTS) test.o $(LFLAGS) -o sensor.t

tree.t: tree.t.o $(OBJECTS) test.o
	g++ tree.t.o $(OBJECTS) test.o $(LFLAGS) -o tree.t

tree2.t: tree2.t.o $(OBJECTS) test.o
	g++ tree2.t.o $(OBJECTS) test.o $(LFLAGS) -o tree2.t

uri.t: uri.t.o $(OBJECTS) test.o
	g++ uri.t.o $(OBJECTS) test.o $(LFLAGS) -o uri.t

