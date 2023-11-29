#!/bin/bash
clear
if [ -f "hw2" ]; then
    rm hw2
fi
echo "" > key_query_out.txt
echo "" > range_query_out.txt
make
./hw2
diff -s key_query_out.txt key_query_ans.txt
diff -s range_query_out.txt range_query_ans.txt
cat time_used.txt

which valgrind
if [ "$?" -ne 0 ]
then
    echo "valgrind command not found"
else
    valgrind --leak-check=full ./hw2
fi