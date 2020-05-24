analyzer: analyze.cpp analyze.h code.cpp code.h gencode.cpp gencode.h parser.cpp parser.h scanner.cpp scanner.h symtab.cpp symtab.h
	g++ analyze.cpp code.cpp gencode.cpp scanner.cpp parser.cpp symtab.cpp -o analyzer
TM: TM.C
	gcc TM.C -o tiny
clear:
	rm  tmp.txt Token_tree.txt listing.txt code.txt scanner.txt