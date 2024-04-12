#include "Component.h"

// Component
Component::Component(const std::string& name) {
    this->name = name;
    type = -1;
}  // 构造函数(构造函数不能是虚函数)

Component::~Component() {}  // 析构函数(虚函数)

int Component::getType() const {
    return type;
}

std::string Component::getName() const {
    return name;
}

// **** Resistor ****
Resistor::Resistor(const std::string& name,
                   const std::string& nplus,
                   const std::string& nminus,
                   double resistance)
    : Component(name), nplus(nplus), nminus(nminus), resistance(resistance) {
    type = COMPONENT_RESISTOR;
}

// **** Capacitor ****
Capacitor::Capacitor(const std::string& name,
                     const std::string& nplus,
                     const std::string& nminus,
                     double capacitance,
                     double initial_voltage)
    : Component(name),
      nplus(nplus),
      nminus(nminus),
      capacitance(capacitance),
      initial_voltage(initial_voltage) {
    type = COMPONENT_CAPACITOR;
}

// **** Inductor ****
Inductor::Inductor(const std::string& name,
                   const std::string& nplus,
                   const std::string& nminus,
                   double inductance,
                   double initial_current)
    : Component(name),
      nplus(nplus),
      nminus(nminus),
      inductance(inductance),
      initial_current(initial_current) {
    type = COMPONENT_INDUCTOR;
}

// **** VCVS ****
VCVS::VCVS(const std::string& name,
           const std::string& nplus,
           const std::string& nminus,
           const std::string& ncplus,
           const std::string& ncminus,
           double gain)
    : Component(name),
      nplus(nplus),
      nminus(nminus),
      ncplus(ncplus),
      ncminus(ncminus),
      gain(gain) {
    type = COMPONENT_VCVS;
}

// **** CCCS ****
CCCS::CCCS(const std::string& name,
           const std::string& nplus,
           const std::string& nminus,
           const std::string& vsource,
           double gain)
    : Component(name),
      nplus(nplus),
      nminus(nminus),
      vsource(vsource),
      gain(gain) {
    type = COMPONENT_CCCS;
}

// **** VCCS ****
VCCS::VCCS(const std::string& name,
           const std::string& nplus,
           const std::string& nminus,
           const std::string& ncplus,
           const std::string& ncminus,
           double gain)
    : Component(name),
      nplus(nplus),
      nminus(nminus),
      ncplus(ncplus),
      ncminus(ncminus),
      gain(gain) {
    type = COMPONENT_VCCS;
}

// **** CCVS ****
CCVS::CCVS(const std::string& name,
           const std::string& nplus,
           const std::string& nminus,
           const std::string& vsource,
           double gain)
    : Component(name),
      nplus(nplus),
      nminus(nminus),
      vsource(vsource),
      gain(gain) {
    type = COMPONENT_CCVS;
}

// **** VoltageSource ****
VoltageSource::VoltageSource(const std::string& name,
                             const std::string& nplus,
                             const std::string& nminus,
                             double dc_voltage,
                             double ac_magnitude,
                             double ac_phase)
    : Component(name),
      nplus(nplus),
      nminus(nminus),
      dc_voltage(dc_voltage),
      ac_magnitude(ac_magnitude),
      ac_phase(ac_phase) {
    type = COMPONENT_VOLTAGE_SOURCE;
    function = nullptr;
}

void VoltageSource::setFunction(const Function* func) {
    function = const_cast<Function*>(func);
}

Function* VoltageSource::getFunction() const {
    return function;
}

// **** CurrentSource ****
CurrentSource::CurrentSource(const std::string& name,
                             const std::string& nplus,
                             const std::string& nminus,
                             double dc_current,
                             double ac_magnitude,
                             double ac_phase)
    : Component(name),
      nplus(nplus),
      nminus(nminus),
      dc_current(dc_current),
      ac_magnitude(ac_magnitude),
      ac_phase(ac_phase) {
    type = COMPONENT_CURRENT_SOURCE;
    function = nullptr;
}

void CurrentSource::setFunction(const Function* func) {
    function = const_cast<Function*>(func);
}

Function* CurrentSource::getFunction() const {
    return function;
}

// **** Diode ****
Diode::Diode(const std::string& name,
             const std::string& nplus,
             const std::string& nminus,
             const std::string& modelname,
             double initial_voltage)
    : Component(name),
      nplus(nplus),
      nminus(nminus),
      modelname(modelname),
      initial_voltage(initial_voltage) {
    type = COMPONENT_DIODE;
}

DiodeModel* Diode::getModel() const {
    return model;
}
