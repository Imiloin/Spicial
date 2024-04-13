#include "Circuit.h"
#include <QDebug>

Circuit::Circuit(Netlist& netlist_) : netlist(netlist_) {
    this->preProcess();
}

Circuit::~Circuit() {
    // delete simulations
    for (DCSimulation* sim : dc_simulations) {
        delete sim;
    }
    for (ACSimulation* sim : ac_simulations) {
        delete sim;
    }
    for (TranSimulation* sim : tran_simulations) {
        delete sim;
    }
}

void Circuit::preProcess() {
    // add nodes and branches
    // and create node, branch index for components
    // and get model ptr for components
    for (Component* component : netlist.components) {
        switch (component->getType()) {
            case (COMPONENT_RESISTOR): {
                Resistor* resistor = dynamic_cast<Resistor*>(component);
                resistor->id_nplus = nodes.addNode(resistor->nplus);
                resistor->id_nminus = nodes.addNode(resistor->nminus);
                break;
            }
            case (COMPONENT_CAPACITOR): {
                Capacitor* capacitor = dynamic_cast<Capacitor*>(component);
                capacitor->id_nplus = nodes.addNode(capacitor->nplus);
                capacitor->id_nminus = nodes.addNode(capacitor->nminus);
                capacitor->id_branch = branches.addBranch(capacitor->name);
                break;
            }
            case (COMPONENT_INDUCTOR): {
                Inductor* inductor = dynamic_cast<Inductor*>(component);
                inductor->id_nplus = nodes.addNode(inductor->nplus);
                inductor->id_nminus = nodes.addNode(inductor->nminus);
                inductor->id_branch = branches.addBranch(inductor->name);
                break;
            }
            case (COMPONENT_VCVS): {
                VCVS* vcvs = dynamic_cast<VCVS*>(component);
                vcvs->id_nplus = nodes.addNode(vcvs->nplus);
                vcvs->id_nminus = nodes.addNode(vcvs->nminus);
                vcvs->id_ncplus = nodes.addNode(vcvs->ncplus);
                vcvs->id_ncminus = nodes.addNode(vcvs->ncminus);
                vcvs->id_branch = branches.addBranch(vcvs->name);
                break;
            }
            case (COMPONENT_CCCS): {
                CCCS* cccs = dynamic_cast<CCCS*>(component);
                cccs->id_nplus = nodes.addNode(cccs->nplus);
                cccs->id_nminus = nodes.addNode(cccs->nminus);
                break;
            }
            case (COMPONENT_VCCS): {
                VCCS* vccs = dynamic_cast<VCCS*>(component);
                vccs->id_nplus = nodes.addNode(vccs->nplus);
                vccs->id_nminus = nodes.addNode(vccs->nminus);
                vccs->id_ncplus = nodes.addNode(vccs->ncplus);
                vccs->id_ncminus = nodes.addNode(vccs->ncminus);
                break;
            }
            case (COMPONENT_CCVS): {
                CCVS* ccvs = dynamic_cast<CCVS*>(component);
                ccvs->id_nplus = nodes.addNode(ccvs->nplus);
                ccvs->id_nminus = nodes.addNode(ccvs->nminus);
                ccvs->id_branch = branches.addBranch(ccvs->name);
                break;
            }
            case (COMPONENT_VOLTAGE_SOURCE): {
                VoltageSource* voltage_source =
                    dynamic_cast<VoltageSource*>(component);
                voltage_source->id_nplus = nodes.addNode(voltage_source->nplus);
                voltage_source->id_nminus =
                    nodes.addNode(voltage_source->nminus);
                voltage_source->id_branch =
                    branches.addBranch(voltage_source->name);
                break;
            }
            case (COMPONENT_CURRENT_SOURCE): {
                CurrentSource* current_source =
                    dynamic_cast<CurrentSource*>(component);
                current_source->id_nplus = nodes.addNode(current_source->nplus);
                current_source->id_nminus =
                    nodes.addNode(current_source->nminus);
                break;
            }
            case (COMPONENT_DIODE): {
                Diode* diode = dynamic_cast<Diode*>(component);
                diode->id_nplus = nodes.addNode(diode->nplus);
                diode->id_nminus = nodes.addNode(diode->nminus);
                diode->model =
                    dynamic_cast<DiodeModel*>(getModelPtr(diode->modelname));
                break;
            }
        }
    }

    // 更新 branch 索引，加上节点数
    int node_num = nodes.getNodeNum();
    for (Component* component : netlist.components) {
        switch (component->getType()) {
            case (COMPONENT_RESISTOR): {
                break;
            }
            case (COMPONENT_CAPACITOR): {
                Capacitor* capacitor = dynamic_cast<Capacitor*>(component);
                capacitor->id_branch += node_num;
                break;
            }
            case (COMPONENT_INDUCTOR): {
                Inductor* inductor = dynamic_cast<Inductor*>(component);
                inductor->id_branch += node_num;
                break;
            }
            case (COMPONENT_VCVS): {
                VCVS* vcvs = dynamic_cast<VCVS*>(component);
                vcvs->id_branch += node_num;
                break;
            }
            case (COMPONENT_CCCS): {
                break;
            }
            case (COMPONENT_VCCS): {
                break;
            }
            case (COMPONENT_CCVS): {
                CCVS* ccvs = dynamic_cast<CCVS*>(component);
                ccvs->id_branch += node_num;
                break;
            }
            case (COMPONENT_VOLTAGE_SOURCE): {
                VoltageSource* voltage_source =
                    dynamic_cast<VoltageSource*>(component);
                voltage_source->id_branch += node_num;
                break;
            }
            case (COMPONENT_CURRENT_SOURCE): {
                break;
            }
            case (COMPONENT_DIODE): {
                // Diode* diode = dynamic_cast<Diode*>(component);
                break;
            }
        }
    }

    // 创建 MNA, RHS 模板
    this->generateMNATemplate();
    // 更新 Simulation 中的 MNA, RHS 模板(static)
    Simulation test_sim = Simulation(*(netlist.analyses.front()), netlist,
                                     nodes, branches, MNA_T, RHS_T);
}

