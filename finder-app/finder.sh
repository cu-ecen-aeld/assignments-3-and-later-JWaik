#!/bin/sh

if [ -z "$1" ];
then
    echo "Error: please specify file directory"
    exit 1
else
    filesdir=$1
fi

if [ -z "$2" ];
then
    echo "Error: please specify string for searching"
    exit 1
else
    searchstr=$2
fi
echo ${filecount}
filecount=$(find ${filesdir} -type f | wc -l)
strline=$(grep -r ${searchstr} ${filesdir} | wc -l)

# echo ${strline}
echo "The number of files are ${filecount} and the number of matching lines are ${strline}"

exit 0