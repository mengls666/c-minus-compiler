parser: parser.cpp parser.h scanner.cpp scanner.h
	g++ parser.cpp scanner.cpp -o parser
TM: TM.C
	gcc TM.C -o tiny