void Circuit::printNodes() {
    nodes.printNodes();
}

void Circuit::printBranches() {
    branches.printBranches();
}

void Circuit::generateMNATemplate() {
    // 生成 MNA, RHS 模板
    int node_num = nodes.getNodeNum();
    // qDebug() << "generateDCMNA() node_num: " << node_num;
    int branch_num = branches.getBranchNum();
    // qDebug() << "generateDCMNA() branch_num: " << branch_num;
    int matrix_size = node_num + branch_num;
    arma::sp_mat* MNA =
        new arma::sp_mat(matrix_size, matrix_size);  // create sparse MNA
    arma::vec* RHS =
        new arma::vec(arma::zeros<arma::vec>(matrix_size));  // create RHS
    // qDebug() << "generateDCMNA() matrix_size: " << matrix_size;
    for (Component* component : netlist.components) {
        switch (component->getType()) {
            case COMPONENT_RESISTOR: {
                Resistor* resistor = dynamic_cast<Resistor*>(component);
                int id_nplus = resistor->getIdNplus();
                int id_nminus = resistor->getIdNminus();
                double resistance = resistor->getResistance();

                (*MNA)(id_nplus, id_nplus) += 1 / resistance;
                (*MNA)(id_nminus, id_nminus) += 1 / resistance;
                (*MNA)(id_nplus, id_nminus) -= 1 / resistance;
                (*MNA)(id_nminus, id_nplus) -= 1 / resistance;
                break;
            }
            case COMPONENT_CAPACITOR: {
                Capacitor* capacitor = dynamic_cast<Capacitor*>(component);
                int id_nplus = capacitor->getIdNplus();
                int id_nminus = capacitor->getIdNminus();
                int id_branch = capacitor->getIdBranch();
                double capacitance = capacitor->getCapacitance();

                (*MNA)(id_nplus, id_nplus) += 0 * capacitance;
                (*MNA)(id_nminus, id_nminus) += 0 * capacitance;
                (*MNA)(id_nplus, id_nminus) -= 0 * capacitance;
                (*MNA)(id_nminus, id_nplus) -= 0 * capacitance;
                (*MNA)(id_branch, id_nplus) = 0 * capacitance;
                (*MNA)(id_branch, id_nminus) = -0 * capacitance;
                (*MNA)(id_branch, id_branch) = -1;
                break;
            }
            case COMPONENT_INDUCTOR: {
                Inductor* inductor = dynamic_cast<Inductor*>(component);
                int id_nplus = inductor->getIdNplus();
                int id_nminus = inductor->getIdNminus();
                int id_branch = inductor->getIdBranch();
                // double inductance = inductor->getInductance();

                (*MNA)(id_nplus, id_branch) = 1;
                (*MNA)(id_nminus, id_branch) = -1;
                (*MNA)(id_branch, id_nplus) = 1;
                (*MNA)(id_branch, id_nminus) = -1;
                break;
            }
            case COMPONENT_VCVS: {
                VCVS* vcvs = dynamic_cast<VCVS*>(component);
                int id_nplus = vcvs->getIdNplus();
                int id_nminus = vcvs->getIdNminus();
                int id_ncplus = vcvs->getIdNCplus();
                int id_ncminus = vcvs->getIdNCminus();
                int id_branch = vcvs->getIdBranch();
                double gain = vcvs->getGain();

                (*MNA)(id_nplus, id_branch) = 1;
                (*MNA)(id_nminus, id_branch) = -1;
                (*MNA)(id_branch, id_nplus) = 1;
                (*MNA)(id_branch, id_nminus) = -1;
                (*MNA)(id_branch, id_ncplus) = -gain;
                (*MNA)(id_branch, id_ncminus) = gain;
                break;
            }
            case COMPONENT_CCCS: {
                CCCS* cccs = dynamic_cast<CCCS*>(component);
                int id_nplus = cccs->getIdNplus();
                int id_nminus = cccs->getIdNminus();
                int id_vsource =
                    branches.getBranchIndex(cccs->getName()) + node_num;
                double gain = cccs->getGain();

                (*MNA)(id_nplus, id_vsource) += gain;
                (*MNA)(id_nminus, id_vsource) -= gain;
                break;
            }
            case COMPONENT_VCCS: {
                VCCS* vccs = dynamic_cast<VCCS*>(component);
                int id_nplus = vccs->getIdNplus();
                int id_nminus = vccs->getIdNminus();
                int id_ncplus = vccs->getIdNCplus();
                int id_ncminus = vccs->getIdNCminus();
                double gain = vccs->getGain();

                (*MNA)(id_nplus, id_ncplus) += gain;
                (*MNA)(id_nplus, id_ncminus) -= gain;
                (*MNA)(id_nminus, id_ncplus) -= gain;
                (*MNA)(id_nminus, id_ncminus) += gain;
                break;
            }
            case COMPONENT_CCVS: {
                CCVS* ccvs = dynamic_cast<CCVS*>(component);
                int id_nplus = ccvs->getIdNplus();
                int id_nminus = ccvs->getIdNminus();
                int id_branch = ccvs->getIdBranch();
                double gain = ccvs->getGain();
                int id_vsource =
                    branches.getBranchIndex(ccvs->getVsource()) + node_num;

                (*MNA)(id_branch, id_nplus) = 1;
                (*MNA)(id_branch, id_nminus) = -1;
                (*MNA)(id_nplus, id_branch) = 1;
                (*MNA)(id_nminus, id_branch) = -1;
                (*MNA)(id_branch, id_vsource) = -gain;
                break;
            }
            case COMPONENT_VOLTAGE_SOURCE: {
                VoltageSource* voltage_source =
                    dynamic_cast<VoltageSource*>(component);
                int id_nplus = voltage_source->getIdNplus();
                int id_nminus = voltage_source->getIdNminus();
                int id_branch = voltage_source->getIdBranch();
                double dc_voltage = voltage_source->getDCVoltage();

                (*MNA)(id_nplus, id_branch) = 1;
                (*MNA)(id_nminus, id_branch) = -1;
                (*MNA)(id_branch, id_nplus) = 1;
                (*MNA)(id_branch, id_nminus) = -1;
                (*RHS)(id_branch) = dc_voltage;
                break;
            }
            case COMPONENT_CURRENT_SOURCE: {
                CurrentSource* current_source =
                    dynamic_cast<CurrentSource*>(component);
                int id_nplus = current_source->getIdNplus();
                int id_nminus = current_source->getIdNminus();
                double dc_current = current_source->getDCCurrent();

                (*RHS)(id_nplus) = -dc_current;
                (*RHS)(id_nminus) = dc_current;
                break;
            }
            case COMPONENT_DIODE: {
                Diode* diode = dynamic_cast<Diode*>(component);
                int id_nplus = diode->getIdNplus();
                int id_nminus = diode->getIdNminus();
                DiodeModel* model = diode->getModel();

                double v0 = 0;  // 从0V开始迭代
                double i0 = model->calcCurrentAtVoltage(v0);
                double g0 = model->calcConductanceAtVoltage(v0);
                double j0 = i0 - g0 * v0;

                (*MNA)(id_nplus, id_nplus) += g0;
                (*MNA)(id_nminus, id_nminus) += g0;
                (*MNA)(id_nplus, id_nminus) -= g0;
                (*MNA)(id_nminus, id_nplus) -= g0;
                (*RHS)(id_nplus) -= j0;
                (*RHS)(id_nminus) += j0;
                break;
            }
            default: {
                qDebug() << "generateMNA() Unknown component type:"
                         << component->getType();
                break;
            }
        }
    }
    /** 保存 MNA 模板 */
    // 未考虑 analysis 语句的模板，若为静态工作点分析，求解该方程即可
    MNA_T = MNA;
    RHS_T = RHS;
    // arma::vec x = arma::spsolve(MNA, RHS);
}

