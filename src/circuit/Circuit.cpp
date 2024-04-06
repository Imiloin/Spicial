#include "Circuit.h"
#include <QDebug>

Circuit::Circuit(const std::string& file) {
    this->file_path = file;

    // add ground node, name is "0"
    std::string gnd = "0";
    nodes.push_back(gnd);

    simulation_type = -1;

    MNA_DC_T = nullptr;
    RHS_DC_T = nullptr;
    MNA_AC_T = nullptr;
    RHS_AC_T = nullptr;
    MNA_TRAN_T = nullptr;
    RHS_TRAN_T = nullptr;
}

Circuit::~Circuit() {
    for (Component* component : components) {
        delete component;
    }

    delete MNA_DC_T;
    delete RHS_DC_T;
    delete MNA_AC_T;
    delete RHS_AC_T;
    delete MNA_TRAN_T;
    delete RHS_TRAN_T;
}

bool Circuit::hasNode(const std::string& node_name) {  // 判断是否有节点
    return std::find(nodes.begin(), nodes.end(), node_name) != nodes.end();
}

void Circuit::addNode(const std::string& node_name) {  // 添加节点
    nodes.push_back(node_name);
}

int Circuit::getNodeIndex(const std::string& name) {  // 获取节点的编号
    auto it = std::find(nodes.begin(), nodes.end(), name);
    if (it == nodes.end()) {
        qDebug() << "getNodeIndex(" << name.c_str() << ")";
        printf("Node not found\n");
        return -1;
    }
    return std::distance(nodes.begin(), it);
}

int Circuit::getNodeIndexExgnd(
    const std::string& name) {  // 获取节点的编号，不包括地
    // if (!nodes.empty() && nodes.front() == "0") {
    //     nodes.erase(nodes.begin());
    // }
    return getNodeIndex(name) - 1;
}

int Circuit::getNodeNum() const {
    return nodes.size();
}

int Circuit::getNodeNumExgnd() const {
    return nodes.size() - 1;
}

void Circuit::printNodes() const {
    // print {node: index}
    std::cout << std::endl;
    std::cout << "------------------- " << std::endl
              << "Nodes: {name: index}" << std::endl;
    for (std::size_t i = 0; i < nodes.size(); i++) {
        std::cout << "{" << nodes[i] << ": " << i << "}" << std::endl;
    }
    std::cout << "------------------- " << std::endl;
}

bool Circuit::hasBranch(const std::string& branch_name) {
    return std::find(branches.begin(), branches.end(), branch_name) !=
           branches.end();
}

void Circuit::addBranch(const std::string& branch_name) {
    branches.push_back(branch_name);
}

int Circuit::getBranchIndex(const std::string& name) {
    auto it = std::find(branches.begin(), branches.end(), name);
    if (it == branches.end()) {
        qDebug() << "getBranchIndex(" << name.c_str() << ")";
        printf("Branch not found\n");
        return -1;
    }
    return std::distance(branches.begin(), it);
}

int Circuit::getBranchNum() const {
    return branches.size();
}

void Circuit::printBranches() const {
    // print {branch: index}
    std::cout << std::endl;
    std::cout << "------------------- " << std::endl
              << "Branches: {name: index}" << std::endl;
    for (std::size_t i = 0; i < branches.size(); i++) {
        std::cout << "{" << branches[i] << ": " << i << "}" << std::endl;
    }
    std::cout << "------------------- " << std::endl;
}

void Circuit::addComponent(Component* component) {
    components.push_back(component);
}

Component* Circuit::getComponentPtr(const std::string& name) {
    for (Component* component : components) {
        if (component->getName() == name) {
            return component;
        }
    }
    return nullptr;
}

void Circuit::printSize() const {
    std::cout << "Nodes: " << nodes.size() << std::endl;
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
}

double Circuit::calcFunctionAtTime(const Function* func,
                                   double time,
                                   double tstep,
                                   double tstop) {
    // 默认func不为空
    if (time < 0) {
        qDebug() << "!calcFunctionAtTime() time < 0";
        return 0;
    } else if (time > tstop) {
        qDebug() << "!calcFunctionAtTime() time > tstop";
        return 0;
    }
    switch (func->type) {
        case TOKEN_FUNC_SIN: {
            double vo = func->values[0];
            double va = func->values[1];
            double freq =
                std::max(func->values[2], 1 / tstop);  // default 1/tstop
            double td = func->values[3];               // default 0
            double theta = func->values[4];            // default 0
            double phi = func->values[5];              // default 0

            if (time <= td) {
                return vo + va * sin(2 * M_PI * phi / 360);
            } else {  // td < time <= tstop
                return vo +
                       va * exp(-theta * (time - td)) *
                           sin(2 * M_PI * (freq * (time - td) + phi / 360));
            }
        }

        case TOKEN_FUNC_PULSE: {
            double v1 = func->values[0];
            double v2 = func->values[1];
            double td = func->values[2];                    // default 0
            double tr = std::max(func->values[3], tstep);   // default tstep
            double tf = std::max(func->values[4], tstep);   // default tstep
            double pw = std::max(func->values[5], tstop);   // default tstop
            double per = std::max(func->values[6], tstop);  // default tstop

            time = fmod(time, per);
            if (time <= td) {
                return v1;
            } else if (time <= td + tr) {
                return v1 + (v2 - v1) / tr * (time - td);
            } else if (time <= td + tr + pw) {
                return v2;
            } else if (time <= td + tr + pw + tf) {
                return v2 + (v1 - v2) / tf * (time - td - tr - pw);
            } else {  // td + tr + pw + tf < time <= tstop
                return v1;
            }
        }
            // add more function types here

        default:
            qDebug() << "!No such function type";
            return -1;
    }
}

void Circuit::parseResistor(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            double resistance) {
    if (!resistor_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseResistor(" << name.c_str() << ")";
        std::cerr << "Parse error: Resistor " << name << " already exists.\n";
        // throw std::invalid_argument("Resistor already exists.");
        return;
    }

    Resistor* resistor = new Resistor(name, nplus, nminus, resistance);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    this->addComponent(resistor);
}

