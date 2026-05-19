#!/bin/sh

if [ -z "$1" ];
then
    echo "Error: please specify file directory"
    exit 1
else
    writefile=$1
fi

if [ -z "$2" ];
then
    echo "Error: please specify string for writing"
    exit 1
else
    writestr=$2
fi


mkdir -p "${writefile%/*}"

if [ $? -eq 0 ]; then
    echo "Directory created successfully."
else
    echo "Failed to create directory."
fi

touch "$writefile"
echo $writestr > $writefile