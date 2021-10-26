# makefile

all: myShell

myShell: myShell.cpp 
	g++ -g -w -std=c++11 -o run myShell.cpp 

clean:
	rm myShell