void Circuit::parseCapacitor(const std::string& name,
                             const std::string& nplus,
                             const std::string& nminus,
                             double capacitance,
                             double initial_voltage) {
    if (!capacitor_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseCapacitor(" << name.c_str() << ")";
        std::cerr << "Parse error: Capacitor " << name << " already exists.\n";
        // throw std::invalid_argument("Capacitor already exists.");
        return;
    }

    Capacitor* capacitor =
        new Capacitor(name, nplus, nminus, capacitance, initial_voltage);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    // branches.push_back(name);  // only add to branches when using tran
    // simulation
    this->addComponent(capacitor);
    this->addCapacitor(capacitor);
}

void Circuit::addCapacitor(Capacitor* capacitor) {
    capacitors.push_back(capacitor);
}

void Circuit::parseInductor(const std::string& name,
                            const std::string& nplus,
                            const std::string& nminus,
                            double inductance,
                            double initial_current) {
    if (!inductor_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseInductor(" << name.c_str() << ")";
        std::cerr << "Parse error: Inductor " << name << " already exists.\n";
        // throw std::invalid_argument("Inductor already exists.");
        return;
    }

    Inductor* inductor =
        new Inductor(name, nplus, nminus, inductance, initial_current);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    branches.push_back(name);
    this->addComponent(inductor);
    this->addInductor(inductor);
}

void Circuit::addInductor(Inductor* inductor) {
    inductors.push_back(inductor);
}

void Circuit::parseVCVS(const std::string& name,
                        const std::string& nplus,
                        const std::string& nminus,
                        const std::string& ncplus,
                        const std::string& ncminus,
                        double gain) {
    if (!vcvs_name_set.insert(name).second) {  // if already exists in the set
        qDebug() << "parseVCVS(" << name.c_str() << ")";
        std::cerr << "Parse error: VCVS " << name << " already exists.\n";
        // throw std::invalid_argument("VCVS already exists.");
        return;
    }

    VCVS* vcvs = new VCVS(name, nplus, nminus, ncplus, ncminus, gain);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    if (!hasNode(ncplus)) {
        addNode(ncplus);
    }
    if (!hasNode(ncminus)) {
        addNode(ncminus);
    }
    branches.push_back(name);
    this->addComponent(vcvs);
}

void Circuit::parseCCCS(const std::string& name,
                        const std::string& nplus,
                        const std::string& nminus,
                        const std::string& vsource,
                        double gain) {
    if (!cccs_name_set.insert(name).second) {  // if already exists in the set
        qDebug() << "parseCCCS(" << name.c_str() << ")";
        std::cerr << "Parse error: CCCS " << name << " already exists.\n";
        // throw std::invalid_argument("CCCS already exists.");
        return;
    }
    if (!hasBranch(vsource)) {
        std::cerr << "Parse error: CCCS " << name << " source not found.\n";
        return;
    }

    CCCS* cccs = new CCCS(name, nplus, nminus, vsource, gain);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    this->addComponent(cccs);
}

void Circuit::parseVCCS(const std::string& name,
                        const std::string& nplus,
                        const std::string& nminus,
                        const std::string& ncplus,
                        const std::string& ncminus,
                        double gain) {
    if (!vccs_name_set.insert(name).second) {  // if already exists in the set
        qDebug() << "parseVCCS(" << name.c_str() << ")";
        std::cerr << "Parse error: VCCS " << name << " already exists.\n";
        // throw std::invalid_argument("VCCS already exists.");
        return;
    }

    VCCS* vccs = new VCCS(name, nplus, nminus, ncplus, ncminus, gain);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    if (!hasNode(ncplus)) {
        addNode(ncplus);
    }
    if (!hasNode(ncminus)) {
        addNode(ncminus);
    }
    this->addComponent(vccs);
}

void Circuit::parseCCVS(const std::string& name,
                        const std::string& nplus,
                        const std::string& nminus,
                        const std::string& vsource,
                        double gain) {
    if (!ccvs_name_set.insert(name).second) {  // if already exists in the set
        qDebug() << "parseCCVS(" << name.c_str() << ")";
        std::cerr << "Parse error: CCVS " << name << " already exists.\n";
        // throw std::invalid_argument("CCVS already exists.");
        return;
    }
    if (!hasBranch(vsource)) {
        std::cerr << "Parse error: CCVS " << name << " source not found.\n";
        return;
    }

    CCVS* ccvs = new CCVS(name, nplus, nminus, vsource, gain);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    branches.push_back(name);
    this->addComponent(ccvs);
}

void Circuit::parseVoltageSource(const std::string& name,
                                 const std::string& nplus,
                                 const std::string& nminus,
                                 double dc_voltage,
                                 double ac_magnitude,
                                 double ac_phase) {
    if (!voltagesource_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseVoltageSource(" << name.c_str() << ")";
        std::cerr << "Parse error: Voltage source " << name
                  << " already exists.\n";
        // throw std::invalid_argument("Voltage source already exists.");
        return;
    }

    VoltageSource* voltage_source = new VoltageSource(
        name, nplus, nminus, dc_voltage, ac_magnitude, ac_phase);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    // qDebug() << "parseVoltageSource() name: " << name.c_str();
    branches.push_back(name);
    this->addComponent(voltage_source);
    this->addVoltageSource(voltage_source);
}
// 重载 parseVoltageSource() 函数，含有 Function 参数
void Circuit::parseVoltageSource(const std::string& name,
                                 const std::string& nplus,
                                 const std::string& nminus,
                                 const Function& func) {
    if (!voltagesource_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseVoltageSource(" << name.c_str() << ")";
        std::cerr << "Parse error: Voltage source " << name
                  << " already exists.\n";
        // throw std::invalid_argument("Voltage source already exists.");
        return;
    }

    VoltageSource* voltage_source =
        new VoltageSource(name, nplus, nminus, 0, 0, 0);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    voltage_source->setFunction(&func);
    // qDebug() << "parseVoltageSource() name: " << name.c_str();
    branches.push_back(name);
    this->addComponent(voltage_source);
    this->addVoltageSource(voltage_source);
}

void Circuit::addVoltageSource(VoltageSource* voltage_source) {
    voltage_sources.push_back(voltage_source);
}

