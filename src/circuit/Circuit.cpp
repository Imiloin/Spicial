#include "Circuit.h"
#include <QDebug>

Circuit::Circuit() {
    // add ground node, name is "0"
    std::string gnd = "0";
    nodes.push_back(gnd);

    MNA_DC_T = nullptr;
    RHS_DC_T = nullptr;
    MNA_AC_T = nullptr;
    RHS_AC_T = nullptr;
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

int Circuit::getNodeNum() const {
    return nodes.size();
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
                (*MNA)(id_branch, id_branch) -= 0 * inductance;
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

                (*MNA)(id_nplus, id_branch) += 1;
                (*MNA)(id_nminus, id_branch) -= 1;
                (*MNA)(id_branch, id_nplus) += 1;
                (*MNA)(id_branch, id_nminus) -= 1;
                (*MNA)(id_branch, id_ncplus) -= gain;
                (*MNA)(id_branch, id_ncminus) += gain;
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
        double ac_magnitude =
            dynamic_cast<VoltageSource*>(voltage_source)->getACMagnitude();
        double ac_phase =
            dynamic_cast<VoltageSource*>(voltage_source)->getACPhase() / 180 *
            M_PI;
        int id_branch = getBranchIndex(voltage_source->getName()) + node_num;

        (*RHS_AC_T)(id_branch) = ac_magnitude * exp(j * ac_phase);
    }
    for (CurrentSource* current_source : current_sources) {
        int id_nplus = getNodeIndex(
            dynamic_cast<CurrentSource*>(current_source)->getNplus());
        int id_nminus = getNodeIndex(
            dynamic_cast<CurrentSource*>(current_source)->getNminus());
        double ac_magnitude =
            dynamic_cast<CurrentSource*>(current_source)->getACMagnitude();
        double ac_phase =
            dynamic_cast<CurrentSource*>(current_source)->getACPhase() / 180 *
            M_PI;

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

    // Resize the matrices, add additional columns and rows for capacitors
    int new_size = getNodeNum() + getBranchNum() + capacitors.size();
    MNA_TRAN_T->resize(new_size, new_size);
    RHS_TRAN_T->resize(new_size);

    // 生成 Tran 状态 MNA 模板
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
                printf("status: %d\n", status);
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
                printf("status: %d\n", status);
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
            int id_nplus =
                getNodeIndex(dynamic_cast<Capacitor*>(capacitor)->getNplus());
            int id_nminus =
                getNodeIndex(dynamic_cast<Capacitor*>(capacitor)->getNminus());
            double capacitance =
                dynamic_cast<Capacitor*>(capacitor)->getCapacitance();

            MNA_AC(id_nplus, id_nplus) += 2 * M_PI * freq * capacitance * j;
            MNA_AC(id_nminus, id_nminus) += 2 * M_PI * freq * capacitance * j;
            MNA_AC(id_nplus, id_nminus) -= 2 * M_PI * freq * capacitance * j;
            MNA_AC(id_nminus, id_nplus) -= 2 * M_PI * freq * capacitance * j;
        }
        for (Inductor* inductor : inductors) {
            double inductance =
                dynamic_cast<Inductor*>(inductor)->getInductance();
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
        arma::vec absx = arma::abs(x);
        iter_results.push_back(absx);
    }
}

void Circuit::TranSimulation(int analysis_type,
                             double step,
                             double stop_time,
                             double start_time) {
    if (MNA_TRAN_T == nullptr || RHS_TRAN_T == nullptr) {
        this->generateTranMNA();
    }
    // to do:::
}

void Circuit::printResults() const {
    auto iter1 = iter_values.begin();
    auto iter2 = iter_results.begin();

    // 迭代结果
    while (iter1 != iter_values.end() && iter2 != iter_results.end()) {
        double value = *iter1;
        arma::vec result = *iter2;

        printf("%s = %e: \n", iter_name.c_str(), value);
        std::cout << result << std::endl;

        ++iter1;
        ++iter2;
    }
}
