#ifndef SPICIAL_STRUCTS_H
#define SPICIAL_STRUCTS_H

#include <string>
#include <vector>

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

struct Analysis {
    int analysis_type;  // OP, AC, DC, TRAN
    int source_type;  // for DC
    std::string source_name;  // for DC
    double step;  // for TRAN
    std::string sim_name;
    std::vector<double> sim_values;
};

struct Output {
    int output_type;  // PRINT, PLOT
    int analysis_type;  // OP, DC, AC, TRAN
    std::vector<Variable> var_list;
};

struct ColumnData {
    std::string name;
    std::vector<double> values;
};

#endif  // SPICIAL_STRUCTS_H