void Circuit::parseCurrentSource(const std::string& name,
                                 const std::string& nplus,
                                 const std::string& nminus,
                                 double dc_current,
                                 double ac_magnitude,
                                 double ac_phase) {
    if (!currentsource_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseCurrentSource(" << name.c_str() << ")";
        std::cerr << "Parse error: Current source " << name
                  << " already exists.\n";
        // throw std::invalid_argument("Current source already exists.");
        return;
    }

    CurrentSource* current_source = new CurrentSource(
        name, nplus, nminus, dc_current, ac_magnitude, ac_phase);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    // qDebug() << "parseCurrentSource() name: " << name.c_str();
    this->addComponent(current_source);
    this->addCurrentSource(current_source);
}
// 重载 parseCurrentSource() 函数，含有 Function 参数
void Circuit::parseCurrentSource(const std::string& name,
                                 const std::string& nplus,
                                 const std::string& nminus,
                                 const Function& func) {
    if (!currentsource_name_set.insert(name)
             .second) {  // if already exists in the set
        qDebug() << "parseCurrentSource(" << name.c_str() << ")";
        std::cerr << "Parse error: Current source " << name
                  << " already exists.\n";
        // throw std::invalid_argument("Current source already exists.");
        return;
    }

    CurrentSource* current_source =
        new CurrentSource(name, nplus, nminus, 0, 0, 0);
    if (!hasNode(nplus)) {
        addNode(nplus);
    }
    if (!hasNode(nminus)) {
        addNode(nminus);
    }
    current_source->setFunction(&func);
    // qDebug() << "parseCurrentSource() name: " << name.c_str();
    this->addComponent(current_source);
    this->addCurrentSource(current_source);
}

void Circuit::addCurrentSource(CurrentSource* current_source) {
    current_sources.push_back(current_source);
}

void Circuit::generateDCMNA() {
    // 生成 DC 状态 MNA 模板
    int node_num = getNodeNum();
    qDebug() << "generateDCMNA() node_num: " << node_num;
    int branch_num = getBranchNum();
    qDebug() << "generateDCMNA() branch_num: " << branch_num;
    int matrix_size = node_num + branch_num;
    arma::sp_mat* MNA =
        new arma::sp_mat(matrix_size, matrix_size);  // create sparse MNA
    arma::vec* RHS =
        new arma::vec(arma::zeros<arma::vec>(matrix_size));  // create RHS
    qDebug() << "generateDCMNA() matrix_size: " << matrix_size;
    for (Component* component : components) {
        switch (component->getType()) {
            case COMPONENT_RESISTOR: {
                int id_nplus = getNodeIndex(
                    dynamic_cast<Resistor*>(component)->getNplus());
                int id_nminus = getNodeIndex(
                    dynamic_cast<Resistor*>(component)->getNminus());
                double resistance =
                    dynamic_cast<Resistor*>(component)->getResistance();

                (*MNA)(id_nplus, id_nplus) += 1 / resistance;
                (*MNA)(id_nminus, id_nminus) += 1 / resistance;
                (*MNA)(id_nplus, id_nminus) -= 1 / resistance;
                (*MNA)(id_nminus, id_nplus) -= 1 / resistance;
                break;
            }
            case COMPONENT_CAPACITOR: {
                // do nothing
                int id_nplus = getNodeIndex(
                    dynamic_cast<Capacitor*>(component)->getNplus());
                int id_nminus = getNodeIndex(
                    dynamic_cast<Capacitor*>(component)->getNminus());
                double capacitance =
                    dynamic_cast<Capacitor*>(component)->getCapacitance();
                // int id_branch = getBranchIndex(component->getName()) +
                // node_num;

                (*MNA)(id_nplus, id_nplus) += 0 * capacitance;
                (*MNA)(id_nminus, id_nminus) += 0 * capacitance;
                (*MNA)(id_nplus, id_nminus) -= 0 * capacitance;
                (*MNA)(id_nminus, id_nplus) -= 0 * capacitance;
                break;
            }
            case COMPONENT_INDUCTOR: {
                int id_nplus = getNodeIndex(
                    dynamic_cast<Inductor*>(component)->getNplus());
                int id_nminus = getNodeIndex(
                    dynamic_cast<Inductor*>(component)->getNminus());
                double inductance =
                    dynamic_cast<Inductor*>(component)->getInductance();
                int id_branch = getBranchIndex(component->getName()) + node_num;

                (*MNA)(id_nplus, id_branch) = 1;
                (*MNA)(id_nminus, id_branch) = -1;
                (*MNA)(id_branch, id_nplus) = 1;
                (*MNA)(id_branch, id_nminus) = -1;
                (*MNA)(id_branch, id_branch) = -0 * inductance;
                break;
            }
            case COMPONENT_VCVS: {
                int id_nplus =
                    getNodeIndex(dynamic_cast<VCVS*>(component)->getNplus());
                int id_nminus =
                    getNodeIndex(dynamic_cast<VCVS*>(component)->getNminus());
                int id_ncplus =
                    getNodeIndex(dynamic_cast<VCVS*>(component)->getNCplus());
                int id_ncminus =
                    getNodeIndex(dynamic_cast<VCVS*>(component)->getNCminus());
                double gain = dynamic_cast<VCVS*>(component)->getGain();
                int id_branch = getBranchIndex(component->getName()) + node_num;

                (*MNA)(id_nplus, id_branch) = 1;
                (*MNA)(id_nminus, id_branch) = -1;
                (*MNA)(id_branch, id_nplus) = 1;
                (*MNA)(id_branch, id_nminus) = -1;
                (*MNA)(id_branch, id_ncplus) = -gain;
                (*MNA)(id_branch, id_ncminus) = gain;
                break;
            }
            case COMPONENT_CCCS: {
                int id_nplus =
                    getNodeIndex(dynamic_cast<CCCS*>(component)->getNplus());
                int id_nminus =
                    getNodeIndex(dynamic_cast<CCCS*>(component)->getNminus());
                double gain = dynamic_cast<CCCS*>(component)->getGain();
                int id_vsource =
                    getBranchIndex(
                        dynamic_cast<CCCS*>(component)->getVsource()) +
                    node_num;

                (*MNA)(id_nplus, id_vsource) += gain;
                (*MNA)(id_nminus, id_vsource) -= gain;
                break;
            }
            case COMPONENT_VCCS: {
                int id_nplus =
                    getNodeIndex(dynamic_cast<VCCS*>(component)->getNplus());
                int id_nminus =
                    getNodeIndex(dynamic_cast<VCCS*>(component)->getNminus());
                int id_ncplus =
                    getNodeIndex(dynamic_cast<VCCS*>(component)->getNCplus());
                int id_ncminus =
                    getNodeIndex(dynamic_cast<VCCS*>(component)->getNCminus());
                double gain = dynamic_cast<VCCS*>(component)->getGain();

                (*MNA)(id_nplus, id_ncplus) += gain;
                (*MNA)(id_nplus, id_ncminus) -= gain;
                (*MNA)(id_nminus, id_ncplus) -= gain;
                (*MNA)(id_nminus, id_ncminus) += gain;
                break;
            }
            case COMPONENT_CCVS: {
                int id_nplus =
                    getNodeIndex(dynamic_cast<CCVS*>(component)->getNplus());
                int id_nminus =
                    getNodeIndex(dynamic_cast<CCVS*>(component)->getNminus());
                double gain = dynamic_cast<CCVS*>(component)->getGain();
                int id_vsource =
                    getBranchIndex(
                        dynamic_cast<CCVS*>(component)->getVsource()) +
                    node_num;
                int id_branch = getBranchIndex(component->getName()) + node_num;

                (*MNA)(id_branch, id_nplus) = 1;
                (*MNA)(id_branch, id_nminus) = -1;
                (*MNA)(id_nplus, id_branch) = 1;
                (*MNA)(id_nminus, id_branch) = -1;
                (*MNA)(id_branch, id_vsource) = -gain;
                break;
            }
            case COMPONENT_VOLTAGE_SOURCE: {
                int id_nplus = getNodeIndex(
                    dynamic_cast<VoltageSource*>(component)->getNplus());
                int id_nminus = getNodeIndex(
                    dynamic_cast<VoltageSource*>(component)->getNminus());
                double dc_voltage =
                    dynamic_cast<VoltageSource*>(component)->getDCVoltage();
                int id_branch = getBranchIndex(component->getName()) + node_num;

                (*MNA)(id_nplus, id_branch) = 1;
                (*MNA)(id_nminus, id_branch) = -1;
                (*MNA)(id_branch, id_nplus) = 1;
                (*MNA)(id_branch, id_nminus) = -1;
                (*RHS)(id_branch) = dc_voltage;
                break;
            }
            case COMPONENT_CURRENT_SOURCE: {
                int id_nplus = getNodeIndex(
                    dynamic_cast<CurrentSource*>(component)->getNplus());
                int id_nminus = getNodeIndex(
                    dynamic_cast<CurrentSource*>(component)->getNminus());
                double dc_current =
                    dynamic_cast<CurrentSource*>(component)->getDCCurrent();

                (*RHS)(id_nplus) = -dc_current;
                (*RHS)(id_nminus) = dc_current;
                break;
            }
            default: {
                break;
            }
        }
    }
    /** 保存 MNA 模板 */
    // 未考虑 analysis 语句的模板
    MNA_DC_T = MNA;
    RHS_DC_T = RHS;
    // arma::vec x = arma::spsolve(MNA, RHS);
}

