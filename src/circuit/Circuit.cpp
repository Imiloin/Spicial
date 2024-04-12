#include "Circuit.h"
#include <QDebug>

Circuit::Circuit(Netlist& netlist_) : netlist(netlist_) {
    this->preProcess();
}

Circuit::~Circuit() {
    /*
    for (auto& component : components)   // delete components
        delete component;
    for (auto& model : models)   // delete models
        delete model;
    */
    for (DCSimulation* sim : dc_simulations) {
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
}

void Circuit::printNodes() {
    nodes.printNodes();
}

void Circuit::printBranches() {
    branches.printBranches();
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
    for (Analysis* analysis : netlist.analyses) {
        switch (analysis->analysis_type) {
            case ANALYSIS_DC: {
                qDebug() << "runSimulations() ANALYSIS_DC";
                DCSimulation* dc_simulation =
                    new DCSimulation(*analysis, netlist, nodes, branches);
                dc_simulation->runSimulation();
                break;
            }
            case ANALYSIS_AC: {
                break;
            }
            case ANALYSIS_TRAN: {
                break;
            }
            default: {
                break;
            }
        }
    }
}
