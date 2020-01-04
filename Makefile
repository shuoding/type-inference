repl : repl.cpp
	g++ -std=c++11 -o repl repl.cpp

.PHONY : clean
clean :
	-rm repl
