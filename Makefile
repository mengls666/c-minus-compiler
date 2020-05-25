cminus: scanner.o parser.o code.o symtab.o analyze.o gencode.o main.cpp
	@g++ scanner.o parser.o code.o symtab.o analyze.o gencode.o main.cpp -o cminus
scanner: scanner.cpp scanner.h
	@g++ scanner.cpp -c
parser: parser.cpp parser.h
	@g++ parser.cpp -c -w
code: code.cpp code.h
	@g++ code.cpp -c
symtab: symtab.cpp symtab.h
	@g++ symtab.cpp -c
analyze: analyze.cpp analyze.h
	@g++ analyze.cpp -c
gc: gencode.cpp gencode.h
	@g++ gencode.cpp -c
TM: TM.C
	@gcc TM.C -o tiny
clean:
	@rm  -f *.o scanner.txt Token_tree.txt