void Circuit::printMNATemplate() {
    if (MNA_T == nullptr || RHS_T == nullptr) {
        qDebug() << "MNA_T or RHS_T is nullptr.";
        return;
    }
    std::cout << "-------------------------------" << std::endl;
    (arma::mat(*MNA_T)).print("MNA_T");
    (*RHS_T).print("RHS_T");
    std::cout << "-------------------------------" << std::endl;
}

// already defined in Netlist class
Component* Circuit::getComponentPtr(const std::string& name) {
    auto it = std::find_if(
        netlist.components.begin(), netlist.components.end(),
        [&name](Component* component) { return component->getName() == name; });
    if (it == netlist.components.end()) {
        qDebug() << "getComponentPtr(" << name.c_str() << ")"
                 << "Component not found";
        return nullptr;
    }
    return *it;
}

// already defined in Netlist class
Model* Circuit::getModelPtr(const std::string& name) {
    // std::vector<Model*>& models;
    auto it = std::find_if(
        netlist.models.begin(), netlist.models.end(),
        [&name](Model* model) { return model->getName() == name; });
    if (it == netlist.models.end()) {
        qDebug() << "getModelPtr(" << name.c_str() << ")";
        printf("Model not found\n");
        return nullptr;
    }
    return *it;
}

void Circuit::runSimulations() {
    qDebug() << "runSimulations()";
    for (Analysis* analysis : netlist.analyses) {
        switch (analysis->analysis_type) {
            case ANALYSIS_OP: {
                break;
            }
            case ANALYSIS_DC: {
                qDebug() << "runSimulations() ANALYSIS_DC";
                DCSimulation* dc_simulation =
                    new DCSimulation(*analysis, netlist, nodes, branches);
                dc_simulation->runSimulation();
                dc_simulations.push_back(dc_simulation);
                break;
            }
            case ANALYSIS_AC: {
                qDebug() << "runSimulations() ANALYSIS_AC";
                ACSimulation* ac_simulation =
                    new ACSimulation(*analysis, netlist, nodes, branches);
                ac_simulation->runSimulation();
                ac_simulations.push_back(ac_simulation);
                break;
            }
            case ANALYSIS_TRAN: {
                qDebug() << "runSimulations() ANALYSIS_TRAN";
                TranSimulation* tran_simulation =
                    new TranSimulation(*analysis, netlist, nodes, branches);
                tran_simulation->runSimulation();
                tran_simulations.push_back(tran_simulation);
                break;
            }
            default: {
                break;
            }
        }
    }
}

