#include "Netlist.h"
#include <QDebug>

Netlist::Netlist(const std::string& file) {
    this->file_path = file;

    /////// test only ////////
    Model* diode1 = new DiodeModel("diode1");
    models.push_back(diode1);
    /////// delete after test ////////
}

Netlist::~Netlist() {
    for (Component* component : components) {
        delete component;
    }
    for (Model* model : models) {
        delete model;
    }
    for (Analysis* analysis : analyses) {
        delete analysis;
    }
    for (Output* output : outputs) {
        delete output;
    }
}

bool Netlist::hasModel(const std::string& modelname) {
    for (Model* model : models) {
        if (model->getName() == modelname) {
            return true;
        }
    }
    return false;
}

void Netlist::addModel(Model* model) {
    models.push_back(model);
}

/*
Model* Netlist::getModelPtr(const std::string& name) {
    for (Model* model : models) {
        if (model->getName() == name) {
            return model;
        }
    }
    return nullptr;
}
*/

Model* Netlist::getModelPtr(const std::string& name) {
    // std::vector<Model*>& models;
    auto it = std::find_if(models.begin(), models.end(), [&name](Model* model) {
        return model->getName() == name;
    });
    if (it == models.end()) {
        qDebug() << "getModelPtr(" << name.c_str() << ")";
        printf("Model not found\n");
        return nullptr;
    }
    return *it;
}

void Netlist::printModels() const {
    // print {model: name}
    std::cout << std::endl;
    std::cout << "------------------- " << std::endl
              << "Models: {name}" << std::endl;
    for (Model* model : models) {
        std::cout << "{" << model->getName() << "}" << std::endl;
    }
    std::cout << "------------------- " << std::endl;
}

void Netlist::addComponent(Component* component) {
    components.push_back(component);
}

// overwrite existing component with new component
void Netlist::replaceComponent(const std::string& name,
                               Component* newComponent) {
    for (auto it = components.begin(); it != components.end(); ++it) {
        if ((*it)->getName() == name) {
            delete *it;  // 如果原组件是动态分配的，需要先删除
            *it = newComponent;
            return;
        }
    }
}

/*
Component* Netlist::getComponentPtr(const std::string& name) {
    for (Component* component : components) {
        if (component->getName() == name) {
            return component;
        }
    }
    return nullptr;
}
*/

Component* Netlist::getComponentPtr(const std::string& name) const {
    auto it = std::find_if(
        components.begin(), components.end(),
        [&name](Component* component) { return component->getName() == name; });
    if (it == components.end()) {
        qDebug() << "getComponentPtr(" << name.c_str() << ")"
                 << "Component not found";
        return nullptr;
    }
    return *it;
}

void Netlist::printComponentSize() const {
    std::cout << "Resistor: " << resistor_name_set.size() << std::endl;
    std::cout << "Capacitor: " << capacitor_name_set.size() << std::endl;
    std::cout << "Inductor: " << inductor_name_set.size() << std::endl;
    std::cout << "VCVS: " << vcvs_name_set.size() << std::endl;
    std::cout << "CCCS: " << cccs_name_set.size() << std::endl;
    std::cout << "VCCS: " << vccs_name_set.size() << std::endl;
    std::cout << "CCVS: " << ccvs_name_set.size() << std::endl;
    std::cout << "Voltage Source: " << voltagesource_name_set.size()
              << std::endl;
    std::cout << "Current Source: " << currentsource_name_set.size()
              << std::endl;
    std::cout << "Diode: " << diode_name_set.size() << std::endl;
}

void Netlist::parseResistor(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            double resistance) {
    Resistor* resistor = new Resistor(name, nplus, nminus, resistance);
    if (!resistor_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseResistor(" << name.c_str() << ")";
        std::cerr << "Parse warning: Resistor " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, resistor);
        return;
    }

    this->addComponent(resistor);
}

void Netlist::parseCapacitor(const std::string& name,
                             const std::string& nplus,
                             const std::string& nminus,
                             double capacitance,
                             double initial_voltage) {
    Capacitor* capacitor =
        new Capacitor(name, nplus, nminus, capacitance, initial_voltage);
    if (!capacitor_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseCapacitor(" << name.c_str() << ")";
        std::cerr << "Parse warning: Capacitor " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, capacitor);
        return;
    }

    this->addComponent(capacitor);
    capacitors.push_back(capacitor);
}

void Netlist::parseInductor(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            double inductance,
                            double initial_current) {
    Inductor* inductor =
        new Inductor(name, nplus, nminus, inductance, initial_current);
    if (!inductor_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseInductor(" << name.c_str() << ")";
        std::cerr << "Parse warning: Inductor " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, inductor);
        return;
    }

    this->addComponent(inductor);
    inductors.push_back(inductor);
}

