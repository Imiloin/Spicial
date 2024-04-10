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

double DiodeModel::calcCurrentAtVoltage(double voltage) const {
    if (voltage == 0) {
        return 0;
    }
    return is * (exp((ELECTRON_CHARGE * voltage) / (n * BOLTZMANN_CONSTANT * temperature)) - 1);
}

double DiodeModel::calcVoltageAtCurrent(double current) const {
    if (current == 0) {
        return 0;
    }
    return (n * BOLTZMANN_CONSTANT * temperature / ELECTRON_CHARGE) * log(1 + current / is);
}

double DiodeModel::calcConductanceAtVoltage(double voltage) const {
    return (is * ELECTRON_CHARGE) / (n * BOLTZMANN_CONSTANT * temperature) *
           exp((ELECTRON_CHARGE * voltage) / (n * BOLTZMANN_CONSTANT * temperature));
}