void Circuit::generateACMNA() {
    if (MNA_DC_T == nullptr || RHS_DC_T == nullptr) {
        qDebug() << "generateACMNA(), DC MNA not generated.";
        this->generateDCMNA();
    }
    int node_num = getNodeNum();
    std::complex<double> j(0, 1);
    // 生成 AC 状态 MNA，复制 DC 状态 MNA，将虚部置零
    arma::sp_mat MNA_zerofill = arma::sp_mat(size((*MNA_DC_T)));
    MNA_AC_T = new arma::sp_cx_mat((*MNA_DC_T), MNA_zerofill);
    arma::vec RHS_zerofill = arma::vec(size((*RHS_DC_T)), arma::fill::zeros);
    RHS_AC_T = new arma::cx_vec((*RHS_DC_T), RHS_zerofill);

    // 生成 AC 状态 MNA 模板
    for (VoltageSource* voltage_source : voltage_sources) {
        double ac_magnitude = voltage_source->getACMagnitude();
        double ac_phase = voltage_source->getACPhase() / 180 * M_PI;
        int id_branch = getBranchIndex(voltage_source->getName()) + node_num;

        (*RHS_AC_T)(id_branch) = ac_magnitude * exp(j * ac_phase);
    }
    for (CurrentSource* current_source : current_sources) {
        int id_nplus = getNodeIndex(current_source->getNplus());
        int id_nminus = getNodeIndex(current_source->getNminus());
        double ac_magnitude = current_source->getACMagnitude();
        double ac_phase = current_source->getACPhase() / 180 * M_PI;

        (*RHS_AC_T)(id_nplus) = -ac_magnitude * exp(j * ac_phase);
        (*RHS_AC_T)(id_nminus) = ac_magnitude * exp(j * ac_phase);
    }
}

void Circuit::generateTranMNA() {
    if (MNA_DC_T == nullptr || RHS_DC_T == nullptr) {
        qDebug() << "generateTranMNA(), DC MNA not generated.";
        this->generateDCMNA();
    }
    // 生成 Tran 状态 MNA，复制 DC 状态 MNA
    MNA_TRAN_T = new arma::sp_mat(*MNA_DC_T);
    RHS_TRAN_T = new arma::vec(*RHS_DC_T);
    // (*MNA_TRAN_T).print("MNA_TRAN_T, before resize");

    // Resize the matrices, add additional columns and rows for capacitors
    int new_size = getNodeNum() + getBranchNum() + capacitors.size();
    MNA_TRAN_T->resize(new_size, new_size);
    RHS_TRAN_T->resize(new_size);
    // (*MNA_TRAN_T).print("MNA_TRAN_T, after resize");

    // 生成 Tran 状态 MNA 模板
    // 为 Capacitor 添加额外的 branch
    for (Capacitor* capacitor : capacitors) {
        branches.push_back(capacitor->getName());  // only add to branches when
                                                   // using tran simulation
        int id_nplus = getNodeIndex(capacitor->getNplus());
        int id_nminus = getNodeIndex(capacitor->getNminus());
        int id_branch = getBranchIndex(capacitor->getName()) + getNodeNum();

        (*MNA_TRAN_T)(id_nplus, id_branch) = 1;
        (*MNA_TRAN_T)(id_nminus, id_branch) = -1;
        (*MNA_TRAN_T)(id_branch, id_branch) = -1;
    }
}

void Circuit::printMNADCTemplate() const {
    if (MNA_DC_T == nullptr || RHS_DC_T == nullptr) {
        qDebug() << "printMNADCTemplate(), DC MNA not generated.";
        return;
    }
    arma::sp_mat MNA = *MNA_DC_T;
    arma::vec RHS = *RHS_DC_T;
    MNA.print("MNA_DC_template (including ground):");
    RHS.print("RHS_DC_template (including ground):");
    // 删除第 0 行和第 0 列
    MNA.shed_row(0);
    MNA.shed_col(0);
    MNA.print("MNA_DC_template (excluding ground):");
    RHS.shed_row(0);
    RHS.print("RHS_DC_template (excluding ground):");
}

