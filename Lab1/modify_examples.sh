#!/bin/sh

#creating directories

mkdir "example"
cd "example"
#create 3 folders
mkdir "a" "b" "c"
#[parent folder name].[level].[folder number]
#level 1
echo "[parent folder name].[level].[folder number]"

mkdir "a/a.1.1" "a/a.1.2"
touch "a/a.1.1.txt"

mkdir "b/b.1.1"
touch "b/b.1.1.txt" "b/b.1.2.txt"

mkdir "c/c.1.1" "c/c.1.2" "c/c.1.3"

#level 2
mkdir "a/a.1.1/a.2.1"
touch "a/a.1.1/a.2.1.txt"

mkdir "b/b.1.1/b.2.1" "b/b.1.1/b.2.2"

mkdir "c/c.1.2/c.2.1"
touch "c/c.1.1/c.2.1.txt"

#level 3
touch "a/a.1.1/a.2.1/a.3.1.txt"

mkdir "b/b.1.1/b.2.2/b.3.1"

ls -R

echo "--------------------------"
echo "examples of correct usage:"
echo "--------------------------"

# -h
echo "modify -h"
../modify.sh -h

# -u
echo "--------------------------"
echo "modify -u a b"
../modify.sh -u a b
ls -R

# -l
echo "--------------------------"
echo "modify -l A B"
../modify.sh -l A 
ls -R

# -r -u
echo "--------------------------"
echo "modify -r -u a B c"
../modify.sh -r -u a B c
ls -R

# -r -l
echo "--------------------------"
echo "modify -r -l A B C"
../modify.sh -r -l A B C
ls -R

# -r -sed parameter
echo "--------------------------"
echo "modify -r 's/c/x/' a b c"
../modify.sh -r 's/c/x/' a b c
ls -R

# incorrect scenarios
echo "--------------------------"
echo "incorrect scenarios:"
echo "--------------------------"

# no directory
echo "modify -r -l"
../modify.sh -r -l
ls

# no parameter
echo "--------------------------"
echo "modify a b"
../modify.sh a b
ls

# incorrect parameter
echo "--------------------------"
echo "modify -a a b"
../modify.sh -a a b
ls

# incorrect directory name
echo "--------------------------"
echo "modify -u d"
../modify.sh -u d
ls

# more the one parameter
echo "--------------------------"
echo "modify -r -l -u a b"
../modify.sh -r -l -u a b
ls

# same directory twice
echo "--------------------------"
echo "modify -r -u a a"
../modify.sh -r -u a a
ls

# incorrect syntax
echo "--------------------------"
echo "modify -r- u a"
../modify.sh -r- u a
ls

# tests done, remove examples directory
cd ..
rm -r example
