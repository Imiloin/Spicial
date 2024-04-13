#include "Simulation.h"
#include <QDebug>

const arma::sp_mat* Simulation::MNA_T = nullptr;
const arma::vec* Simulation::RHS_T = nullptr;

Simulation::Simulation(Analysis& analysis_,
                       Netlist& netlist_,
                       Nodes& nodes_,
                       Branches& branches_,
                       const arma::sp_mat* MNA_T_,
                       const arma::vec* RHS_T_)
    : analysis(analysis_),
      netlist(netlist_),
      nodes(nodes_),
      branches(branches_) {
    if (MNA_T_ != nullptr && RHS_T_ != nullptr) {
        MNA_T = MNA_T_;
        RHS_T = RHS_T_;
    }
}

Simulation::~Simulation() {}

void Simulation::runSimulation() {
    // do nothing
    if (MNA_T == nullptr || RHS_T == nullptr) {
        qDebug() << "MNA_T or RHS_T is nullptr.";
        return;
    }
    return;
}

DCSimulation::DCSimulation(Analysis& analysis_,
                           Netlist& netlist_,
                           Nodes& nodes_,
                           Branches& branches_)
    : Simulation(analysis_, netlist_, nodes_, branches_) {
    MNA_DC_T = new arma::sp_mat(*MNA_T);
    RHS_DC_T = new arma::vec(*RHS_T);
}

void DCSimulation::runSimulation() {
    if (MNA_DC_T == nullptr || RHS_DC_T == nullptr) {
        qDebug() << "MNA_DC_T or RHS_DC_T is nullptr.";
        return;
    }

    int source_type = analysis.source_type;
    std::string source = analysis.source_name;

    qDebug() << "DCSimulation::runSimulation() source_type: " << source_type;
    switch (source_type) {
        case (COMPONENT_VOLTAGE_SOURCE): {
            qDebug() << "DCSimulation::runSimulation() source: "
                     << source.c_str();
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
                    qDebug() << "DCSimulation::runSimulation() solve failed.";
                } else {
                    // x.print("DCSimulation::runSimulation() x:");
                    // /////////////////////
                }
                iter_results.push_back(x);
            }
            break;
        }
        case (COMPONENT_CURRENT_SOURCE): {
            qDebug() << "DCSimulation() source: " << source.c_str();
            Component* current_source = netlist.getComponentPtr(source);
            if (current_source == nullptr) {
                qDebug() << "DCSimulation::runSimulation() current source not "
                            "found.";
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
                    qDebug() << "DCSimulation::runSimulation() solve failed.";
                } else {
                    // x.print("DCSimulation::runSimulation() x:");
                    // /////////////////////
                }
                iter_results.push_back(x);
            }
            break;
        }
        default:
            qDebug() << "DCSimulation::runSimulation() Unknown source type.";
            break;
    }
}

const std::vector<arma::vec>& DCSimulation::getIterResults() {
    if (iter_results.empty()) {
        qDebug() << "DCSimulation::getIterResults() iter_results is empty.";
    }
    return iter_results;
}

ACSimulation::ACSimulation(Analysis& analysis_,
                           Netlist& netlist_,
                           Nodes& nodes_,
                           Branches& branches_)
    : Simulation(analysis_, netlist_, nodes_, branches_) {
    std::complex<double> j(0, 1);
    // 生成 AC 状态 MNA，复制 base MNA，将虚部置零
    arma::sp_mat MNA_zerofill = arma::sp_mat(size((*MNA_T)));
    MNA_AC_T = new arma::sp_cx_mat((*MNA_T), MNA_zerofill);
    arma::vec RHS_zerofill = arma::vec(size((*RHS_T)), arma::fill::zeros);
    RHS_AC_T = new arma::cx_vec((*RHS_T), RHS_zerofill);

    // 生成 AC 状态 MNA 模板
    for (VoltageSource* voltage_source : netlist.voltage_sources) {
        double ac_magnitude = voltage_source->getACMagnitude();
        double ac_phase = voltage_source->getACPhase() / 180 * M_PI;
        int id_branch = voltage_source->getIdBranch();

        (*RHS_AC_T)(id_branch) = ac_magnitude * exp(j * ac_phase);
    }
    for (CurrentSource* current_source : netlist.current_sources) {
        int id_nplus = current_source->getIdNplus();
        int id_nminus = current_source->getIdNminus();
        double ac_magnitude = current_source->getACMagnitude();
        double ac_phase = current_source->getACPhase() / 180 * M_PI;

        (*RHS_AC_T)(id_nplus) = -ac_magnitude * exp(j * ac_phase);
        (*RHS_AC_T)(id_nminus) = ac_magnitude * exp(j * ac_phase);
    }
}