void Circuit::printMNAACTemplate() const {
    if (MNA_AC_T == nullptr || RHS_AC_T == nullptr) {
        qDebug() << "printMNAACTemplate(), AC MNA not generated.";
        return;
    }
    arma::sp_cx_mat MNA = *MNA_AC_T;
    arma::cx_vec RHS = *RHS_AC_T;
    MNA.print("MNA_AC_template (including ground):");
    RHS.print("RHS_AC_template (including ground):");
    // 删除第 0 行和第 0 列
    MNA.shed_row(0);
    MNA.shed_col(0);
    MNA.print("MNA_AC_template (excluding ground):");
    RHS.shed_row(0);
    RHS.print("RHS_AC_template (excluding ground):");
}

void Circuit::printMNATranTemplate() const {
    if (MNA_TRAN_T == nullptr || RHS_TRAN_T == nullptr) {
        qDebug() << "printMNATranTemplate(), Tran MNA not generated.";
        return;
    }
    arma::sp_mat MNA = *MNA_TRAN_T;
    arma::vec RHS = *RHS_TRAN_T;
    MNA.print("MNA_Tran_template (including ground):");
    RHS.print("RHS_Tran_template (including ground):");
    // 删除第 0 行和第 0 列
    MNA.shed_row(0);
    MNA.shed_col(0);
    MNA.print("MNA_Tran_template (excluding ground):");
    RHS.shed_row(0);
    RHS.print("RHS_Tran_template (excluding ground):");
}

void Circuit::DCSimulation(int analysis_type,
                           const std::string& source,
                           double start,
                           double end,
                           double increment) {
    if (MNA_DC_T == nullptr || RHS_DC_T == nullptr) {
        this->generateDCMNA();
    }
    for (double iter = start; iter <= end; iter += increment) {
        iter_values.push_back(iter);
    }
    simulation_type = ANALYSIS_DC;

    qDebug() << "DCSimulation() analysis_type: " << analysis_type;
    switch (analysis_type) {
        case (COMPONENT_VOLTAGE_SOURCE): {
            qDebug() << "DCSimulation() source: " << source.c_str();
            iter_name = "voltage(" + source + ")";
            int id_vsrc = getBranchIndex(source) + getNodeNum();

            for (double voltage : iter_values) {
                arma::sp_mat MNA_DC = *MNA_DC_T;
                arma::vec RHS_DC = *RHS_DC_T;
                RHS_DC(id_vsrc) = voltage;

                // exclude ground node
                MNA_DC.shed_row(0);
                MNA_DC.shed_col(0);
                RHS_DC.shed_row(0);

                // check if the matrix is singular
                if (arma::det(arma::mat(MNA_DC)) == 0) {
                    qDebug()
                        << "The matrix is singular after shedding the first "
                           "row and column, cannot solve the system.";
                    continue;
                }

                // qDebug() << "Source: " << source.c_str()
                //          << "at voltage: " << voltage;
                // MNA_DC.print("MNA_DC");
                // RHS_DC.print("RHS_DC");

                arma::vec x;
                bool status = arma::spsolve(x, MNA_DC, RHS_DC);
                // printf("status: %d\n", status);
                if (!status) {
                    qDebug() << "DCSimulation() solve failed.";
                } else {
                    // x.print("DCSimulation() x:");
                }
                iter_results.push_back(x);
            }
            break;
        }
        case (COMPONENT_CURRENT_SOURCE): {
            qDebug() << "DCSimulation() source: " << source.c_str();
            iter_name = "current(" + source + ")";
            Component* current_source = getComponentPtr(source);
            if (current_source == nullptr) {
                qDebug() << "DCSimulation() current source not found.";
                return;
            }
            int id_nplus = getNodeIndex(
                dynamic_cast<CurrentSource*>(current_source)->getNplus());
            int id_nminus = getNodeIndex(
                dynamic_cast<CurrentSource*>(current_source)->getNminus());

            for (double current : iter_values) {
                arma::sp_mat MNA_DC = *MNA_DC_T;
                arma::vec RHS_DC = *RHS_DC_T;
                RHS_DC(id_nplus) = -current;
                RHS_DC(id_nminus) = current;

                // exclude ground node
                MNA_DC.shed_row(0);
                MNA_DC.shed_col(0);
                RHS_DC.shed_row(0);

                // check if the matrix is singular
                if (arma::det(arma::mat(MNA_DC)) == 0) {
                    qDebug()
                        << "The matrix is singular after shedding the first "
                           "row and column, cannot solve the system.";
                    continue;
                }

                // qDebug() << "Source " << source.c_str()
                //          << "at current: " << current;
                // MNA_DC.print("MNA_DC");
                // RHS_DC.print("RHS_DC");

                arma::vec x;
                bool status = arma::spsolve(x, MNA_DC, RHS_DC);
                // printf("status: %d\n", status);
                if (!status) {
                    qDebug() << "DCSimulation() solve failed.";
                } else {
                    // x.print("DCSimulation() x:");
                }
                iter_results.push_back(x);
            }
            break;
        }
        default:
            break;
    }
}

