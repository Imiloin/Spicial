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
    // 应当从 netlist 中获取默认参数
    // 这里暂时使用默认参数
    rel_tol = 1e-3;
    abs_tol = 5e-5;
    max_iter = 100;
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

arma::vec Simulation::solveOneOP(arma::sp_mat& MNA,
                                 arma::vec& RHS,
                                 arma::vec& x_prev) const {
    // 创建 x_previter
    arma::vec x_previter = x_prev;
    // arma::vec x_previter_gnd = x_prev_gnd;

    arma::vec x = x_prev;  // 保存当前迭代的解
    // arma::vec x = arma::zeros(x_prev.n_elem);  // 保存当前迭代的解

    // 对非线性器件进行迭代求解
    for (int iter = 0; iter < max_iter; iter++) {
        arma::sp_mat MNA_iter = MNA;
        arma::vec RHS_iter = RHS;

        x_previter = x;
        // 为 x_prev 补一个 0，与 index 对应（地节点的电压就是 0）
        arma::vec x_previter_gnd = x_previter;
        x_previter_gnd.insert_rows(0, arma::zeros(1));

        for (Diode* diode : netlist.diodes) {
            int id_nplus = diode->getIdNplus();
            int id_nminus = diode->getIdNminus();
            int id_branch = diode->getIdBranch();
            DiodeModel* model = diode->getModel();

            // 从上一轮迭代的解开始迭代
            double vk = x_previter_gnd(id_nplus) - x_previter_gnd(id_nminus);
            double ik = model->calcCurrentAtVoltage(vk);
            double gk = model->calcConductanceAtVoltage(vk);
            double jk = ik - gk * vk;

            MNA_iter(id_nplus, id_nplus) += gk;
            MNA_iter(id_nplus, id_nminus) -= gk;
            MNA_iter(id_nminus, id_nminus) += gk;
            MNA_iter(id_nminus, id_nplus) -= gk;
            RHS_iter(id_nplus) -= jk;
            RHS_iter(id_nminus) += jk;
            MNA_iter(id_branch, id_nplus) = gk;
            MNA_iter(id_branch, id_nminus) = -gk;
            // MNA_iter(id_branch, id_branch) = -1;
            RHS_iter(id_branch) = -jk;
        }

        // exclude ground node
        MNA_iter.shed_row(0);
        MNA_iter.shed_col(0);
        RHS_iter.shed_row(0);

        /*
        // check if the matrix is singular
        if (arma::det(arma::mat(MNA_iter)) == 0) {
            qDebug() << "The matrix is singular after shedding the first "
                        "row and column, cannot solve the system.";
            x = x_previter;
            continue;
        }
        */

        // MNA_iter.print("MNA_iter");
        // RHS_iter.print("RHS_iter");

        bool status = arma::spsolve(x, MNA_iter, RHS_iter);
        // printf("status: %d\n", status);
        if (!status) {
            qDebug() << "solveOneOP() solve failed, iter: " << iter;

            x = x_previter;
            // 无解时，添加随机噪声
            arma::vec noise = arma::randn(x.n_elem) * abs_tol;
            x += noise;
        } else {
            // 使用阻尼技术，避免振荡解
            // double damping_factor = 0.5;
            // x = damping_factor * x + (1 - damping_factor) * x_previter;

            // x.print("solveOneOP() x:");
            // 检查是否收敛
            arma::vec err = arma::abs(x - x_previter);
            bool status_abs = all(err <= abs_tol);
            bool status_rel = all(err <= rel_tol * arma::abs(x_previter));
            if (status_abs && status_rel) {
                // qDebug() << "solveOneOP() converged, iter: " << iter;
                return x;
            }

            /*
            else if (iter == max_iter - 1) {
                // MNA_iter.print("MNA_iter");
                // RHS_iter.print("RHS_iter");
                std::cout << "sim_value = " << sim_value << std::endl;
                x_prev.print("x_prev");
                x.print("x");
                x_previter.print("x_previter");
                arma::vec err = arma::abs(x - x_previter);
                arma::vec err_rel = rel_tol * arma::abs(x_previter);
                err.print("err");
                err_rel.print("err_rel");
                qDebug() << "status_abs: " << status_abs
                         << " status_rel: " << status_rel;
            }
            */
        }
    }

    std::cout << "solveOneOP() Warning: sim_value = " << sim_value
              << ", max_iter reached, cannot converge." << std::endl;
    // x.print("x");
    return x;
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

    qDebug() << "DCSimulation::runSimulation()";

    arma::vec x = *RHS_DC_T;  // (偷懒)直接用 RHS_DC_T 作为默认值
    x.shed_row(0);            // 去掉 ground node

    switch (source_type) {
        case (COMPONENT_VOLTAGE_SOURCE): {
            qDebug() << "DCSimulation::runSimulation() source: "
                     << source.c_str();
            Component* voltage_source = netlist.getComponentPtr(source);
            if (voltage_source == nullptr) {
                qDebug() << "DCSimulation::runSimulation() voltage source not "
                            "found.";
                return;
            }
            int id_vsrc =
                dynamic_cast<VoltageSource*>(voltage_source)->getIdBranch();

            for (double voltage : analysis.sim_values) {
                sim_value = voltage;

                arma::sp_mat MNA_DC = *MNA_DC_T;
                arma::vec RHS_DC = *RHS_DC_T;
                arma::vec x_prev = x;

                RHS_DC(id_vsrc) = voltage;

                x = solveOneOP(MNA_DC, RHS_DC, x_prev);
                sim_results.push_back(x);
            }
            break;
        }
        case (COMPONENT_CURRENT_SOURCE): {
            qDebug() << "DCSimulation::runSimulation() source: "
                     << source.c_str();
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

            for (double current : analysis.sim_values) {
                sim_value = current;

                arma::sp_mat MNA_DC = *MNA_DC_T;
                arma::vec RHS_DC = *RHS_DC_T;
                arma::vec x_prev = x;

                RHS_DC(id_nplus) = -current;
                RHS_DC(id_nminus) = current;

                x = solveOneOP(MNA_DC, RHS_DC, x_prev);
                sim_results.push_back(x);
            }
            break;
        }
        default:
            qDebug() << "DCSimulation::runSimulation() Unknown source type.";
            break;
    }
}

