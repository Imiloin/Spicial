#ifndef SPICIAL_STRUCTS_H
#define SPICIAL_STRUCTS_H

#include <vector>
#include <string>

struct Variable {
    int type;
    std::vector<std::string> nodes;
};

struct Option {
    int type;
    double value;
};

struct Function {
    int type;
    std::vector<double> values;
};

#endif // SPICIAL_STRUCTS_H
