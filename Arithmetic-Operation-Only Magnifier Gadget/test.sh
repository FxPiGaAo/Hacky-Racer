#!/bin/bash

cmd="./arithmetic"
rm -f ./arithmetic.c
cp ./arithmetic_source.c ./arithmetic.c

succeed=0
for i in {1..30}
do
    sed -i '158 a "imull %%ebx, %2\\n\\t"' ./arithmetic.c
    make
    $cmd
    status=$?
    if [[ $status -eq 0 ]]
    then
        succeed=1
        break
    fi
done

if [[ $succeed -eq 0 ]]
then
    echo "Cannot find a suitable path length. This arithmetic-only magnifier gadget failed......"
fi