const std::vector<arma::vec>& DCSimulation::getIterResults() {
    if (sim_results.empty()) {
        qDebug() << "DCSimulation::getIterResults() sim_results is empty.";
    }
    return sim_results;
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
        double dc_voltage = voltage_source->getDCVoltage();
        double ac_magnitude = voltage_source->getACMagnitude();
        double ac_phase = voltage_source->getACPhase() / 180 * M_PI;
        int id_branch = voltage_source->getIdBranch();

        (*RHS_AC_T)(id_branch) = dc_voltage + ac_magnitude * exp(j * ac_phase);
    }
    for (CurrentSource* current_source : netlist.current_sources) {
        int id_nplus = current_source->getIdNplus();
        int id_nminus = current_source->getIdNminus();
        double ac_magnitude = current_source->getACMagnitude();
        double ac_phase = current_source->getACPhase() / 180 * M_PI;

        std::complex<double> ac_current = ac_magnitude * exp(j * ac_phase);

        (*RHS_AC_T)(id_nplus) -= ac_current;
        (*RHS_AC_T)(id_nminus) += ac_current;
    }
}

arma::cx_vec ACSimulation::solveOneFreq(arma::sp_cx_mat& MNA_AC,
                                        arma::cx_vec& RHS_AC,
                                        arma::vec& x_op,
                                        double freq) const {
    std::complex<double> j(0, 1);

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
    for (Diode* diode : netlist.diodes) {
        int id_nplus = diode->getIdNplus();
        int id_nminus = diode->getIdNminus();
        DiodeModel* model = diode->getModel();

        // 使用 diode 静态工作点
        double vk = x_op(id_nplus) - x_op(id_nminus);
        double ik = model->calcCurrentAtVoltage(vk);
        double gk = model->calcConductanceAtVoltage(vk);
        double jk = ik - gk * vk;

        MNA_AC(id_nplus, id_nplus) += gk;
        MNA_AC(id_nplus, id_nminus) -= gk;
        MNA_AC(id_nminus, id_nminus) += gk;
        MNA_AC(id_nminus, id_nplus) -= gk;
        RHS_AC(id_nplus) -= jk;
        RHS_AC(id_nminus) += jk;
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
        qDebug() << "ACSimulation::solveOneFreq() at frequency: " << freq
                 << "solve failed.";
        arma::vec x_op_zerofill = arma::vec(size(x_op), arma::fill::zeros);
        return arma::cx_vec(x_op, x_op_zerofill);
    } else {
        // x.print("ACSimulation::solveOneFreq() x:");
        return x;
    }
}