void Circuit::ACSimulation(int analysis_type,
                           double n,
                           double freq_start,
                           double freq_end) {
    if (MNA_AC_T == nullptr || RHS_AC_T == nullptr) {
        this->generateACMNA();
    }
    simulation_type = ANALYSIS_AC;

    iter_name = "frequency";
    int node_num = getNodeNum();
    std::complex<double> j(0, 1);
    qDebug() << "ACSimulation() analysis_type: " << analysis_type;

    // 获取仿真的频率点
    switch (analysis_type) {
        case (TOKEN_DEC): {
            int n_per_dec = std::round(n);
            double ratio = pow(10.0, 1.0 / n_per_dec);
            for (double freq = freq_start; freq < freq_end; freq *= ratio) {
                iter_values.push_back(freq);
            }
            iter_values.push_back(freq_end);
            break;
        }
        case (TOKEN_OCT): {
            int n_per_oct = std::round(n);
            double ratio = pow(8.0, 1.0 / n_per_oct);
            for (double freq = freq_start; freq < freq_end; freq *= ratio) {
                iter_values.push_back(freq);
            }
            iter_values.push_back(freq_end);
            break;
        }
        case (TOKEN_LIN): {
            int n_lin = std::round(n);
            double step = (freq_end - freq_start) / n_lin;
            for (double freq = freq_start; freq <= freq_end; freq += step) {
                iter_values.push_back(freq);
            }
            break;
        }
        default:
            qDebug() << "ACSimulation() analysis_type error.";
            break;
    }

    // 运行 AC 分析
    for (double freq : iter_values) {
        arma::sp_cx_mat MNA_AC = *MNA_AC_T;
        arma::cx_vec RHS_AC = *RHS_AC_T;

        for (Capacitor* capacitor : capacitors) {
            int id_nplus = getNodeIndex(capacitor->getNplus());
            int id_nminus = getNodeIndex(capacitor->getNminus());
            double capacitance = capacitor->getCapacitance();

            MNA_AC(id_nplus, id_nplus) += 2 * M_PI * freq * capacitance * j;
            MNA_AC(id_nminus, id_nminus) += 2 * M_PI * freq * capacitance * j;
            MNA_AC(id_nplus, id_nminus) -= 2 * M_PI * freq * capacitance * j;
            MNA_AC(id_nminus, id_nplus) -= 2 * M_PI * freq * capacitance * j;
        }
        for (Inductor* inductor : inductors) {
            double inductance = inductor->getInductance();
            int id_branch = getBranchIndex(inductor->getName()) + node_num;

            MNA_AC(id_branch, id_branch) -= 2 * M_PI * freq * inductance * j;
        }

        // exclude ground node
        MNA_AC.shed_row(0);
        MNA_AC.shed_col(0);
        RHS_AC.shed_row(0);

        // check if the matrix is singular
        /*
        arma::cx_mat MNA_AC_dense =
            arma::conv_to<arma::cx_mat>::from(MNA_AC);
        if (std::abs(arma::det(MNA_AC_dense)) < 1e-5) {
            qDebug() << "The matrix MNA_AC is singular.";
            continue;
        }
        */

        // qDebug() << "AC Simulation at frequency: " << freq;
        // MNA_AC.print("MNA_AC");
        // RHS_AC.print("RHS_AC");

        arma::cx_vec x;
        bool status = arma::spsolve(x, MNA_AC, RHS_AC);
        // printf("status: %d\n", status);
        if (!status) {
            qDebug() << "ACSimulation() solve failed.";
        } else {
            // x.print("ACSimulation() x:");
        }
        // arma::vec absx = arma::abs(x);
        iter_cresults.push_back(x);
    }
}

arma::vec Circuit::tranBackEuler(double time,
                                 double h,
                                 double tstep,
                                 double tstop,
                                 const arma::vec x_prev) {
    int node_num = getNodeNum();
    arma::sp_mat MNA_TRAN = *MNA_TRAN_T;
    arma::vec RHS_TRAN = *RHS_TRAN_T;

    for (Capacitor* capacitor : capacitors) {
        int id_nplus = getNodeIndex(capacitor->getNplus());
        int id_nminus = getNodeIndex(capacitor->getNminus());
        double capacitance = capacitor->getCapacitance();
        int id_branch = getBranchIndex(capacitor->getName()) + node_num;

        MNA_TRAN(id_branch, id_nplus) = capacitance / h;
        MNA_TRAN(id_branch, id_nminus) = -capacitance / h;
        RHS_TRAN(id_branch) =
            capacitance / h * (x_prev(id_nplus) - x_prev(id_nminus));
    }

    for (Inductor* inductor : inductors) {
        double inductance = inductor->getInductance();
        int id_branch = getBranchIndex(inductor->getName()) + node_num;

        MNA_TRAN(id_branch, id_branch) = -inductance / h;
        RHS_TRAN(id_branch) = -inductance / h * x_prev(id_branch);
    }

    for (VoltageSource* voltage_source : voltage_sources) {
        if (voltage_source->getFunction() == nullptr) {
            continue;  // 没有 function，直接使用 DC 电压，已在 MNA_TRAN_T
                       // 中存在
        }
        int id_branch = getBranchIndex(voltage_source->getName()) + node_num;

        RHS_TRAN(id_branch) = calcFunctionAtTime(voltage_source->getFunction(),
                                                 time, tstep, tstop);
    }

    for (CurrentSource* current_source : current_sources) {
        if (current_source->getFunction() == nullptr) {
            continue;  // 没有 function，直接使用 DC 电流，已在 MNA_TRAN_T
                       // 中存在
        }
        int id_nplus = getNodeIndex(current_source->getNplus());
        int id_nminus = getNodeIndex(current_source->getNminus());
        double current_time = calcFunctionAtTime(current_source->getFunction(),
                                                 time, tstep, tstop);

        RHS_TRAN(id_nplus) = -current_time;
        RHS_TRAN(id_nminus) = current_time;
    }

    // exclude ground node
    MNA_TRAN.shed_row(0);
    MNA_TRAN.shed_col(0);
    RHS_TRAN.shed_row(0);
    // check if the matrix is singular
    if (arma::det(arma::mat(MNA_TRAN)) == 0) {
        qDebug() << "The matrix is singular after shedding the first "
                    "row and column, cannot solve the system.";
        MNA_TRAN.print("MNA_TRAN");
        RHS_TRAN.print("RHS_TRAN");
    }

    arma::vec x;
    bool status = arma::spsolve(x, MNA_TRAN, RHS_TRAN);
    // printf("status: %d\n", status);
    if (!status) {
        qDebug() << "TranSimulation() solve failed.";
        return x_prev;
    } else {
        return x;
    }
}