void Netlist::parseVCVS(const std::string& name,
                        const std::string& nplus,
                        const std::string& nminus,
                        const std::string& ncplus,
                        const std::string& ncminus,
                        double gain) {
    VCVS* vcvs = new VCVS(name, nplus, nminus, ncplus, ncminus, gain);
    if (!vcvs_name_set.insert(name).second) {  // if already exists in the set
        qDebug() << "parseVCVS(" << name.c_str() << ")";
        std::cerr << "Parse warning: VCVS " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, vcvs);
        return;
    }

    this->addComponent(vcvs);
}

void Netlist::parseCCCS(const std::string& name,
                        const std::string& nplus,
                        const std::string& nminus,
                        const std::string& vsource,
                        double gain) {
    CCCS* cccs = new CCCS(name, nplus, nminus, vsource, gain);
    if (!cccs_name_set.insert(name).second) {  // if already exists in the set
        qDebug() << "parseCCCS(" << name.c_str() << ")";
        std::cerr << "Parse warning: CCCS " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, cccs);
        return;
    }

    this->addComponent(cccs);
}

void Netlist::parseVCCS(const std::string& name,
                        const std::string& nplus,
                        const std::string& nminus,
                        const std::string& ncplus,
                        const std::string& ncminus,
                        double gain) {
    VCCS* vccs = new VCCS(name, nplus, nminus, ncplus, ncminus, gain);
    if (!vccs_name_set.insert(name).second) {  // if already exists in the set
        qDebug() << "parseVCCS(" << name.c_str() << ")";
        std::cerr << "Parse warning: VCCS " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, vccs);
        return;
    }

    this->addComponent(vccs);
}

void Netlist::parseCCVS(const std::string& name,
                        const std::string& nplus,
                        const std::string& nminus,
                        const std::string& vsource,
                        double gain) {
    CCVS* ccvs = new CCVS(name, nplus, nminus, vsource, gain);
    if (!ccvs_name_set.insert(name).second) {  // if already exists in the set
        qDebug() << "parseCCVS(" << name.c_str() << ")";
        std::cerr << "Parse warning: CCVS " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, ccvs);
        return;
    }

    this->addComponent(ccvs);
}

void Netlist::parseVoltageSource(const std::string& name,
                                 const std::string& nplus,
                                 const std::string& nminus,
                                 double dc_voltage,
                                 double ac_magnitude,
                                 double ac_phase) {
    VoltageSource* voltage_source = new VoltageSource(
        name, nplus, nminus, dc_voltage, ac_magnitude, ac_phase);
    if (!voltagesource_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseVoltageSource(" << name.c_str() << ")";
        std::cerr << "Parse warning: Voltage source " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, voltage_source);
        return;
    }

    // qDebug() << "parseVoltageSource() name: " << name.c_str();
    this->addComponent(voltage_source);
    voltage_sources.push_back(voltage_source);
}
// 重载 parseVoltageSource() 函数，含有 Function 参数
void Netlist::parseVoltageSource(const std::string& name,
                                 const std::string& nplus,
                                 const std::string& nminus,
                                 const Function& func) {
    VoltageSource* voltage_source =
        new VoltageSource(name, nplus, nminus, 0, 0, 0);
    voltage_source->setFunction(&func);
    if (!voltagesource_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseVoltageSource(" << name.c_str() << ")";
        std::cerr << "Parse warning: Voltage source " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, voltage_source);
        return;
    }

    // qDebug() << "parseVoltageSource() name: " << name.c_str();
    this->addComponent(voltage_source);
    voltage_sources.push_back(voltage_source);
}

void Netlist::parseCurrentSource(const std::string& name,
                                 const std::string& nplus,
                                 const std::string& nminus,
                                 double dc_current,
                                 double ac_magnitude,
                                 double ac_phase) {
    CurrentSource* current_source = new CurrentSource(
        name, nplus, nminus, dc_current, ac_magnitude, ac_phase);
    if (!currentsource_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseCurrentSource(" << name.c_str() << ")";
        std::cerr << "Parse warning: Current source " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, current_source);
        return;
    }

    // qDebug() << "parseCurrentSource() name: " << name.c_str();
    this->addComponent(current_source);
    current_sources.push_back(current_source);
}
// 重载 parseCurrentSource() 函数，含有 Function 参数
void Netlist::parseCurrentSource(const std::string& name,
                                 const std::string& nplus,
                                 const std::string& nminus,
                                 const Function& func) {
    CurrentSource* current_source =
        new CurrentSource(name, nplus, nminus, 0, 0, 0);
    current_source->setFunction(&func);
    if (!currentsource_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseCurrentSource(" << name.c_str() << ")";
        std::cerr << "Parse warning: Current source " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, current_source);
        return;
    }

    // qDebug() << "parseCurrentSource() name: " << name.c_str();
    this->addComponent(current_source);
    current_sources.push_back(current_source);
}

