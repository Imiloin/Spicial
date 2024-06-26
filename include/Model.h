#ifndef SPICIAL_MODEL_H
#define SPICIAL_MODEL_H

#include <cmath>
#include <string>
#include "linetype.h"

class Model {
   protected:
    int type;
    std::string name;
    static double temperature;  // in Kelvin

   public:
    static const double ELECTRON_CHARGE;     // 元电荷
    static const double BOLTZMANN_CONSTANT;  // 玻尔兹曼常数

    Model(const std::string& name);
    virtual ~Model();
    int getType() const;
    std::string getName() const;
    void setTemperature(double temp);
};

// Diode Model
class DiodeModel : public Model {
   private:
    double is;
    double n;

   public:
    DiodeModel(const std::string& name,
               double is = 1e-12,  // saturation current default 1e-12
               double n = -1  // if n = -1, then n = ELECTRON_CHARGE / (40 *
                              // BOLTZMANN_CONSTANT * temperture)
    );
    double calcCurrentAtVoltage(double voltage) const;
    double calcVoltageAtCurrent(double current) const;
    double calcConductanceAtVoltage(double voltage) const;  // dI/dV
};

#endif  // SPICIAL_MODEL_H