void ACSimulation::runSimulation() {
    if (MNA_AC_T == nullptr || RHS_AC_T == nullptr) {
        qDebug() << "MNA_AC_T or RHS_AC_T is nullptr.";
        return;
    }

    qDebug() << "ACSimulation::runSimulation()";

    // 先忽略交流信号，求解非线性器件的静态工作点 //
    arma::vec x_op = *RHS_T;  // (偷懒)直接用 RHS_DC_T 作为默认值
    x_op.shed_row(0);         // 去掉 ground node

    arma::sp_mat MNA_AC_OP = *MNA_T;
    arma::vec RHS_AC_OP = *RHS_T;

    x_op = solveOneOP(MNA_AC_OP, RHS_AC_OP, x_op);  // 静态工作点

    // 运行 AC 分析，此时就是线性系统 //
    arma::cx_vec x;
    for (double freq : analysis.sim_values) {
        sim_value = freq;

        arma::sp_cx_mat MNA_AC = *MNA_AC_T;
        arma::cx_vec RHS_AC = *RHS_AC_T;

        x = solveOneFreq(MNA_AC, RHS_AC, x_op, freq);

        sim_cresults.push_back(x);
    }
}

const std::vector<arma::cx_vec>& ACSimulation::getIterResults() {
    if (sim_cresults.empty()) {
        qDebug() << "ACSimulation::getIterCResults() sim_cresults is empty.";
    }
    return sim_cresults;
}

