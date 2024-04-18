#include "Nodes.h"
#include <QDebug>
#include <iostream>

Nodes::Nodes() {
    // add ground node, name is "0"
    std::string gnd = "0";
    nodes.push_back(gnd);
}

Nodes::~Nodes() {}

int Nodes::addNode(const std::string& newNode) {
    // 若新节点已经存在，返回节点的索引
    auto it = std::find(nodes.begin(), nodes.end(), newNode);
    if (it != nodes.end()) {
        return std::distance(nodes.begin(), it);
    }
    nodes.push_back(newNode);
    // 若新节点被添加，返回节点的索引（最后一个）
    return nodes.size() - 1;
}

int Nodes::getNodeIndex(const std::string& name) const {  // 获取节点的编号
    auto it = std::find(nodes.begin(), nodes.end(), name);
    if (it == nodes.end()) {
        qDebug() << "getNodeIndex(" << name.c_str() << ")";
        printf("Node not found\n");
        return 0;
    }
    return std::distance(nodes.begin(), it);
}

int Nodes::getNodeIndexExgnd(const std::string& name) const {
    if (name == "0") {
        qDebug() << "getNodeIndexExgnd() trying to get ground node";
        return 0;
    }
    if (nodes.size() < 2) {
        printf("No nodes excluding ground\n");
        return 0;
    }
    auto it = std::find(nodes.begin() + 1, nodes.end(), name);
    if (it == nodes.end()) {
        qDebug() << "getNodeIndexExgnd(" << name.c_str() << ")";
        printf("Node not found\n");
        return 0;
    }
    return std::distance(nodes.begin() + 1, it);
}

int Nodes::getNodeNum() const {
    return nodes.size();
}

int Nodes::getNodeNumExgnd() const {
    return nodes.size() - 1;
}

void Nodes::printNodes() const {
    // print {node: index}
    long unsigned int node_num = getNodeNum();
    std::cout << std::endl;
    std::cout << "-------------------------------" << std::endl
              << "Nodes: {name: index}" << std::endl;
    for (std::size_t i = 0; i < node_num; i++) {
        std::cout << "{" << nodes[i] << ": " << i << "}" << std::endl;
    }
    std::cout << "-------------------------------" << std::endl;
}
