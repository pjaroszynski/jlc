CXX = clang++
CXX = g++
CXXFLAGS = -O2
CXXFLAGS = -ggdb3
CXXFLAGS += -Wall
CXXFLAGS += -std=c++0x
CXXFLAGS += -I /home/peper/devel/boost-svn/
LDFLAGS = $(shell llvm-config --ldflags)
OBJS = jlc.o exception.o
GENERATED = jlc

all : jlc

%.o : %.cc $(wildcard *.hh)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

jlc : $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

.PRECIOUS : $(GENERATED)

clean :
	rm -rf $(OBJS) $(GENERATED)
