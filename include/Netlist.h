#ifndef SPICIAL_NETLIST_H
#define SPICIAL_NETLIST_H

/** 仅保存解析得到的网表，不进行任何仿真 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <unordered_set>
#include <vector>
#include "Component.h"
#include "Model.h"
#include "linetype.h"
#include "structs.h"
#include "tokentype.h"

class Netlist {
   public:
    Netlist(const std::string& file, const std::string& title = "Circuit");
    ~Netlist();

    friend class Circuit;
    friend class Simulation;
    friend class DCSimulation;
    friend class ACSimulation;
    friend class TranSimulation;

    bool hasModel(const std::string& model_name);

    void addModel(Model* model);

    Model* getModelPtr(const std::string& name);

    void printModels() const;

    void addComponent(Component* component);

    void replaceComponent(const std::string& name, Component* newComponent);

    Component* getComponentPtr(const std::string& name) const;

    void parseResistor(const std::string& name,
                       const std::string& nplus,
                       const std::string& nminus,
                       double resistance);

    void parseCapacitor(const std::string& name,
                        const std::string& nplus,
                        const std::string& nminus,
                        double capacitance,
                        double initial_voltage = 0);

    void parseInductor(const std::string& name,
                       const std::string& nplus,
                       const std::string& nminus,
                       double inductance,
                       double initial_current = 0);

    void parseVCVS(const std::string& name,
                   const std::string& nplus,
                   const std::string& nminus,
                   const std::string& ncplus,
                   const std::string& ncminus,
                   double gain);

    void parseCCCS(const std::string& name,
                   const std::string& nplus,
                   const std::string& nminus,
                   const std::string& vsource,
                   double gain);

    void parseVCCS(const std::string& name,
                   const std::string& nplus,
                   const std::string& nminus,
                   const std::string& ncplus,
                   const std::string& ncminus,
                   double gain);

    void parseCCVS(const std::string& name,
                   const std::string& nplus,
                   const std::string& nminus,
                   const std::string& vsource,
                   double gain);

    // no function
    void parseVoltageSource(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            double dc_voltage,
                            double ac_magnitude = 0,
                            double ac_phase = 0);
    // with function
    void parseVoltageSource(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            double dc_voltage,
                            double ac_magnitude,
                            double ac_phase,
                            const Function& func);
    // no function
    void parseCurrentSource(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            double dc_current,
                            double ac_magnitude = 0,
                            double ac_phase = 0);
    // with function
    void parseCurrentSource(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            double dc_current,
                            double ac_magnitude,
                            double ac_phase,
                            const Function& func);

    void parseDiode(const std::string& name,
                    const std::string& nplus,
                    const std::string& nminus,
                    const std::string& model,
                    double initial_voltage = 0);

    void printComponentSize() const;

    void parseDC(int source_type,
                 const std::string& source,
                 double start,
                 double end,
                 double increment);

    void parseAC(int analysis_type,
                 double n,
                 double freq_start,
                 double freq_end);

    void parseTran(double step, double stop_time, double start_time = 0);

    void parsePrint(int analysis_type, const std::vector<Variable>& var_list);

    void parsePlot(int analysis_type, const std::vector<Variable>& var_list);

   private:
    std::string file_path;
    std::string title;

    std::list<Component*> components;  // 元件，包括 R, C, L, VCVS, CCCS, VCCS, CCVS, VS, IS, D, M
    std::vector<Model*> models;        // 模型
    std::list<Analysis*> analyses;     // 分析，包括 OP, AC, DC, TRAN
    std::list<Output*> outputs;        // 输出，包括 PRINT, PLOT

    // set only contains names
    std::unordered_set<std::string> resistor_name_set = {};
    std::unordered_set<std::string> capacitor_name_set = {};
    std::unordered_set<std::string> inductor_name_set = {};
    std::unordered_set<std::string> vcvs_name_set = {};
    std::unordered_set<std::string> cccs_name_set = {};
    std::unordered_set<std::string> vccs_name_set = {};
    std::unordered_set<std::string> ccvs_name_set = {};
    std::unordered_set<std::string> voltagesource_name_set = {};
    std::unordered_set<std::string> currentsource_name_set = {};
    std::unordered_set<std::string> diode_name_set = {};

    // some special components
    std::vector<Capacitor*> capacitors;
    std::vector<Inductor*> inductors;
    std::vector<VoltageSource*> voltage_sources;
    std::vector<CurrentSource*> current_sources;
    std::vector<Diode*> diodes;
};

#endif  // SPICIAL_NETLIST_H
