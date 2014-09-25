CXX=g++
CFLAGS=-O -Wall -Wno-unused
SRC_FILES=Element.cpp Node.cpp Parser.cpp String.cpp Text.cpp Visitor.cpp parser_test.cpp
.PHONY: all

all: parser_test gen

.PHONY: gen

gen: XmlGenerator.cpp gen.cpp
	$(CXX) $(CFLAGS) -o gen XmlGenerator.cpp gen.cpp

.PHONY: parser_test

parser_test: $(SRC_FILES:%.cpp=%.o)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp %.hpp
	$(CXX) $(CFLAGS) -c $<

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<

clean:
	rm -rf *.o parser_test gen