#include "gencode.h"
#include "cmdline.h"
extern FILE *listing;
extern FILE *code;
extern bool TraceCode; //print trace information in the tm file
int main(int argc,char *argv[]) {
    cmdline::parser a;
    a.add<string>("output", 'o', "filename of generated .tm file", true, "");
    a.add<string>("input", 'i', "filename of your c-minus source code", true, "");
    a.add("trace", 't', "print the trace information");
    TraceCode = !a.exist("trace");
    a.parse_check(argc, argv);
	Parser P(a.get<string>("input"));
    listing = stdout;
    code = fopen(a.get<string>("output").c_str(),"w");
	buildTable(P.synTree);
	typeCheck(P.synTree);
    gentiny(P.synTree);
    cout << "Successfully generated. The code is saved to " << a.get<string>("output") << endl;
    fclose(code);
}