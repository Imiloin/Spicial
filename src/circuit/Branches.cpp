#include "Branches.h"
#include <QDebug>
#include <iostream>

Branches::Branches() {}

Branches::~Branches() {}

int Branches::addBranch(const std::string& newBranch) {
    // 若新支路已经存在，返回支路的索引
    auto it = std::find(branches.begin(), branches.end(), newBranch);
    if (it != branches.end()) {
        return std::distance(branches.begin(), it);
    }
    branches.push_back(newBranch);
    // 若新支路被添加，返回支路的索引（最后一个）
    return branches.size() - 1;
}

int Branches::getBranchIndex(const std::string& name) const {
    auto it = std::find(branches.begin(), branches.end(), name);
    if (it == branches.end()) {
        qDebug() << "getBranchIndex(" << name.c_str() << ")";
        printf("Branch not found\n");
        return -1;
    }
    return std::distance(branches.begin(), it);
}

int Branches::getBranchNum() const {
    return branches.size();
}

void Branches::printBranches() const {
    // print {branch: index}
    std::cout << std::endl;
    std::cout << "-------------------------------" << std::endl
              << "Branches: {name: index}" << std::endl;
    for (std::size_t i = 0; i < branches.size(); i++) {
        std::cout << "{" << branches[i] << ": " << i << "}" << std::endl;
    }
    std::cout << "-------------------------------" << std::endl;
}
