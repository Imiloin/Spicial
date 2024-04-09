#include "Model.h"
#include <QDebug>

// Constants
const double Model::ELECTRON_CHARGE = 1.602176634e-19;  // Coulombs
const double Model::BOLTZMANN_CONSTANT = 1.380649e-23;  // Joules per Kelvin

Model::Model(const std::string& name, double temperature)
    : name(name), temperature(temperature) {
    type = -1;
}

Model::~Model() {}

int Model::getType() const {
    return type;
}

std::string Model::getName() const {
    return name;
}

// **** Diode Model ****
DiodeModel::DiodeModel(const std::string& name, double is, double n)
    : Model(name) {
    this->is = is;
    if (n < 0) {
        this->n = ELECTRON_CHARGE / (40 * BOLTZMANN_CONSTANT * temperature);
    } else {
        this->n = n;
    }
    type = COMPONENT_DIODE;
}