CXX=g++
INC=-I ../lib
LIB=-L ../target/debug
RPATH=-Wl,-rpath,../target/debug 

TESTS = replica.cpp string.cpp uuid.cpp task.cpp

.PHONY: all test

all: test

test: doctest
	@rm -rf test-db
	@./doctest --no-version --no-intro
	@rm -rf test-db

%.o: %.cpp ../lib/taskchampion.h
	$(CXX) $(INC) -c $< -o $@

doctest: doctest.o $(subst .cpp,.o,$(TESTS))
	$(CXX) $(LIB) $(RPATH) $< $(subst .cpp,.o,$(TESTS)) -ltaskchampion -o $@
