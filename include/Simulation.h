#ifndef SPICIAL_SIMULATION_H
#define SPICIAL_SIMULATION_H

#include "Circuit.h"
#include "structs.h"

class Simulation {
   public:
    Simulation(Circuit& circuit, Analysis& analysis);
    ~Simulation();

    void generateMNA();  // generate MNA and RHS templates

   protected:
    Circuit& circuit;
    Analysis& analysis;
    Netlist& netlist;
    std::vector<std::string>& nodes;
    std::vector<std::string>& nodes_exgnd;
    std::vector<std::string>& branches;

    // MNA and RHS templates
    arma::sp_mat* MNA_T;
    arma::vec* RHS_T;
};

#endif  // SPICIAL_SIMULATION_H
