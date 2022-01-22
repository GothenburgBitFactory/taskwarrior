CXX=g++
INC=-I ../lib
LIB=-L ../target/debug
RPATH=-Wl,-rpath,../target/debug 

TESTS = uuid.cpp

.PHONY: all test

all: test

test: doctest
	@./doctest --no-version --no-intro

%.o: %.cpp ../lib/taskchampion.h
	$(CXX) $(INC) -c $< -o $@

doctest: doctest.o $(subst .cpp,.o,$(TESTS))
	$(CXX) $(LIB) $(RPATH) $< $(subst .cpp,.o,$(TESTS)) -ltaskchampion -o $@
