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
#include "Component.h"
#include "call_plot.h"
#include "linetype.h"
#include "structs.h"
#include "tokentype.h"

class Circuit {
   public:
    Circuit(const std::string& file);
    ~Circuit();

    bool hasNode(const std::string& node_name);

    void addNode(const std::string& node_name);

    int getNodeIndex(const std::string& name);
    int getNodeIndexExgnd(const std::string& name);

    int getNodeNum() const;
    int getNodeNumExgnd() const;

    void printNodes() const;

    bool hasBranch(const std::string& branch_name);

    void addBranch(const std::string& branch_name);

    int getBranchIndex(const std::string& name);

    int getBranchNum() const;

    void printBranches() const;

    void addComponent(Component* component);

    Component* getComponentPtr(const std::string& name);

    void printSize() const;

    double calcFunctionAtTime(const Function* func,
                              double time,
                              double tstep,
                              double tstop);

    void parseResistor(const std::string& name,
                       const std::string& nplus,
                       const std::string& nminus,
                       double resistance);

    void parseCapacitor(const std::string& name,
                        const std::string& nplus,
                        const std::string& nminus,
                        double capacitance,
                        double initial_voltage = 0);
    void addCapacitor(Capacitor* capacitor);

    void parseInductor(const std::string& name,
                       const std::string& nplus,
                       const std::string& nminus,
                       double inductance,
                       double initial_current = 0);
    void addInductor(Inductor* inductor);

    void parseVCVS(const std::string& name,
                   const std::string& nplus,
                   const std::string& nminus,
                   const std::string& ncplus,
                   const std::string& ncminus,
                   double gain);

    void parseCCCS(const std::string& name,
                   const std::string& nplus,
                   const std::string& nminus,
                   const std::string& vsource,
                   double gain);

    void parseVCCS(const std::string& name,
                   const std::string& nplus,
                   const std::string& nminus,
                   const std::string& ncplus,
                   const std::string& ncminus,
                   double gain);

    void parseCCVS(const std::string& name,
                   const std::string& nplus,
                   const std::string& nminus,
                   const std::string& vsource,
                   double gain);

    // no function
    void parseVoltageSource(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            double dc_voltage,
                            double ac_magnitude = 0,
                            double ac_phase = 0);
    // with function
    void parseVoltageSource(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            const Function& func);
    void addVoltageSource(VoltageSource* voltage_source);

    // no function
    void parseCurrentSource(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            double dc_current,
                            double ac_magnitude = 0,
                            double ac_phase = 0);
    // with function
    void parseCurrentSource(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            const Function& func);
    void addCurrentSource(CurrentSource* current_source);

    void generateDCMNA();
    void generateACMNA();
    void generateTranMNA();
    void printMNADCTemplate() const;
    void printMNAACTemplate() const;
    void printMNATranTemplate() const;

    void DCSimulation(int analysis_type,
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
    std::string file_path;

    std::vector<std::string> nodes;
    std::vector<std::string> nodes_exgnd;  // exclude gnd
    std::vector<std::string> branches;  // use component name as branch name
    std::vector<Component*> components;

    // set only contains names
    std::unordered_set<std::string> resistor_name_set = {};
    std::unordered_set<std::string> capacitor_name_set = {};
    std::unordered_set<std::string> inductor_name_set = {};
    std::unordered_set<std::string> vcvs_name_set = {};
    std::unordered_set<std::string> cccs_name_set = {};
    std::unordered_set<std::string> vccs_name_set = {};
    std::unordered_set<std::string> ccvs_name_set = {};
    std::unordered_set<std::string> voltagesource_name_set = {};
    std::unordered_set<std::string> currentsource_name_set = {};

    std::vector<Capacitor*> capacitors;
    std::vector<Inductor*> inductors;
    std::vector<VoltageSource*> voltage_sources;
    std::vector<CurrentSource*> current_sources;

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
