#include "call_parser.h"
#include <QDebug>

extern FILE* yyin;

Netlist *callNetlistParser(const char* fileName) {
    std::cout << "Start parsing " << fileName << std::endl << std::endl;

    // Check if fileName is a directory
    struct stat s;
    if (stat(fileName, &s) == 0 && S_ISDIR(s.st_mode)) {
        printf("%s is a directory, not a netlistfile. Please select a valid file to parse.\n", fileName);
        return nullptr;
    }
    
    yyin = fopen(fileName, "r");
    if (!yyin) {
        printf("Can not open %s.\n", fileName);
        return nullptr;
    }

    yyrestart(yyin);

    // read title and move the pointer to second line
    char ch_title[128];
    fgets(ch_title, 128, yyin);
    std::string title(ch_title);
    title.erase(title.find_last_not_of("\n\r") + 1);  // 去除末尾的换行符
    printf("[Title] %s\n", title.c_str());

    // create netlist instance
    Netlist *netlist = new Netlist(fileName, title);

    yy::Parser parser(netlist);
    int ret = parser.parse();
    if (ret != 0) {
        printf("Parse %s failed.\n", fileName);
        return nullptr;
    } else {
        std::cout << std::endl
                  << "Parse " << fileName << " successfully." << std::endl;
    }

    fclose(yyin);

    return netlist;
}
