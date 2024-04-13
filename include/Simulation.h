#ifndef SPICIAL_SIMULATION_H
#define SPICIAL_SIMULATION_H

#include <armadillo>
#include <variant>
#include "Branches.h"
#include "Netlist.h"
#include "Nodes.h"
#include "structs.h"

class Simulation {
   public:
    Simulation(Analysis& analysis_,
               Netlist& netlist_,
               Nodes& nodes_,
               Branches& branches_);
    virtual ~Simulation();

    void generateMNA();  // generate MNA and RHS templates

    virtual void runSimulation() = 0;

    std::string getIterName() { return analysis.iter_name; }
    const std::vector<double>& getIterValues() const {
        return analysis.iter_values;
    }

   protected:
    const Analysis& analysis;
    const Netlist& netlist;
    const Nodes& nodes;
    const Branches& branches;

    // MNA and RHS templates
    static arma::sp_mat* MNA_T;
    static arma::vec* RHS_T;
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

#endif  // SPICIAL_SIMULATION_H
