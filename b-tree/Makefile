sources = main.c linear_sequence_assoc.h tree.c test.c
binary = tree
repo = /media/DATA/pile/projects/git/IMCS/

$(binary): $(sources)
	gcc -o $(binary) -g3 $(sources)
clean:
	rm $(binary)
copy:
	cp $(sources) Makefile $(repo)