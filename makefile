all: task1 task2

task1: task1.cpp Utils.h Utils.cpp
	g++ -Wall -Werror -std=c++14 -O -o task1 task1.cpp Utils.cpp

task2: task2.cpp Utils.h Utils.cpp
	g++ -Wall -Werror -std=c++14 -O -o task2 task2.cpp Utils.cpp

clean:
	rm task1 task2

#  vim: set ts=4 sw=0 tw=79 fdm=manual ff=unix fenc=utf-8 noet :