void ACSimulation::runSimulation() {
    if (MNA_AC_T == nullptr || RHS_AC_T == nullptr) {
        qDebug() << "MNA_AC_T or RHS_AC_T is nullptr.";
        return;
    }

    std::complex<double> j(0, 1);
    qDebug() << "ACSimulation::runSimulation()";

    // 运行 AC 分析
    for (double freq : analysis.iter_values) {
        arma::sp_cx_mat MNA_AC = *MNA_AC_T;
        arma::cx_vec RHS_AC = *RHS_AC_T;

        for (Capacitor* capacitor : netlist.capacitors) {
            int id_nplus = capacitor->getIdNplus();
            int id_nminus = capacitor->getIdNminus();
            double capacitance = capacitor->getCapacitance();

            MNA_AC(id_nplus, id_nplus) += 2 * M_PI * freq * capacitance * j;
            MNA_AC(id_nminus, id_nminus) += 2 * M_PI * freq * capacitance * j;
            MNA_AC(id_nplus, id_nminus) -= 2 * M_PI * freq * capacitance * j;
            MNA_AC(id_nminus, id_nplus) -= 2 * M_PI * freq * capacitance * j;
        }
        for (Inductor* inductor : netlist.inductors) {
            double inductance = inductor->getInductance();
            int id_branch = inductor->getIdBranch();

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
            qDebug() << "ACSimulation::runSimulation() solve failed.";
        } else {
            // x.print("ACSimulation::runSimulation() x:");
        }
        iter_cresults.push_back(x);
    }
}

const std::vector<arma::cx_vec>& ACSimulation::getIterResults() {
    if (iter_cresults.empty()) {
        qDebug() << "ACSimulation::getIterCResults() iter_cresults is empty.";
    }
    return iter_cresults;
}

TranSimulation::TranSimulation(Analysis& analysis_,
                               Netlist& netlist_,
                               Nodes& nodes_,
                               Branches& branches_)
    : Simulation(analysis_, netlist_, nodes_, branches_) {
    tstart = 0;
    tstep = analysis.step;
    tstop = 0;
    if (!analysis.iter_values.empty()) {
        tstart = analysis.iter_values.front();
        tstop = analysis.iter_values.back();
    }

    // 生成 Tran 状态 MNA，复制 base MNA
    MNA_TRAN_T = new arma::sp_mat(*MNA_T);
    RHS_TRAN_T = new arma::vec(*RHS_T);

    // 生成 Tran 状态 MNA 模板
    // 为 Capacitor 的 branch 进行调整
    for (Capacitor* capacitor : netlist.capacitors) {
        int id_nplus = capacitor->getIdNplus();
        int id_nminus = capacitor->getIdNminus();
        int id_branch = capacitor->getIdBranch();

        (*MNA_TRAN_T)(id_nplus, id_branch) = 1;
        (*MNA_TRAN_T)(id_nminus, id_branch) = -1;
        // (*MNA_TRAN_T)(id_branch, id_branch) = -1;
    }
}

