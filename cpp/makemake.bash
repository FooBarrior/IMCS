#!/bin/bash
mkdir -p "$1";
declare bin;
if [[ $2 ]]; then
	bin=$2
else
	[[ "$1" =~ "\/?([^\/])*" ]]
	bin="${BASH_REMATCH[0]}.bin"
fi

declare f="$1/Makefile"
declare m

m+="binary = $bin\n"
m+="commit_path = /media/DATA/pile/projects/git/IMCS/cpp/$1\n\n"
declare binary="\$(binary)"
declare commit_path="\$(commit_path)"

m+="$binary: main.cpp\n\tg++ -o$binary -g *.cpp\n"
m+="clean:\n\trm $binary\n"

m+="commit:\n"
m+="\tmkdir -p $commit_path\n"
for l in "*.cpp" "*.h" "Makefile"; do
	m+="\tcp $l $commit_path\n"
done

echo -e $m > $f

