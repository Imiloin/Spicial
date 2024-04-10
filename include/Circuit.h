#ifndef SPICIAL_CIRCUIT_H
#define SPICIAL_CIRCUIT_H

#include <algorithm>
#include <armadillo>
#include <complex>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>  // for std::invalid_argument
#include <string>
#include <unordered_set>
#include <vector>
#include "Netlist.h"
#include "call_plot.h"
#include "linetype.h"
#include "structs.h"
#include "tokentype.h"

class Circuit {
   public:
    Circuit(Netlist& netlist_);
    ~Circuit();

    void generateNodesBranches();  // 生成 nodes 和 branches，同时为 components
                                   // 分配 nodes 和 branches 的索引

    int addNode(const std::string& newNode);

    int getNodeIndex(const std::string& name);
    int getNodeIndexExgnd(const std::string& name);

    int getNodeNum() const;
    int getNodeNumExgnd() const;

    void printNodes() const;

    int addBranch(const std::string& newBranch);

    int getBranchIndex(const std::string& name);

    int getBranchNum() const;

    void printBranches() const;

    double calcFunctionAtTime(const Function* func,
                              double time,
                              double tstep,
                              double tstop);

    void generateDCMNA();
    void generateACMNA();
    void generateTranMNA();
    void printMNADCTemplate() const;
    void printMNAACTemplate() const;
    void printMNATranTemplate() const;

    void DCSimulation(int source_type,
                      const std::string& source,
                      double start,
                      double end,
                      double increment);
    void ACSimulation(int analysis_type,
                      double n,
                      double freq_start,
                      double freq_end);
    arma::vec tranBackEuler(double time,
                            double h,
                            double tstep,
                            double tstop,
                            const arma::vec x_prev);
    void TranSimulation(double step, double stop_time, double start_time = 0);

    void printAnalysis(int analysis_type,
                       const std::vector<Variable>& var_list);
    void plotAnalysis(int analysis_type, const std::vector<Variable>& var_list);
    // 其他方法...
    void printResults() const;

   private:
    Netlist& netlist;

    // nodes 和 branches 不会混淆，nodes 名为小写字母，branches 名为大写字母
    std::vector<std::string> nodes;
    std::vector<std::string> nodes_exgnd;  // exclude gnd
    std::vector<std::string> branches;     // use component name as branch name
    // std::vector<Component*> components;
    // std::vector<Model*> models;

    // MNA and RHS pointers, a template without analysis
    arma::sp_mat* MNA_DC_T;
    arma::vec* RHS_DC_T;
    arma::sp_cx_mat* MNA_AC_T;
    arma::cx_vec* RHS_AC_T;
    arma::sp_mat* MNA_TRAN_T;
    arma::vec* RHS_TRAN_T;

    // save the results of simulation
    int simulation_type;
    std::string iter_name;
    std::vector<double> iter_values;
    std::vector<arma::vec> iter_results;      // exclude gnd!!!
    std::vector<arma::cx_vec> iter_cresults;  // for AC simulation

    // save results of print or plot
    ColumnData xdata;
    std::vector<ColumnData> ydata;
};

#endif  // SPICIAL_CIRCUIT_H
