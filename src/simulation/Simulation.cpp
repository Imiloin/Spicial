#include "Simulation.h"
#include <QDebug>

arma::sp_mat* Simulation::MNA_T = nullptr;
arma::vec* Simulation::RHS_T = nullptr;

Simulation::Simulation(Analysis& analysis_,
                       Netlist& netlist_,
                       Nodes& nodes_,
                       Branches& branches_)
    : analysis(analysis_),
      netlist(netlist_),
      nodes(nodes_),
      branches(branches_) {
    if (MNA_T == nullptr || RHS_T == nullptr) {
        this->generateMNA();
    }
}

Simulation::~Simulation() {}

void Simulation::generateMNA() {
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
                double inductance = inductor->getInductance();

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

DCSimulation::DCSimulation(Analysis& analysis_,
                           Netlist& netlist_,
                           Nodes& nodes_,
                           Branches& branches_)
    : Simulation(analysis_, netlist_, nodes_, branches_) {
    MNA_DC_T = MNA_T;
    RHS_DC_T = RHS_T;
}

void DCSimulation::runSimulation() {
    int source_type = analysis.source_type;
    std::string source = analysis.source_name;

    qDebug() << "DCSimulation() source_type: " << source_type;
    switch (source_type) {
        case (COMPONENT_VOLTAGE_SOURCE): {
            qDebug() << "DCSimulation() source: " << source.c_str();
            int id_vsrc =
                dynamic_cast<VoltageSource*>(netlist.getComponentPtr(source))
                    ->getIdBranch();

            for (double voltage : analysis.iter_values) {
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
                    x.print("DCSimulation() x:");  /////////////////////
                }
                iter_results.push_back(x);
            }
            break;
        }
        case (COMPONENT_CURRENT_SOURCE): {
            qDebug() << "DCSimulation() source: " << source.c_str();
            Component* current_source = netlist.getComponentPtr(source);
            if (current_source == nullptr) {
                qDebug() << "DCSimulation() current source not found.";
                return;
            }
            int id_nplus =
                dynamic_cast<CurrentSource*>(netlist.getComponentPtr(source))
                    ->getIdNplus();
            int id_nminus =
                dynamic_cast<CurrentSource*>(netlist.getComponentPtr(source))
                    ->getIdNminus();

            for (double current : analysis.iter_values) {
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
                    x.print("DCSimulation() x:");  /////////////////////
                }
                iter_results.push_back(x);
            }
            break;
        }
        default:
            break;
    }
}
