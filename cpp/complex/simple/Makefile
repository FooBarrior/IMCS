binary = complex.bin
commit_path = /media/DATA/pile/projects/git/IMCS/cpp/complex/simple

$(binary): main.cpp
	g++ -o$(binary) -g *.cpp
clean:
	rm $(binary)
commit:
	mkdir -p $(commit_path)
	cp *.cpp $(commit_path)
	cp *.h $(commit_path)
	cp Makefile $(commit_path)

