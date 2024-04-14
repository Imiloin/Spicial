#ifndef SPICIAL_SIMULATION_H
#define SPICIAL_SIMULATION_H

#include <armadillo>
#include <variant>
#include "Branches.h"
#include "Netlist.h"
#include "Nodes.h"
#include "function.h"
#include "structs.h"

class Simulation {  // 静态工作点的基类
   public:
    Simulation(Analysis& analysis_,
               Netlist& netlist_,
               Nodes& nodes_,
               Branches& branches_,
               const arma::sp_mat* MNA_T_ = nullptr,
               const arma::vec* RHS_T_ = nullptr);
    virtual ~Simulation();

    virtual void runSimulation();  // run op simulation

    std::string getSimName() { return analysis.sim_name; }
    const std::vector<double>& getIterValues() const {
        return analysis.sim_values;
    }

    // 求解一个工作点
    arma::vec solveOneOP(arma::sp_mat& MNA, arma::vec& RHS, arma::vec& x_prev);  // real

   protected:
    const Analysis& analysis;
    const Netlist& netlist;
    const Nodes& nodes;
    const Branches& branches;

    // simulation parameters for non-linear solver
    double rel_tol;  // relative tolerance, default 1e-3
    double abs_tol;  // absolute tolerance, default 5e-5
    int max_iter;    // maximum iteration number, default 100

    // MNA and RHS templates
    static const arma::sp_mat* MNA_T;
    static const arma::vec* RHS_T;
};

class DCSimulation : public Simulation {
   public:
    DCSimulation(Analysis& analysis_,
                 Netlist& netlist_,
                 Nodes& nodes_,
                 Branches& branches_);

    void runSimulation() override;

    const std::vector<arma::vec>& getIterResults();

   private:
    arma::sp_mat* MNA_DC_T;
    arma::vec* RHS_DC_T;

    std::vector<arma::vec> iter_results;  // exclude gnd!!!
};

class ACSimulation : public Simulation {
   public:
    ACSimulation(Analysis& analysis_,
                 Netlist& netlist_,
                 Nodes& nodes_,
                 Branches& branches_);

    void runSimulation() override;

    const std::vector<arma::cx_vec>& getIterResults();

   private:
    arma::sp_cx_mat* MNA_AC_T;
    arma::cx_vec* RHS_AC_T;

    std::vector<arma::cx_vec> iter_cresults;  // exclude gnd!!!
};

class TranSimulation : public Simulation {
   public:
    TranSimulation(Analysis& analysis_,
                   Netlist& netlist_,
                   Nodes& nodes_,
                   Branches& branches_);

    arma::vec tranBackEuler(double time,
                            double h,
                            const arma::vec x_prevtime);

    void runSimulation() override;

    const std::vector<arma::vec>& getIterResults();

   private:
    arma::sp_mat* MNA_TRAN_T;
    arma::vec* RHS_TRAN_T;

    double tstart;
    double tstep;
    double tstop;

    std::vector<arma::vec> iter_results;  // exclude gnd!!!
};

#endif  // SPICIAL_SIMULATION_H
