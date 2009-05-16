PROJECT = 1.8
CFLAGS  = -I. -I../../library/include -Wall -pedantic -ggdb3 -fno-rtti -fstack-check
LFLAGS  = 
LIBS    = 
OBJECTS = main.o Context.o TDB.o T.o Sequence.o Filter.o Att.o Date.o Duration.o Keymap.o

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

