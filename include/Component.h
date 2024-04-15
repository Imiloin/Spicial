#ifndef SPICIAL_COMPONENT_H
#define SPICIAL_COMPONENT_H

#include <string>
#include "Model.h"
#include "linetype.h"
#include "structs.h"

class Component {
   protected:
    int type;
    std::string name;

   public:
    friend class Circuit;
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
    int id_nplus;
    int id_nminus;

   public:
    friend class Circuit;
    Resistor(const std::string& name,
             const std::string& nplus,
             const std::string& nminus,
             double resistance);
    std::string getNplus() const { return nplus; }
    std::string getNminus() const { return nminus; }
    double getResistance() const { return resistance; }
    int getIdNplus() const { return id_nplus; }
    int getIdNminus() const { return id_nminus; }
};

// Capacitor
class Capacitor : public Component {
   private:
    std::string nplus;
    std::string nminus;
    double capacitance;
    double initial_voltage;
    int id_nplus;
    int id_nminus;
    int id_branch;

   public:
    friend class Circuit;
    Capacitor(const std::string& name,
              const std::string& nplus,
              const std::string& nminus,
              double capacitance,
              double initial_voltage);
    std::string getNplus() const { return nplus; }
    std::string getNminus() const { return nminus; }
    double getCapacitance() const { return capacitance; }
    double getInitialVoltage() const { return initial_voltage; }
    int getIdNplus() const { return id_nplus; }
    int getIdNminus() const { return id_nminus; }
    int getIdBranch() const { return id_branch; }
};

// Inductor
class Inductor : public Component {
   private:
    std::string nplus;
    std::string nminus;
    double inductance;
    double initial_current;
    int id_nplus;
    int id_nminus;
    int id_branch;

   public:
    friend class Circuit;
    Inductor(const std::string& name,
             const std::string& nplus,
             const std::string& nminus,
             double inductance,
             double initial_current);
    std::string getNplus() const { return nplus; }
    std::string getNminus() const { return nminus; }
    double getInductance() const { return inductance; }
    double getInitialCurrent() const { return initial_current; }
    int getIdNplus() const { return id_nplus; }
    int getIdNminus() const { return id_nminus; }
    int getIdBranch() const { return id_branch; }
};

// VCVS
class VCVS : public Component {
   private:
    std::string nplus;
    std::string nminus;
    std::string ncplus;
    std::string ncminus;
    double gain;
    int id_nplus;
    int id_nminus;
    int id_ncplus;
    int id_ncminus;
    int id_branch;

   public:
    friend class Circuit;
    VCVS(const std::string& name,
         const std::string& nplus,
         const std::string& nminus,
         const std::string& ncplus,
         const std::string& ncminus,
         double gain);
    std::string getNplus() const { return nplus; }
    std::string getNminus() const { return nminus; }
    std::string getNCplus() const { return ncplus; }
    std::string getNCminus() const { return ncminus; }
    double getGain() const { return gain; }
    int getIdNplus() const { return id_nplus; }
    int getIdNminus() const { return id_nminus; }
    int getIdNCplus() const { return id_ncplus; }
    int getIdNCminus() const { return id_ncminus; }
    int getIdBranch() const { return id_branch; }
};

// CCCS
class CCCS : public Component {
   private:
    std::string nplus;
    std::string nminus;
    std::string vsource;
    double gain;
    int id_nplus;
    int id_nminus;

   public:
    friend class Circuit;
    CCCS(const std::string& name,
         const std::string& nplus,
         const std::string& nminus,
         const std::string& vsource,
         double gain);
    std::string getNplus() const { return nplus; }
    std::string getNminus() const { return nminus; }
    std::string getVsource() const { return vsource; }
    double getGain() const { return gain; }
    int getIdNplus() const { return id_nplus; }
    int getIdNminus() const { return id_nminus; }
};

// VCCS
class VCCS : public Component {
   private:
    std::string nplus;
    std::string nminus;
    std::string ncplus;
    std::string ncminus;
    double gain;
    int id_nplus;
    int id_nminus;
    int id_ncplus;
    int id_ncminus;

   public:
    friend class Circuit;
    VCCS(const std::string& name,
         const std::string& nplus,
         const std::string& nminus,
         const std::string& ncplus,
         const std::string& ncminus,
         double gain);
    std::string getNplus() const { return nplus; }
    std::string getNminus() const { return nminus; }
    std::string getNCplus() const { return ncplus; }
    std::string getNCminus() const { return ncminus; }
    double getGain() const { return gain; }
    int getIdNplus() const { return id_nplus; }
    int getIdNminus() const { return id_nminus; }
    int getIdNCplus() const { return id_ncplus; }
    int getIdNCminus() const { return id_ncminus; }
};

// CCVS
class CCVS : public Component {
   private:
    std::string nplus;
    std::string nminus;
    std::string vsource;
    double gain;
    int id_nplus;
    int id_nminus;
    int id_branch;

   public:
    friend class Circuit;
    CCVS(const std::string& name,
         const std::string& nplus,
         const std::string& nminus,
         const std::string& vsource,
         double gain);
    std::string getNplus() const { return nplus; }
    std::string getNminus() const { return nminus; }
    std::string getVsource() const { return vsource; }
    double getGain() const { return gain; }
    int getIdNplus() const { return id_nplus; }
    int getIdNminus() const { return id_nminus; }
    int getIdBranch() const { return id_branch; }
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
    int id_nplus;
    int id_nminus;
    int id_branch;

   public:
    friend class Circuit;
    VoltageSource(const std::string& name,
                  const std::string& nplus,
                  const std::string& nminus,
                  double dc_voltage,
                  double ac_magnitude,
                  double ac_phase);
    std::string getNplus() const { return nplus; }
    std::string getNminus() const { return nminus; }
    double getDCVoltage() const { return dc_voltage; }
    double getACMagnitude() const { return ac_magnitude; }
    double getACPhase() const { return ac_phase; }
    void setFunction(const Function* func);
    Function* getFunction() const;
    int getIdNplus() const { return id_nplus; }
    int getIdNminus() const { return id_nminus; }
    int getIdBranch() const { return id_branch; }
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
    int id_nplus;
    int id_nminus;

   public:
    friend class Circuit;
    CurrentSource(const std::string& name,
                  const std::string& nplus,
                  const std::string& nminus,
                  double dc_current,
                  double ac_magnitude,
                  double ac_phase);
    std::string getNplus() const { return nplus; }
    std::string getNminus() const { return nminus; }
    double getDCCurrent() const { return dc_current; }
    double getACMagnitude() const { return ac_magnitude; }
    double getACPhase() const { return ac_phase; }
    void setFunction(const Function* func);
    Function* getFunction() const;
    int getIdNplus() const { return id_nplus; }
    int getIdNminus() const { return id_nminus; }
};

// Diode
class Diode : public Component {
   private:
    std::string nplus;
    std::string nminus;
    std::string modelname;
    double initial_voltage;
    int id_nplus;
    int id_nminus;
    int id_branch;
    DiodeModel* model;

   public:
    friend class Circuit;
    Diode(const std::string& name,
          const std::string& nplus,
          const std::string& nminus,
          const std::string& modelname,
          double initial_voltage);
    std::string getNplus() const { return nplus; }
    std::string getNminus() const { return nminus; }
    std::string getModelname() const { return modelname; }
    double getInitialVoltage() const { return initial_voltage; }
    int getIdNplus() const { return id_nplus; }
    int getIdNminus() const { return id_nminus; }
    int getIdBranch() const { return id_branch; }
    DiodeModel* getModel() const;
};

#endif  // SPICIAL_COMPONENT_H
