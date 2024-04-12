#ifndef SPICIAL_CIRCUIT_H
#define SPICIAL_CIRCUIT_H

// #include <algorithm>
// #include <complex>
#include <iostream>
#include <string>
#include <vector>
#include "Branches.h"
#include "Netlist.h"
#include "Nodes.h"
#include "Simulation.h"
#include "call_plot.h"
#include "function.h"
#include "linetype.h"
#include "structs.h"
#include "tokentype.h"

class Circuit {
   public:
    Circuit(Netlist& netlist_);
    ~Circuit();

    void preProcess();  // 生成 nodes 和 branches，同时为 components
                        // 分配 nodes, branches 的索引
                        // 分配 model 的指针

    void printNodes();
    void printBranches();
    
    Component* getComponentPtr(const std::string& name);

    Model* getModelPtr(const std::string& name);

    void runSimulations();

    void printAnalysis(int analysis_type,
                       const std::vector<Variable>& var_list);
    void plotAnalysis(int analysis_type, const std::vector<Variable>& var_list);
    // 其他方法...
    void printResults() const;

   private:
    Netlist& netlist;

    // nodes 和 branches 不会混淆，nodes 名为小写字母，branches 名为大写字母
    Nodes nodes;
    Branches branches;  // use component name as branch name

    // 从 netlist 中获取的 components, models
    // std::list<Component*> components;
    // std::vector<Model*>& models;

    // Simulation lists
    std::list<DCSimulation*> dc_simulations;
    // std::list<ACSimulation*> ac_simulations;
    // std::list<TranSimulation*> tran_simulations;

    // save results of print or plot
    // ColumnData xdata;
    // std::vector<ColumnData> ydata;
};

#endif  // SPICIAL_CIRCUIT_H
