#include "Simulation.h"
#include <QDebug>

Simulation::Simulation(Circuit& circuit, Analysis& analysis)
    : circuit(circuit),
      netlist(circuit.netlist),
      analysis(analysis),
      nodes(circuit.nodes),
      nodes_exgnd(circuit.nodes_exgnd),
      branches(circuit.branches) {}

Simulation::~Simulation() {}

void Simulation::generateMNA() {
    // 生成 MNA, RHS 模板
    int node_num = circuit.getNodeNum();
    // qDebug() << "generateDCMNA() node_num: " << node_num;
    int branch_num = circuit.getBranchNum();
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
                int id_nplus = resistor->id_nplus;
                int id_nminus = resistor->id_nminus;
                double resistance = resistor->resistance;

                (*MNA)(id_nplus, id_nplus) += 1 / resistance;
                (*MNA)(id_nminus, id_nminus) += 1 / resistance;
                (*MNA)(id_nplus, id_nminus) -= 1 / resistance;
                (*MNA)(id_nminus, id_nplus) -= 1 / resistance;
                break;
            }
            case COMPONENT_CAPACITOR: {
                Capacitor* capacitor = dynamic_cast<Capacitor*>(component);
                int id_nplus = capacitor->id_nplus;
                int id_nminus = capacitor->id_nminus;
                int id_branch = capacitor->id_branch;
                double capacitance = capacitor->capacitance;

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
                int id_nplus = inductor->id_nplus;
                int id_nminus = inductor->id_nminus;
                int id_branch = inductor->id_branch;
                double inductance = inductor->inductance;

                (*MNA)(id_nplus, id_branch) = 1;
                (*MNA)(id_nminus, id_branch) = -1;
                (*MNA)(id_branch, id_nplus) = 1;
                (*MNA)(id_branch, id_nminus) = -1;
                break;
            }
            case COMPONENT_VCVS: {
                VCVS* vcvs = dynamic_cast<VCVS*>(component);
                int id_nplus = vcvs->id_nplus;
                int id_nminus = vcvs->id_nminus;
                int id_ncplus = vcvs->id_ncplus;
                int id_ncminus = vcvs->id_ncminus;
                int id_branch = vcvs->id_branch;
                double gain = vcvs->gain;

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
                int id_nplus = cccs->id_nplus;
                int id_nminus = cccs->id_nminus;
                int id_vsource = circuit.getBranchIndex(cccs->name) + node_num;
                double gain = cccs->gain;

                (*MNA)(id_nplus, id_vsource) += gain;
                (*MNA)(id_nminus, id_vsource) -= gain;
                break;
            }
            case COMPONENT_VCCS: {
                VCCS* vccs = dynamic_cast<VCCS*>(component);
                int id_nplus = vccs->id_nplus;
                int id_nminus = vccs->id_nminus;
                int id_ncplus = vccs->id_ncplus;
                int id_ncminus = vccs->id_ncminus;
                double gain = vccs->gain;

                (*MNA)(id_nplus, id_ncplus) += gain;
                (*MNA)(id_nplus, id_ncminus) -= gain;
                (*MNA)(id_nminus, id_ncplus) -= gain;
                (*MNA)(id_nminus, id_ncminus) += gain;
                break;
            }
            case COMPONENT_CCVS: {
                CCVS* ccvs = dynamic_cast<CCVS*>(component);
                int id_nplus = ccvs->id_nplus;
                int id_nminus = ccvs->id_nminus;
                int id_branch = ccvs->id_branch;
                double gain = ccvs->gain;
                int id_vsource =
                    circuit.getBranchIndex(ccvs->getVsource()) + node_num;

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
                int id_nplus = voltage_source->id_nplus;
                int id_nminus = voltage_source->id_nminus;
                int id_branch = voltage_source->id_branch;
                double dc_voltage = voltage_source->dc_voltage;              

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
                int id_nplus = current_source->id_nplus;
                int id_nminus = current_source->id_nminus;
                double dc_current = current_source->dc_current;

                (*RHS)(id_nplus) = -dc_current;
                (*RHS)(id_nminus) = dc_current;
                break;
            }
            case COMPONENT_DIODE: {
                Diode* diode = dynamic_cast<Diode*>(component);
                int id_nplus = diode->id_nplus;
                int id_nminus = diode->id_nminus;
                DiodeModel* model = diode->model;

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
                qDebug() << "generateMNA() Unknown component type:" << component->getType();
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