TranSimulation::TranSimulation(Analysis& analysis_,
                               Netlist& netlist_,
                               Nodes& nodes_,
                               Branches& branches_)
    : Simulation(analysis_, netlist_, nodes_, branches_) {
    tstart = 0;
    tstep = analysis.step;
    tstop = 0;
    if (!analysis.sim_values.empty()) {
        tstart = analysis.sim_values.front();
        tstop = analysis.sim_values.back();
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
                                        arma::vec x_prevtime) const {
    arma::sp_mat MNA_TRAN = *MNA_TRAN_T;
    arma::vec RHS_TRAN = *RHS_TRAN_T;
    arma::vec x_prevtime_gnd = x_prevtime;
    x_prevtime_gnd.insert_rows(0, arma::zeros(1));  // insert ground node

    for (Capacitor* capacitor : netlist.capacitors) {
        int id_nplus = capacitor->getIdNplus();
        int id_nminus = capacitor->getIdNminus();
        double capacitance = capacitor->getCapacitance();
        int id_branch = capacitor->getIdBranch();

        MNA_TRAN(id_branch, id_nplus) = capacitance / h;
        MNA_TRAN(id_branch, id_nminus) = -capacitance / h;
        RHS_TRAN(id_branch) =
            capacitance / h *
            (x_prevtime_gnd(id_nplus) - x_prevtime_gnd(id_nminus));
    }

    for (Inductor* inductor : netlist.inductors) {
        double inductance = inductor->getInductance();
        int id_branch = inductor->getIdBranch();

        MNA_TRAN(id_branch, id_branch) = -inductance / h;
        RHS_TRAN(id_branch) = -inductance / h * x_prevtime_gnd(id_branch);
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

    arma::vec x;
    x = solveOneOP(MNA_TRAN, RHS_TRAN, x_prevtime);

    return x;
}

void TranSimulation::runSimulation() {
    qDebug() << "TranSimulation::runSimulation()";
    int step_split = 8;  // 步长分割数，用于计算精度
    double h = analysis.step / step_split;  // 为简化处理，使用恒定步长
    double time = 0;                        // 当前时间点
    arma::vec x;                            // 保存当前时间点的解

    if (MNA_TRAN_T == nullptr || RHS_TRAN_T == nullptr) {
        qDebug() << "generateTranMNA() failed.";
    }

    // 先根据初始条件解出第一组解（t = 0） //
    // qDebug() << "Creating MNA_TRAN_0 and RHS_TRAN_0";
    arma::sp_mat* MNA_TRAN_0 = new arma::sp_mat(*MNA_TRAN_T);
    arma::vec* RHS_TRAN_0 = new arma::vec(*RHS_TRAN_T);

    for (Capacitor* capacitor : netlist.capacitors) {
        // 相当于无电流的电压源
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
        // 相当于无电压的电流源
        int id_nplus = inductor->getIdNplus();
        int id_nminus = inductor->getIdNminus();
        double initial_current = inductor->getInitialCurrent();
        int id_branch = inductor->getIdBranch();

        (*MNA_TRAN_0)(id_nplus, id_branch) = 0;
        (*MNA_TRAN_0)(id_nminus, id_branch) = 0;
        (*MNA_TRAN_0)(id_branch, id_nplus) = 0;
        (*MNA_TRAN_0)(id_branch, id_nminus) = 0;
        (*MNA_TRAN_0)(id_branch, id_branch) = -1;
        (*RHS_TRAN_0)(id_nplus) = -initial_current;
        (*RHS_TRAN_0)(id_nminus) = initial_current;
        (*RHS_TRAN_0)(id_branch) = -initial_current;
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
    for (Diode* diode : netlist.diodes) {
        // 已知了起始电压，就已知静态工作点，相当于电流源与电阻并联
        int id_nplus = diode->getIdNplus();
        int id_nminus = diode->getIdNminus();
        int id_branch = diode->getIdBranch();
        DiodeModel* model = diode->getModel();

        double v0 = diode->getInitialVoltage();
        double i0 = model->calcCurrentAtVoltage(v0);
        double g0 = model->calcConductanceAtVoltage(v0);
        double j0 = i0 - g0 * v0;

        (*MNA_TRAN_0)(id_nplus, id_nplus) += g0;
        (*MNA_TRAN_0)(id_nplus, id_nminus) -= g0;
        (*MNA_TRAN_0)(id_nminus, id_nminus) += g0;
        (*MNA_TRAN_0)(id_nminus, id_nplus) -= g0;
        (*RHS_TRAN_0)(id_nplus) -= j0;
        (*RHS_TRAN_0)(id_nminus) += j0;

        // (*MNA_TRAN_0)(id_nplus, id_branch) = 0;
        // (*MNA_TRAN_0)(id_nminus, id_branch) = 0;
        // (*MNA_TRAN_0)(id_branch, id_nplus) = 0;
        // (*MNA_TRAN_0)(id_branch, id_nminus) = 0;
        (*MNA_TRAN_0)(id_branch, id_branch) = -1;
        (*RHS_TRAN_0)(id_branch) = -i0;
    }

    // exclude ground node
    (*MNA_TRAN_0).shed_row(0);
    (*MNA_TRAN_0).shed_col(0);
    (*RHS_TRAN_0).shed_row(0);
    // check if the matrix is singular
    if (arma::det(arma::mat(*MNA_TRAN_0)) == 0) {
        qDebug() << "Solving MNA_TRAN_0 and RHS_TRAN_0";
        qDebug() << "The matrix is singular after shedding the first "
                    "row and column, cannot solve the system.";
        (*MNA_TRAN_0).print("MNA_TRAN_0");
        (*RHS_TRAN_0).print("RHS_TRAN_0");
        return;
    }

    sim_value = 0;
    bool status = arma::spsolve(x, (*MNA_TRAN_0), (*RHS_TRAN_0));
    // printf("status: %d\n", status);
    if (!status) {
        qDebug() << "TranSimulation::runSimulation() t = 0 solve failed.";
        (*MNA_TRAN_0).print("MNA_TRAN_0");
        (*RHS_TRAN_0).print("RHS_TRAN_0");
        return;
    }
    if (tstart == 0) {
        sim_results.push_back(x);  // time = 0 的解
    }

    delete MNA_TRAN_0;
    delete RHS_TRAN_0;
    // 第一组解求解完毕 //

    // arma::sp_mat* MNA_TRAN = new arma::sp_mat(*MNA_TRAN_T);
    // arma::vec* RHS_TRAN = new arma::vec(*RHS_TRAN_T);
    // 求解 (0, tstart) 之间的解，不含两边 //
    for (time = h; time < tstart; time += h) {
        sim_value = time;
        x = tranBackEuler(time, h, x);
    }
    time -= h;  // 回退到 tstart 前一个时间点
    // 求解 [time, tstart] 的解 //
    if (time < tstart) {
        double h_last = tstart - time;
        time = tstart;
        sim_value = time;
        x = tranBackEuler(tstart, h_last, x);
        sim_results.push_back(x);  // tstart 的解
    }
    // 求解 (tstart, tstop] 的解 //
    // std::cout << (time < tstop) << std::endl;
    for (time += h; time <= tstop; time += tstep) {
        sim_value = time;
        double inner_time = time;
        for (int i = 0; i < step_split; i++) {
            x = tranBackEuler(inner_time, h, x);
            inner_time += h;
        }
        sim_results.push_back(x);  // time 的解
        // std::cout << "time: " << time << "\t";
        // x.print("TranSimulation() x:");
    }
}

const std::vector<arma::vec>& TranSimulation::getIterResults() {
    if (sim_results.empty()) {
        qDebug() << "TranSimulation::getIterResults() sim_results is empty.";
    }
    return sim_results;
}
