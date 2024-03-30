#include "Component.h"

// Component
Component::Component(const std::string& name) {
    this->name = name;
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

std::string Resistor::getNplus() const {
    return nplus;
}

std::string Resistor::getNminus() const {
    return nminus;
}

double Resistor::getResistance() const {
    return resistance;
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

std::string Capacitor::getNplus() const {
    return nplus;
}

std::string Capacitor::getNminus() const {
    return nminus;
}

double Capacitor::getCapacitance() const {
    return capacitance;
}

double Capacitor::getInitialVoltage() const {
    return initial_voltage;
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

std::string Inductor::getNplus() const {
    return nplus;
}

std::string Inductor::getNminus() const {
    return nminus;
}

double Inductor::getInductance() const {
    return inductance;
}

double Inductor::getInitialCurrent() const {
    return initial_current;
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

std::string VCVS::getNplus() const {
    return nplus;
}

std::string VCVS::getNminus() const {
    return nminus;
}

std::string VCVS::getNCplus() const {
    return ncplus;
}

std::string VCVS::getNCminus() const {
    return ncminus;
}

double VCVS::getGain() const {
    return gain;
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

std::string CCCS::getNplus() const {
    return nplus;
}

std::string CCCS::getNminus() const {
    return nminus;
}

std::string CCCS::getVsource() const {
    return vsource;
}

double CCCS::getGain() const {
    return gain;
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

std::string VCCS::getNplus() const {
    return nplus;
}

std::string VCCS::getNminus() const {
    return nminus;
}

std::string VCCS::getNCplus() const {
    return ncplus;
}

std::string VCCS::getNCminus() const {
    return ncminus;
}

double VCCS::getGain() const {
    return gain;
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

std::string CCVS::getNplus() const {
    return nplus;
}

std::string CCVS::getNminus() const {
    return nminus;
}

std::string CCVS::getVsource() const {
    return vsource;
}

double CCVS::getGain() const {
    return gain;
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
}

std::string VoltageSource::getNplus() const {
    return nplus;
}

std::string VoltageSource::getNminus() const {
    return nminus;
}

double VoltageSource::getDCVoltage() const {
    return dc_voltage;
}

double VoltageSource::getACMagnitude() const {
    return ac_magnitude;
}

double VoltageSource::getACPhase() const {
    return ac_phase;
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
}

std::string CurrentSource::getNplus() const {
    return nplus;
}

std::string CurrentSource::getNminus() const {
    return nminus;
}

double CurrentSource::getDCCurrent() const {
    return dc_current;
}

double CurrentSource::getACMagnitude() const {
    return ac_magnitude;
}

double CurrentSource::getACPhase() const {
    return ac_phase;
}
