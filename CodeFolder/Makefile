
#CHANGE TO CLANG FOR SHERINES WEIRD REQUIREMENTS!!
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2


INPUT = "A Scandal In Bohemia.txt"
OUTPUT = output.txt

SRCS = FinalAssignment.cpp
HDRS = HashTable.h ChainingHash.h ProbingHash.h

.PHONY: all clean run

all: FinalAssignment

FinalAssignment: $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $@


run: all
	@echo "Running program with default files..."
	@./FinalAssignment $(INPUT) $(OUTPUT)
	@echo "Output written to $(OUTPUT)"

clean:
	rm -f FinalAssignment $(OUTPUT)
