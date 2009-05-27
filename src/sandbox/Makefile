PROJECT = 1.8
CFLAGS  = -I. -I.. -Wall -pedantic -ggdb3 -fno-rtti -fno-stack-check
LFLAGS  = 
LIBS    = 
OBJECTS = main.o Context.o TDB.o T.o ../Sequence.o ../Filter.o ../Att.o      \
          Keymap.o ../Record.o ../Mod.o ../StringTable.o ../util.o ../text.o \
          ../Date.o ../Config.o ../Location.o

all: $(PROJECT)

install: $(PROJECT)
	@echo unimplemented

test: $(PROJECT)
	@echo unimplemented

clean:
	-rm *.o $(PROJECT)

.cpp.o: $(INCLUDE)
	g++ -c $(CFLAGS) $<

$(PROJECT): $(OBJECTS)
	g++ $(OBJECTS) $(LFLAGS) $(LIBS) -o $(PROJECT)

