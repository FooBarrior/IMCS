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

m+="$1: main.cpp\n\tg++ -o$bin -g *.cpp\n"
m+="clean:\n\trm $bin\n"

m+="commit:\n"
declare commit_path="/media/DATA/pile/projects/git/IMCS/cpp/$1"
m+="\tmkdir -p $commit_path"
for l in "*.cpp" "*.h" "Makefile"; do
	m+="\tcp $l $commit_path\n"
done

touch $f
echo -e $m > $f