void Netlist::parseDiode(const std::string& name,
                         const std::string& nplus,
                         const std::string& nminus,
                         const std::string& modelname,
                         double initial_voltage) {
    if (!hasModel(modelname)) {
        qDebug() << "parseDiode(" << name.c_str() << ")";
        std::cerr << "Parse warning: Diode " << modelname
                  << " model not found.\n";
        return;
    }

    Diode* diode = new Diode(name, nplus, nminus, modelname, initial_voltage);
    if (!diode_name_set.insert(name).second) {  // if already exists in the set
        qDebug() << "parseDiode(" << name.c_str() << ")";
        std::cerr << "Parse warning: Diode " << name
                  << " already exists, old one will be replaced.\n";
        replaceComponent(name, diode);
        return;
    }

    this->addComponent(diode);
    diodes.push_back(diode);
}

void Netlist::parseDC(int source_type,
                      const std::string& source,
                      double start,
                      double end,
                      double increment) {
    Analysis* analysis = new Analysis();

    // 将source中的字母转换为大写
    std::string source_u = source;
    std::transform(source_u.begin(), source_u.end(), source_u.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    analysis->analysis_type = ANALYSIS_DC;
    analysis->source_type = source_type;
    analysis->source_name = source_u;
    for (double iter = start; iter <= end; iter += increment) {
        analysis->sim_values.push_back(iter);
    }

    switch (source_type) {
        case (COMPONENT_VOLTAGE_SOURCE): {
            // qDebug() << "parseDC() source: " << source_u.c_str();
            analysis->sim_name = "voltage(" + source_u + ") / V";
            break;
        }
        case (COMPONENT_CURRENT_SOURCE): {
            // qDebug() << "parseDC() source: " << source_u.c_str();
            analysis->sim_name = "current(" + source_u + ") / A";
            break;
        }
        default:
            break;
    }

    analyses.push_back(analysis);
}

void Netlist::parseAC(int ac_type,
                      double n,
                      double freq_start,
                      double freq_end) {
    Analysis* analysis = new Analysis();

    analysis->analysis_type = ANALYSIS_AC;
    analysis->sim_name = "frequency / Hz";

    // qDebug() << "parseAC() ac_type: " << ac_type;

    // 获取仿真的频率点
    switch (ac_type) {
        case (TOKEN_DEC): {
            int n_per_dec = std::round(n);
            double ratio = pow(10.0, 1.0 / n_per_dec);
            for (double freq = freq_start; freq < freq_end; freq *= ratio) {
                analysis->sim_values.push_back(freq);
            }
            analysis->sim_values.push_back(freq_end);
            break;
        }
        case (TOKEN_OCT): {
            int n_per_oct = std::round(n);
            double ratio = pow(8.0, 1.0 / n_per_oct);
            for (double freq = freq_start; freq < freq_end; freq *= ratio) {
                analysis->sim_values.push_back(freq);
            }
            analysis->sim_values.push_back(freq_end);
            break;
        }
        case (TOKEN_LIN): {
            int n_lin = std::round(n);
            double step = (freq_end - freq_start) / n_lin;
            for (double freq = freq_start; freq <= freq_end; freq += step) {
                analysis->sim_values.push_back(freq);
            }
            break;
        }
        default:
            qDebug() << "parseAC() analysis_type error.";
            break;
    }

    analyses.push_back(analysis);
}

void Netlist::parseTran(double step, double stop_time, double start_time) {
    // qDebug() << "parseTran() step: " << step;
    // qDebug() << "parseTran() stop_time: " << stop_time;
    // qDebug() << "parseTran() start_time: " << start_time;
    Analysis* analysis = new Analysis();

    analysis->analysis_type = ANALYSIS_TRAN;
    analysis->sim_name = "time / s";
    analysis->step = step;

    for (double time = start_time; time <= stop_time; time += step) {
        analysis->sim_values.push_back(time);  // 保存仿真时间点
    }

    analyses.push_back(analysis);
}

void Netlist::parsePrint(int analysis_type,
                         const std::vector<Variable>& var_list) {
    Output* output = new Output();
    output->output_type = ANALYSIS_PRINT;
    output->analysis_type = analysis_type;
    output->var_list = var_list;
    outputs.push_back(output);
}

void Netlist::parsePlot(int analysis_type,
                        const std::vector<Variable>& var_list) {
    Output* output = new Output();
    output->output_type = ANALYSIS_PLOT;
    output->analysis_type = analysis_type;
    output->var_list = var_list;
    outputs.push_back(output);
}
