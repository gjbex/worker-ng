CXX = g++
CXXFLAGS = -std=c++14 -g -O2 -Wall -Wextra
CPPFLAGS = -I/usr/local/software/cppzmq/4.2.3/include -MMD -MP
LDFLAGS = -L/usr/local/software/libzmq/4.2.5/lib
LDLIBS = -lzmq

.PRECIOUS: *.o

all: server.exe client.exe parser_test.exe

parser_test.exe: work_parser.o parser_test.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

%.exe: %.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS)

-include $(wildcard *.d)

clean:
	$(RM) $(wildcard *.exe) $(wildcard *.o) $(wildcard *.d)
	$(RM) core $(wildcard core.*)