arma::vec TranSimulation::tranBackEuler(double time,
                                        double h,
                                        const arma::vec x_prev) {
    // int node_num = nodes.getNodeNum();
    arma::sp_mat MNA_TRAN = *MNA_TRAN_T;
    arma::vec RHS_TRAN = *RHS_TRAN_T;
    arma::vec x_prev_gnd = x_prev;
    x_prev_gnd.insert_rows(0, arma::zeros(1));  // insert ground node

    for (Capacitor* capacitor : netlist.capacitors) {
        int id_nplus = capacitor->getIdNplus();
        int id_nminus = capacitor->getIdNminus();
        double capacitance = capacitor->getCapacitance();
        int id_branch = capacitor->getIdBranch();

        MNA_TRAN(id_branch, id_nplus) = capacitance / h;
        MNA_TRAN(id_branch, id_nminus) = -capacitance / h;
        RHS_TRAN(id_branch) =
            capacitance / h * (x_prev_gnd(id_nplus) - x_prev_gnd(id_nminus));
    }

    for (Inductor* inductor : netlist.inductors) {
        double inductance = inductor->getInductance();
        int id_branch = inductor->getIdBranch();

        MNA_TRAN(id_branch, id_branch) = -inductance / h;
        RHS_TRAN(id_branch) = -inductance / h * x_prev_gnd(id_branch);
    }

    for (VoltageSource* voltage_source : netlist.voltage_sources) {
        if (voltage_source->getFunction() == nullptr) {
            continue;  // 没有 function，直接使用 DC 电压，已在 MNA_TRAN_T
                       // 中存在
        }
        int id_branch = voltage_source->getIdBranch();

        RHS_TRAN(id_branch) = calcFunctionAtTime(voltage_source->getFunction(),
                                                 time, tstep, tstop);
    }

    for (CurrentSource* current_source : netlist.current_sources) {
        if (current_source->getFunction() == nullptr) {
            continue;  // 没有 function，直接使用 DC 电流，已在 MNA_TRAN_T
                       // 中存在
        }
        int id_nplus = current_source->getIdNplus();
        int id_nminus = current_source->getIdNminus();
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

void TranSimulation::runSimulation() {
    qDebug() << "TranSimulation::runSimulation()";
    double h = analysis.step;  // 为简化处理，使用恒定步长
    double time = 0;  // 当前时间点
    arma::vec x;      // 保存当前时间点的解

    if (MNA_TRAN_T == nullptr || RHS_TRAN_T == nullptr) {
        qDebug() << "generateTranMNA() failed.";
    }

    // 先根据初始条件解出第一组解（t = 0） //
    // qDebug() << "Creating MNA_TRAN_0 and RHS_TRAN_0";
    arma::sp_mat* MNA_TRAN_0 = new arma::sp_mat(*MNA_TRAN_T);
    arma::vec* RHS_TRAN_0 = new arma::vec(*RHS_TRAN_T);

    for (Capacitor* capacitor : netlist.capacitors) {
        int id_nplus = capacitor->getIdNplus();
        int id_nminus = capacitor->getIdNminus();
        double initial_voltage = capacitor->getInitialVoltage();
        int id_branch = capacitor->getIdBranch();

        (*MNA_TRAN_0)(id_nplus, id_branch) = 1;
        (*MNA_TRAN_0)(id_nminus, id_branch) = -1;
        (*MNA_TRAN_0)(id_branch, id_nplus) = 1;
        (*MNA_TRAN_0)(id_branch, id_nminus) = -1;
        (*MNA_TRAN_0)(id_branch, id_branch) = 0;
        (*RHS_TRAN_0)(id_branch) = initial_voltage;
    }
    for (Inductor* inductor : netlist.inductors) {
        int id_nplus = inductor->getIdNplus();
        int id_nminus = inductor->getIdNminus();
        double initial_current = inductor->getInitialCurrent();
        int id_branch = inductor->getIdBranch();

        (*MNA_TRAN_0)(id_branch, id_nplus) = 0;
        (*MNA_TRAN_0)(id_branch, id_nminus) = 0;
        (*MNA_TRAN_0)(id_branch, id_branch) = 1;
        (*RHS_TRAN_0)(id_branch) = initial_current;
    }
    for (VoltageSource* voltage_source : netlist.voltage_sources) {
        if (voltage_source->getFunction() == nullptr) {
            continue;  // 没有 function，直接使用 DC 电压，已在 MNA_TRAN_T
                       // 中存在
        }
        int id_branch = voltage_source->getIdBranch();

        (*RHS_TRAN_0)(id_branch) =
            calcFunctionAtTime(voltage_source->getFunction(), 0, h, tstop);
    }
    for (CurrentSource* current_source : netlist.current_sources) {
        if (current_source->getFunction() == nullptr) {
            continue;  // 没有 function，直接使用 DC 电流，已在 MNA_TRAN_T
                       // 中存在
        }
        int id_nplus = current_source->getIdNplus();
        int id_nminus = current_source->getIdNminus();
        double current_0 =
            calcFunctionAtTime(current_source->getFunction(), 0, h, tstop);

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
    if (tstart == 0) {
        iter_results.push_back(x);    // time = 0 的解
    }

    delete MNA_TRAN_0;
    delete RHS_TRAN_0;
    // 第一组解求解完毕 //

    // arma::sp_mat* MNA_TRAN = new arma::sp_mat(*MNA_TRAN_T);
    // arma::vec* RHS_TRAN = new arma::vec(*RHS_TRAN_T);
    // 求解 (0, tstart) 之间的解，不含两边 //
    for (time = h; time < tstart; time += h) {
        x = tranBackEuler(time, h, x);
    }
    time -= h;  // 回退到 tstart 前一个时间点
    // 求解 [time, tstart] 的解 //
    if (time < tstart) {
        double h_last = tstart - time;
        time = tstart;
        x = tranBackEuler(tstart, h_last, x);
        iter_results.push_back(x);    // tstart 的解
    }
    // 求解 (tstart, tstop] 的解 //
    // std::cout << (time < tstop) << std::endl;
    for (time += h; time <= tstop; time += tstep) {
        x = tranBackEuler(time, h, x);
        iter_results.push_back(x);    // time 的解
        // std::cout << "time: " << time << "\t";
        // x.print("TranSimulation() x:");
    }
}

const std::vector<arma::vec>& TranSimulation::getIterResults() {
    if (iter_results.empty()) {
        qDebug() << "TranSimulation::getIterResults() iter_results is empty.";
    }
    return iter_results;
}