void Circuit::TranSimulation(double step, double stop_time, double start_time) {
    // qDebug() << "TranSimulation() step: " << step;
    // qDebug() << "TranSimulation() stop_time: " << stop_time;
    // qDebug() << "TranSimulation() start_time: " << start_time;
    if (MNA_TRAN_T == nullptr || RHS_TRAN_T == nullptr) {
        this->generateTranMNA();
    }
    simulation_type = ANALYSIS_TRAN;
    double h = step;  // 为简化处理，使用恒定步长
    iter_name = "time";
    int node_num = getNodeNum();
    double time = 0;  // 当前时间点
    arma::vec x;      // 保存当前时间点的解

    if (MNA_TRAN_T == nullptr || RHS_TRAN_T == nullptr) {
        qDebug() << "generateTranMNA() failed.";
    }
    // (*MNA_TRAN_T).print("MNA_TRAN_T:");
    // 先根据初始条件解出第一组解（t = 0） //
    // qDebug() << "Creating MNA_TRAN_0 and RHS_TRAN_0";
    arma::sp_mat* MNA_TRAN_0 = new arma::sp_mat(*MNA_TRAN_T);
    arma::vec* RHS_TRAN_0 = new arma::vec(*RHS_TRAN_T);

    for (Capacitor* capacitor : capacitors) {
        int id_nplus = getNodeIndex(capacitor->getNplus());
        int id_nminus = getNodeIndex(capacitor->getNminus());
        double initial_voltage = capacitor->getInitialVoltage();
        int id_branch = getBranchIndex(capacitor->getName()) + node_num;

        (*MNA_TRAN_0)(id_nplus, id_branch) = 1;
        (*MNA_TRAN_0)(id_nminus, id_branch) = -1;
        (*MNA_TRAN_0)(id_branch, id_nplus) = 1;
        (*MNA_TRAN_0)(id_branch, id_nminus) = -1;
        (*MNA_TRAN_0)(id_branch, id_branch) = 0;
        (*RHS_TRAN_0)(id_branch) = initial_voltage;
    }
    for (Inductor* inductor : inductors) {
        int id_nplus = getNodeIndex(inductor->getNplus());
        int id_nminus = getNodeIndex(inductor->getNminus());
        double initial_current = inductor->getInitialCurrent();
        int id_branch = getBranchIndex(inductor->getName()) + node_num;

        (*MNA_TRAN_0)(id_branch, id_nplus) = 0;
        (*MNA_TRAN_0)(id_branch, id_nminus) = 0;
        (*MNA_TRAN_0)(id_branch, id_branch) = 1;
        (*RHS_TRAN_0)(id_branch) = initial_current;
    }
    for (VoltageSource* voltage_source : voltage_sources) {
        if (voltage_source->getFunction() == nullptr) {
            continue;  // 没有 function，直接使用 DC 电压，已在 MNA_TRAN_T
                       // 中存在
        }
        int id_branch = getBranchIndex(voltage_source->getName()) + node_num;

        (*RHS_TRAN_0)(id_branch) =
            calcFunctionAtTime(voltage_source->getFunction(), 0, h, stop_time);
    }
    for (CurrentSource* current_source : current_sources) {
        if (current_source->getFunction() == nullptr) {
            continue;  // 没有 function，直接使用 DC 电流，已在 MNA_TRAN_T
                       // 中存在
        }
        int id_nplus = getNodeIndex(current_source->getNplus());
        int id_nminus = getNodeIndex(current_source->getNminus());
        double current_0 =
            calcFunctionAtTime(current_source->getFunction(), 0, h, stop_time);

        (*RHS_TRAN_0)(id_nplus) = -current_0;
        (*RHS_TRAN_0)(id_nminus) = current_0;
    }

    // exclude ground node
    (*MNA_TRAN_0).shed_row(0);
    (*MNA_TRAN_0).shed_col(0);
    (*RHS_TRAN_0).shed_row(0);
    // check if the matrix is singular
    if (arma::det(arma::mat(*MNA_TRAN_0)) == 0) {
        qDebug() << "The matrix is singular after shedding the first "
                    "row and column, cannot solve the system.";
        (*MNA_TRAN_0).print("MNA_TRAN");
        (*RHS_TRAN_0).print("RHS_TRAN");
    }

    bool status = arma::spsolve(x, *MNA_TRAN_0, *RHS_TRAN_0);
    // printf("status: %d\n", status);
    if (!status) {
        qDebug() << "TranSimulation() solve failed.";
    } else {
        // x.print("TranSimulation() t=0 x:");
    }
    if (start_time == 0) {
        iter_values.push_back(time);  // 保存仿真时间点
        iter_results.push_back(x);    // time = 0 的解
    }

    delete MNA_TRAN_0;
    delete RHS_TRAN_0;
    // 第一组解求解完毕 //

    // arma::sp_mat* MNA_TRAN = new arma::sp_mat(*MNA_TRAN_T);
    // arma::vec* RHS_TRAN = new arma::vec(*RHS_TRAN_T);
    // 求解 (0, start_time) 之间的解，不含两边 //
    for (time = h; time < start_time; time += h) {
        x = tranBackEuler(time, h, step, stop_time, x);
    }
    time -= h;  // 回退到 start_time 前一个时间点
    // 求解 [time, start_time] 的解 //
    if (time < start_time) {
        double h_last = start_time - time;
        time = start_time;
        x = tranBackEuler(start_time, h_last, step, stop_time, x);
        iter_values.push_back(time);  // 保存仿真时间点
        iter_results.push_back(x);    // start_time 的解
    }
    // 求解 (start_time, stop_time] 的解 //
    std::cout << (time < stop_time) << std::endl;
    for (time += h; time <= stop_time; time += h) {
        x = tranBackEuler(time, h, step, stop_time, x);
        iter_values.push_back(time);  // 保存仿真时间点
        iter_results.push_back(x);    // time 的解
        // std::cout << "time: " << time << "\t";
        // x.print("TranSimulation() x:");
    }
}

