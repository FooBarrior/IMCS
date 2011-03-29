#!/bin/bash
mkdir -p "$1";
declare bin;
if [[ $2 ]]; then
	bin=$2
else
	[[ "$1" =~ \/?([^\/])* ]]
	bin="${BASH_REMATCH[0]}.bin"
fi

declare f=$1"/Makefile"
declare m

m+="$1: main.c\n\tg++ -o$bin -g *.c\n"
m+="clean:\n\trm $bin\n"
m+="commit:\n"
for l in "*.c" "*.h" "Makefile"; do
	m+="\tcp $l /media/DATA/pile/projects/git/IMCS/cpp/$1\n"
done

touch $f
echo -e $m > $f

declare main=$1/main.c
touch $main
echo -e "\n\n\nint main(){\n\n}" > $main

