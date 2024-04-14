#ifndef SPICIAL_CIRCUIT_H
#define SPICIAL_CIRCUIT_H

// #include <algorithm>
#include <complex>
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

    void generateMNATemplate();

    void printMNATemplate();  // (DEBUG) print MNA and RHS templates

    Component* getComponentPtr(const std::string& name);

    void printComponentSize() const;

    Model* getModelPtr(const std::string& name);

    void printModels() const;

    void runSimulations();

    std::vector<ColumnData> createOutputYData(  // 重载实数版本
        const std::vector<Variable>& var_list,
        const std::vector<arma::vec>& sim_results);
    std::vector<ColumnData> createOutputYData(  // 重载复数版本
        const std::vector<Variable>& var_list,
        const std::vector<arma::cx_vec>& sim_cresults);
    void outputResults();
    void printOutputData(ColumnData& xdata,
                         std::vector<ColumnData>& ydata,
                         std::string sim_type,
                         int sim_id = 0) const;
    void plotOutputData(ColumnData& xdata,
                        std::vector<ColumnData>& ydata) const;

   private:
    Netlist& netlist;

    // nodes 和 branches 不会混淆，nodes 名为小写字母，branches 名为大写字母
    Nodes nodes;
    Branches branches;  // use component name as branch name

    // MNA and RHS templates
    arma::sp_mat* MNA_T;
    arma::vec* RHS_T;

    // Simulation lists
    std::list<DCSimulation*> dc_simulations;
    std::list<ACSimulation*> ac_simulations;
    std::list<TranSimulation*> tran_simulations;

    // Output requests
    std::vector<Variable> dc_print_requests;
    std::vector<Variable> dc_plot_requests;  // 所有的 plot 也会顺便 print
    std::vector<Variable> ac_print_requests;
    std::vector<Variable> ac_plot_requests;
    std::vector<Variable> tran_print_requests;
    std::vector<Variable> tran_plot_requests;
};

#endif  // SPICIAL_CIRCUIT_H
