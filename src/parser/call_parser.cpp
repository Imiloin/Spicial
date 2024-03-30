#include "call_parser.h"
#include <QDebug>

extern FILE* yyin;

Circuit *callNetlistParser(const char* fileName) {
    qDebug() << "callNetlistParser()" << fileName;

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
    char title[128];
    fgets(title, 128, yyin);
    printf("Title: %s", title);

    // create circuit instance
    Circuit *circuit = new Circuit;

    yy::Parser parser(circuit);
    int ret = parser.parse();
    if (ret != 0) {
        printf("Parse %s failed.\n", fileName);
        return nullptr;
    } else {
        std::cout << std::endl
                  << "Parse " << fileName << " successfully." << std::endl;
    }

    fclose(yyin);

    return circuit;
}
