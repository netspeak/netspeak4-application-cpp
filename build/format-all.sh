#!/bin/bash

cd "$(dirname "$0")"
cd ..

clang-format -i ./src/*.[ch]pp
clang-format -i ./src/antlr4/*.[ch]pp
clang-format -i ./src/cli/*.[ch]pp
clang-format -i ./src/netspeak/*.[ch]pp
clang-format -i ./src/netspeak/bighashmap/*.[ch]pp
clang-format -i ./src/netspeak/invertedindex/*.[ch]pp
clang-format -i ./src/netspeak/model/*.[ch]pp
clang-format -i ./src/netspeak/regex/*.[ch]pp
clang-format -i ./src/netspeak/util/*.[ch]pp
clang-format -i ./src/netspeak/value/*.[ch]pp

clang-format -i ./test/netspeak/*.[ch]pp
clang-format -i ./test/netspeak/bighashmap/*.[ch]pp
clang-format -i ./test/netspeak/invertedindex/*.[ch]pp
