#ifndef SPICIAL_SIMULATION_H
#define SPICIAL_SIMULATION_H

#include <armadillo>
#include "Netlist.h"
#include "Nodes.h"
#include "Branches.h"
#include "structs.h"

class Simulation {
   public:
    Simulation(Analysis& analysis_, Netlist& netlist_, Nodes& nodes_, Branches& branches_);
    virtual ~Simulation();

    void generateMNA();  // generate MNA and RHS templates

    virtual void runSimulation() = 0;

   protected:
    Analysis& analysis;
    Netlist& netlist;
    Nodes& nodes;
    Branches& branches;

    // MNA and RHS templates
    static arma::sp_mat* MNA_T;
    static arma::vec* RHS_T;
};

class DCSimulation : public Simulation {
   public:
    DCSimulation(Analysis& analysis_, Netlist& netlist_, Nodes& nodes_, Branches& branches_);

    void runSimulation();
   
   private:
    arma::sp_mat* MNA_DC_T;
    arma::vec* RHS_DC_T;

    std::vector<arma::vec> iter_results;      // exclude gnd!!!
};

#endif  // SPICIAL_SIMULATION_H
