all:
	g++ -std=c++17 -Wall main.cpp -o project
	./project

clean:
	rm -f project