void Circuit::printAnalysis(int analysis_type,
                            const std::vector<Variable>& var_list) {
    // create xdata
    xdata = ColumnData{iter_name, iter_values};

    // assert analysis type is correct
    switch (analysis_type) {
        case TOKEN_ANALYSIS_OP: {
            return;
        }
        case TOKEN_ANALYSIS_DC: {
            if (simulation_type != ANALYSIS_DC) {
                qDebug()
                    << "printAnalysis() simulation_type error, DC expected.";
                qDebug() << "Current simulation_type: " << simulation_type;
                return;
            }
            break;
        }
        case TOKEN_ANALYSIS_AC: {
            if (simulation_type != ANALYSIS_AC) {
                qDebug()
                    << "printAnalysis() simulation_type error, AC expected.";
                qDebug() << "Current simulation_type: " << simulation_type;
                return;
            }
            break;
        }
        case TOKEN_ANALYSIS_TRAN: {
            if (simulation_type != ANALYSIS_TRAN) {
                qDebug()
                    << "printAnalysis() simulation_type error, TRAN expected.";
                qDebug() << "Current simulation_type: " << simulation_type;
                return;
            }
            break;
        }
        default: {
            break;
        }
    }

    // create ydata
    for (const auto& var : var_list) {
        for (const auto& node_branch : var.nodes) {
            ColumnData y;
            switch (var.type) {
                case TOKEN_VAR_VOLTAGE_REAL: {
                    if (analysis_type != TOKEN_ANALYSIS_AC) {
                        break;
                    }
                    y.name = "VR(" + node_branch + ")";
                    int id_node = getNodeIndexExgnd(node_branch);
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(real(cresult(id_node)));
                    }
                    break;
                }
                case TOKEN_VAR_VOLTAGE_IMAG: {
                    if (analysis_type != TOKEN_ANALYSIS_AC) {
                        break;
                    }
                    y.name = "VI(" + node_branch + ")";
                    int id_node = getNodeIndexExgnd(node_branch);
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(imag(cresult(id_node)));
                    }
                    break;
                }
                case TOKEN_VAR_VOLTAGE_MAG: {
                    y.name = "V(" + node_branch + ")";
                    int id_node = getNodeIndexExgnd(node_branch);
                    if (analysis_type != TOKEN_ANALYSIS_AC) {
                        for (const auto& result : iter_results) {
                            y.values.push_back(result(id_node));
                        }
                    } else {
                        for (const auto& cresult : iter_cresults) {
                            y.values.push_back(abs(cresult(id_node)));
                        }
                    }
                    break;
                }
                case TOKEN_VAR_VOLTAGE_PHASE: {
                    if (analysis_type != TOKEN_ANALYSIS_AC) {
                        break;
                    }
                    y.name = "VP(" + node_branch + ")";
                    int id_node = getNodeIndexExgnd(node_branch);
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(arg(cresult(id_node)));
                    }
                    break;
                }
                case TOKEN_VAR_VOLTAGE_DB: {
                    if (analysis_type != TOKEN_ANALYSIS_AC) {
                        break;
                    }
                    y.name = "VDB(" + node_branch + ")";
                    int id_node = getNodeIndexExgnd(node_branch);
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(20 * log10(abs(cresult(id_node))));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_REAL: {
                    if (analysis_type != TOKEN_ANALYSIS_AC) {
                        break;
                    }
                    y.name = "IR(" + node_branch + ")";
                    int node_num = getNodeNumExgnd();
                    int id_branch = getBranchIndex(node_branch) + node_num;
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(real(cresult(id_branch)));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_IMAG: {
                    if (analysis_type != TOKEN_ANALYSIS_AC) {
                        break;
                    }
                    y.name = "II(" + node_branch + ")";
                    int node_num = getNodeNumExgnd();
                    int id_branch = getBranchIndex(node_branch) + node_num;
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(imag(cresult(id_branch)));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_MAG: {
                    y.name = "I(" + node_branch + ")";
                    int node_num = getNodeNumExgnd();
                    int id_branch = getBranchIndex(node_branch) + node_num;
                    if (analysis_type != TOKEN_ANALYSIS_AC) {
                        for (const auto& result : iter_results) {
                            y.values.push_back(result(id_branch));
                        }
                    } else {
                        for (const auto& cresult : iter_cresults) {
                            y.values.push_back(abs(cresult(id_branch)));
                        }
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_PHASE: {
                    if (analysis_type != TOKEN_ANALYSIS_AC) {
                        break;
                    }
                    y.name = "IP(" + node_branch + ")";
                    int node_num = getNodeNumExgnd();
                    int id_branch = getBranchIndex(node_branch) + node_num;
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(arg(cresult(id_branch)));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_DB: {
                    if (analysis_type != TOKEN_ANALYSIS_AC) {
                        break;
                    }
                    y.name = "IDB(" + node_branch + ")";
                    int node_num = getNodeNumExgnd();
                    int id_branch = getBranchIndex(node_branch) + node_num;
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(20 * log10(abs(cresult(id_branch))));
                    }
                    break;
                }
                default: {
                    // qDebug << "printAnalysis() No such variable type!";
                    break;
                }
            }
            ydata.push_back(y);
        }
    }

    // print xdata, ydata and export to csv file
    std::filesystem::path p(file_path);
    p.replace_extension(".csv");
    std::string csv_file_path = p.string();
    std::ofstream file(csv_file_path);

    // 打印并写入 CSV 文件的头部
    std::cout << xdata.name;
    file << xdata.name;
    for (const auto& column : ydata) {
        std::cout << "," << column.name;
        file << "," << column.name;
    }
    std::cout << "\n";
    file << "\n";

    // 打印并写入数据
    for (size_t i = 0; i < xdata.values.size(); ++i) {
        std::cout << xdata.values[i];
        file << xdata.values[i];
        for (const auto& column : ydata) {
            std::cout << "," << column.values[i];
            file << "," << column.values[i];
        }
        std::cout << "\n";
        file << "\n";
    }

    file.close();
}

void Circuit::plotAnalysis(int analysis_type,
                           const std::vector<Variable>& var_list) {
    printAnalysis(analysis_type, var_list);
    callPlot(xdata, ydata);
}

void Circuit::printResults() const {
    switch (simulation_type) {
        case ANALYSIS_DC: {
            auto iter1 = iter_values.begin();
            auto iter2 = iter_results.begin();
            // 输出迭代结果
            while (iter1 != iter_values.end() && iter2 != iter_results.end()) {
                double value = *iter1;
                auto result = *iter2;

                printf("%s = %e: \n", iter_name.c_str(), value);
                std::cout << result << std::endl;

                ++iter1;
                ++iter2;
            }
            break;
        }
        case ANALYSIS_AC: {
            auto iter1 = iter_values.begin();
            auto iter2 = iter_cresults.begin();
            // 输出迭代结果（复数）
            while (iter1 != iter_values.end() && iter2 != iter_cresults.end()) {
                double value = *iter1;
                auto result = *iter2;

                printf("%s = %e: \n", iter_name.c_str(), value);
                std::cout << result << std::endl;

                ++iter1;
                ++iter2;
            }
            break;
        }
        case ANALYSIS_TRAN: {
            auto iter1 = iter_values.begin();
            auto iter2 = iter_results.begin();
            // 输出迭代结果
            while (iter1 != iter_values.end() && iter2 != iter_results.end()) {
                double value = *iter1;
                auto result = *iter2;

                printf("%s = %e: \n", iter_name.c_str(), value);
                std::cout << result << std::endl;

                ++iter1;
                ++iter2;
            }
            break;
        }
        default: {
            qDebug() << "printResults() simulation_type error.";
            break;
        }
    }
}
