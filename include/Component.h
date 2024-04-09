#ifndef SPICIAL_COMPONENT_H
#define SPICIAL_COMPONENT_H

#include <string>
#include "linetype.h"
#include "structs.h"

class Component {
   protected:
    int type;
    std::string name;

   public:
    Component(const std::string& name);
    virtual ~Component();
    int getType() const;
    std::string getName() const;
};

// Resistor
class Resistor : public Component {
   private:
    std::string nplus;
    std::string nminus;
    double resistance;

   public:
    Resistor(const std::string& name,
             const std::string& nplus,
             const std::string& nminus,
             double resistance);
    std::string getNplus() const;
    std::string getNminus() const;
    double getResistance() const;
};

// Capacitor
class Capacitor : public Component {
   private:
    std::string nplus;
    std::string nminus;
    double capacitance;
    double initial_voltage;

   public:
    Capacitor(const std::string& name,
              const std::string& nplus,
              const std::string& nminus,
              double capacitance,
              double initial_voltage);
    std::string getNplus() const;
    std::string getNminus() const;
    double getCapacitance() const;
    double getInitialVoltage() const;
};

// Inductor
class Inductor : public Component {
   private:
    std::string nplus;
    std::string nminus;
    double inductance;
    double initial_current;

   public:
    Inductor(const std::string& name,
             const std::string& nplus,
             const std::string& nminus,
             double inductance,
             double initial_current);
    std::string getNplus() const;
    std::string getNminus() const;
    double getInductance() const;
    double getInitialCurrent() const;
};

// VCVS
class VCVS : public Component {
   private:
    std::string nplus;
    std::string nminus;
    std::string ncplus;
    std::string ncminus;
    double gain;

   public:
    VCVS(const std::string& name,
         const std::string& nplus,
         const std::string& nminus,
         const std::string& ncplus,
         const std::string& ncminus,
         double gain);
    std::string getNplus() const;
    std::string getNminus() const;
    std::string getNCplus() const;
    std::string getNCminus() const;
    double getGain() const;
};

// CCCS
class CCCS : public Component {
   private:
    std::string nplus;
    std::string nminus;
    std::string vsource;
    double gain;

   public:
    CCCS(const std::string& name,
         const std::string& nplus,
         const std::string& nminus,
         const std::string& vsource,
         double gain);
    std::string getNplus() const;
    std::string getNminus() const;
    std::string getVsource() const;
    double getGain() const;
};

// VCCS
class VCCS : public Component {
   private:
    std::string nplus;
    std::string nminus;
    std::string ncplus;
    std::string ncminus;
    double gain;

   public:
    VCCS(const std::string& name,
         const std::string& nplus,
         const std::string& nminus,
         const std::string& ncplus,
         const std::string& ncminus,
         double gain);
    std::string getNplus() const;
    std::string getNminus() const;
    std::string getNCplus() const;
    std::string getNCminus() const;
    double getGain() const;
};

// CCVS
class CCVS : public Component {
   private:
    std::string nplus;
    std::string nminus;
    std::string vsource;
    double gain;

   public:
    CCVS(const std::string& name,
         const std::string& nplus,
         const std::string& nminus,
         const std::string& vsource,
         double gain);
    std::string getNplus() const;
    std::string getNminus() const;
    std::string getVsource() const;
    double getGain() const;
};

// VoltageSource
class VoltageSource : public Component {
   private:
    std::string nplus;
    std::string nminus;
    double dc_voltage;
    double ac_magnitude;
    double ac_phase;
    Function* function;

   public:
    VoltageSource(const std::string& name,
                  const std::string& nplus,
                  const std::string& nminus,
                  double dc_voltage,
                  double ac_magnitude,
                  double ac_phase);
    std::string getNplus() const;
    std::string getNminus() const;
    double getDCVoltage() const;
    double getACMagnitude() const;
    double getACPhase() const;
    void setFunction(const Function* func);
    Function* getFunction() const;
};

// CurrentSource
class CurrentSource : public Component {
   private:
    std::string nplus;
    std::string nminus;
    double dc_current;
    double ac_magnitude;
    double ac_phase;
    Function* function;

   public:
    CurrentSource(const std::string& name,
                  const std::string& nplus,
                  const std::string& nminus,
                  double dc_current,
                  double ac_magnitude,
                  double ac_phase);
    std::string getNplus() const;
    std::string getNminus() const;
    double getDCCurrent() const;
    double getACMagnitude() const;
    double getACPhase() const;
    void setFunction(const Function* func);
    Function* getFunction() const;
};

// Diode
class Diode : public Component {
   private:
    std::string nplus;
    std::string nminus;
    std::string model;
    double initial_voltage;

   public:
    Diode(const std::string& name,
          const std::string& nplus,
          const std::string& nminus,
          const std::string& model,
          double initial_voltage);
    std::string getNplus() const;
    std::string getNminus() const;
    std::string getModel() const;
    double getInitialVoltage() const;
};

#endif  // SPICIAL_COMPONENT_H