std::vector<ColumnData> Circuit::createOutputYData(
    const std::vector<Variable>& var_list,
    const std::vector<arma::vec>& iter_results) {
    std::vector<ColumnData> ydata;
    // 不要用 getComponentPtr()，里面的索引包含地节点
    for (const auto& var : var_list) {
        for (const auto& node_branch : var.nodes) {
            ColumnData y;
            switch (var.type) {
                case TOKEN_VAR_VOLTAGE_REAL: {
                    std::cout << "Warning: voltage real should be used with AC "
                                 "analysis only! Ignored."
                              << std::endl;
                    break;
                }
                case TOKEN_VAR_VOLTAGE_IMAG: {
                    std::cout << "Warning: voltage imag should be used with AC "
                                 "analysis only! Ignored."
                              << std::endl;
                    break;
                }
                case TOKEN_VAR_VOLTAGE_MAG: {
                    y.name = "V(" + node_branch + ")";
                    int id_node = nodes.getNodeIndexExgnd(node_branch);
                    for (const auto& result : iter_results) {
                        y.values.push_back(result(id_node));
                    }
                    break;
                }
                case TOKEN_VAR_VOLTAGE_PHASE: {
                    std::cout
                        << "Warning: voltage phase should be used with AC "
                           "analysis only! Ignored."
                        << std::endl;
                    break;
                }
                case TOKEN_VAR_VOLTAGE_DB: {
                    y.name = "VDB(" + node_branch + ")";
                    int id_node = nodes.getNodeIndexExgnd(node_branch);
                    for (const auto& result : iter_results) {
                        y.values.push_back(20 * log10(result(id_node)));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_REAL: {
                    std::cout << "Warning: current real should be used with AC "
                                 "analysis only! Ignored."
                              << std::endl;
                    break;
                }
                case TOKEN_VAR_CURRENT_IMAG: {
                    std::cout << "Warning: current imag should be used with AC "
                                 "analysis only! Ignored."
                              << std::endl;
                    break;
                }
                case TOKEN_VAR_CURRENT_MAG: {
                    y.name = "I(" + node_branch + ")";
                    int node_num = nodes.getNodeNumExgnd();
                    int id_branch =
                        branches.getBranchIndex(node_branch) + node_num;
                    for (const auto& result : iter_results) {
                        y.values.push_back(result(id_branch));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_PHASE: {
                    std::cout
                        << "Warning: current phase should be used with AC "
                           "analysis only! Ignored."
                        << std::endl;
                    break;
                }
                case TOKEN_VAR_CURRENT_DB: {
                    y.name = "IDB(" + node_branch + ")";
                    int node_num = nodes.getNodeNumExgnd();
                    int id_branch =
                        branches.getBranchIndex(node_branch) + node_num;
                    for (const auto& result : iter_results) {
                        y.values.push_back(20 * log10(result(id_branch)));
                    }
                    break;
                }
                default: {
                    qDebug() << "createOutputYData() No such variable type!";
                    break;
                }
            }
            ydata.push_back(y);
        }
    }
    return ydata;
}

std::vector<ColumnData> Circuit::createOutputYData(
    const std::vector<Variable>& var_list,
    const std::vector<arma::cx_vec>& iter_cresults) {
    std::vector<ColumnData> ydata;

    for (const auto& var : var_list) {
        for (const auto& node_branch : var.nodes) {
            ColumnData y;
            switch (var.type) {
                case TOKEN_VAR_VOLTAGE_REAL: {
                    y.name = "VR(" + node_branch + ")";
                    int id_node = nodes.getNodeIndexExgnd(node_branch);
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(real(cresult(id_node)));
                    }
                    break;
                }
                case TOKEN_VAR_VOLTAGE_IMAG: {
                    y.name = "VI(" + node_branch + ")";
                    int id_node = nodes.getNodeIndexExgnd(node_branch);
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(imag(cresult(id_node)));
                    }
                    break;
                }
                case TOKEN_VAR_VOLTAGE_MAG: {
                    y.name = "V(" + node_branch + ")";
                    int id_node = nodes.getNodeIndexExgnd(node_branch);
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(abs(cresult(id_node)));
                    }
                    break;
                }
                case TOKEN_VAR_VOLTAGE_PHASE: {
                    y.name = "VP(" + node_branch + ")";
                    int id_node = nodes.getNodeIndexExgnd(node_branch);
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(arg(cresult(id_node)));
                    }
                    break;
                }
                case TOKEN_VAR_VOLTAGE_DB: {
                    y.name = "VDB(" + node_branch + ")";
                    int id_node = nodes.getNodeIndexExgnd(node_branch);
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(20 * log10(abs(cresult(id_node))));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_REAL: {
                    y.name = "IR(" + node_branch + ")";
                    int node_num = nodes.getNodeNumExgnd();
                    int id_branch =
                        branches.getBranchIndex(node_branch) + node_num;
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(real(cresult(id_branch)));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_IMAG: {
                    y.name = "II(" + node_branch + ")";
                    int node_num = nodes.getNodeNumExgnd();
                    int id_branch =
                        branches.getBranchIndex(node_branch) + node_num;
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(imag(cresult(id_branch)));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_MAG: {
                    y.name = "I(" + node_branch + ")";
                    int node_num = nodes.getNodeNumExgnd();
                    int id_branch =
                        branches.getBranchIndex(node_branch) + node_num;
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(abs(cresult(id_branch)));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_PHASE: {
                    y.name = "IP(" + node_branch + ")";
                    int node_num = nodes.getNodeNumExgnd();
                    int id_branch =
                        branches.getBranchIndex(node_branch) + node_num;
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(arg(cresult(id_branch)));
                    }
                    break;
                }
                case TOKEN_VAR_CURRENT_DB: {
                    y.name = "IDB(" + node_branch + ")";
                    int node_num = nodes.getNodeNumExgnd();
                    int id_branch =
                        branches.getBranchIndex(node_branch) + node_num;
                    for (const auto& cresult : iter_cresults) {
                        y.values.push_back(20 * log10(abs(cresult(id_branch))));
                    }
                    break;
                }
                default: {
                    qDebug() << "createOutputYData() No such variable type!";
                    break;
                }
            }
            ydata.push_back(y);
        }
    }
    return ydata;
}

void Circuit::outputResults() {
    // 首先遍历所有的 output，将相同类型的输出放在一起（例如可能有多个 .print dc
    // 语句）
    qDebug() << "outputResults()";
    for (Output* output : netlist.outputs) {
        switch (output->analysis_type) {
            case TOKEN_ANALYSIS_OP: {
                break;
            }
            case TOKEN_ANALYSIS_DC: {
                // print, plot 均加入 dc_print_requests
                dc_print_requests.insert(dc_print_requests.end(),
                                         output->var_list.begin(),
                                         output->var_list.end());
                if (output->output_type == ANALYSIS_PLOT) {
                    dc_plot_requests.insert(dc_plot_requests.end(),
                                            output->var_list.begin(),
                                            output->var_list.end());
                }
                break;
            }
            case TOKEN_ANALYSIS_AC: {
                ac_print_requests.insert(ac_print_requests.end(),
                                         output->var_list.begin(),
                                         output->var_list.end());
                if (output->output_type == ANALYSIS_PLOT) {
                    ac_plot_requests.insert(ac_plot_requests.end(),
                                            output->var_list.begin(),
                                            output->var_list.end());
                }
                break;
            }
            case TOKEN_ANALYSIS_TRAN: {
                tran_print_requests.insert(tran_print_requests.end(),
                                           output->var_list.begin(),
                                           output->var_list.end());
                if (output->output_type == ANALYSIS_PLOT) {
                    tran_plot_requests.insert(tran_plot_requests.end(),
                                              output->var_list.begin(),
                                              output->var_list.end());
                }
                break;
            }
            default: {
                qDebug() << "outputResults() No such analysis type!";
                break;
            }
        }
    }

    // 然后分别对于 op、dc、ac、tran 进行输出
    /*
    int op_sim_id = 0;
    for (OpSimulation* op_simulation : op_simulations) {
        if (op_print_requests.empty()) {
            qDebug() << "No op output requests!";
            break;
        }
    }
    */
    int dc_sim_id = 0;
    for (DCSimulation* dc_simulation : dc_simulations) {
        if (dc_print_requests.empty()) {
            qDebug() << "No dc output requests!";
            break;
        }

        std::string iter_name = dc_simulation->getIterName();
        std::vector<double> iter_values = dc_simulation->getIterValues();
        std::vector<arma::vec> iter_results = dc_simulation->getIterResults();

        // create xdata
        ColumnData xdata = ColumnData{iter_name, iter_values};

        // create print ydata
        std::vector<ColumnData> ydata_print;
        ydata_print = createOutputYData(dc_print_requests, iter_results);

        // print
        printOutputData(xdata, ydata_print, "dc", dc_sim_id);

        if (dc_plot_requests.empty()) {
            break;
        }
        // create plot ydata
        std::vector<ColumnData> ydata_plot;
        ydata_plot = createOutputYData(dc_plot_requests, iter_results);

        // plot
        plotOutputData(xdata, ydata_plot);

        ++dc_sim_id;
    }

    int ac_sim_id = 0;
    for (ACSimulation* ac_simulation : ac_simulations) {
        if (ac_print_requests.empty()) {
            qDebug() << "No ac output requests!";
            break;
        }

        std::string iter_name = ac_simulation->getIterName();
        std::vector<double> iter_values = ac_simulation->getIterValues();
        std::vector<arma::cx_vec> iter_cresults =
            ac_simulation->getIterResults();

        // create xdata
        ColumnData xdata = ColumnData{iter_name, iter_values};

        // create print ydata
        std::vector<ColumnData> ydata_print;
        ydata_print = createOutputYData(ac_print_requests, iter_cresults);

        // print
        printOutputData(xdata, ydata_print, "ac", 0);

        if (ac_plot_requests.empty()) {
            break;
        }
        // create plot ydata
        std::vector<ColumnData> ydata_plot;
        ydata_plot = createOutputYData(ac_plot_requests, iter_cresults);

        // plot
        plotOutputData(xdata, ydata_plot);

        ++ac_sim_id;
    }

    int tran_sim_id = 0;
    for (TranSimulation* tran_simulation : tran_simulations) {
        if (tran_print_requests.empty()) {
            qDebug() << "No tran output requests!";
            break;
        }

        std::string iter_name = tran_simulation->getIterName();
        std::vector<double> iter_values = tran_simulation->getIterValues();
        std::vector<arma::vec> iter_results = tran_simulation->getIterResults();

        // create xdata
        ColumnData xdata = ColumnData{iter_name, iter_values};

        // create print ydata
        std::vector<ColumnData> ydata_print;
        ydata_print = createOutputYData(tran_print_requests, iter_results);

        // print
        printOutputData(xdata, ydata_print, "tran", 0);

        if (tran_plot_requests.empty()) {
            break;
        }
        // create plot ydata
        std::vector<ColumnData> ydata_plot;
        ydata_plot = createOutputYData(tran_plot_requests, iter_results);

        // plot
        plotOutputData(xdata, ydata_plot);

        ++tran_sim_id;
    }
}

void Circuit::printOutputData(ColumnData& xdata,
                              std::vector<ColumnData>& ydata,
                              std::string sim_type,
                              int sim_id) const {
    if (xdata.values.size() != ydata[0].values.size()) {
        qDebug() << "printOutputData() xdata and ydata size not match!";
        return;
    }

    // print xdata, ydata and export to csv file
    std::filesystem::path p(netlist.file_path);
    p.replace_filename(p.stem().string() + "-" + sim_type +
                       std::to_string(sim_id) + ".csv");
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

void Circuit::plotOutputData(ColumnData& xdata,
                             std::vector<ColumnData>& ydata) const {
    if (xdata.values.size() != ydata[0].values.size()) {
        qDebug() << "plotOutputData() xdata and ydata size not match!";
        return;
    }
    // call qcustomplot
    callPlot(xdata, ydata);
}
