#ifndef SPICIAL_NODES_H
#define SPICIAL_NODES_H

#include <string>
#include <vector>

class Nodes {
   public:
    Nodes();
    ~Nodes();

    int addNode(const std::string& newNode);
    int getNodeIndex(const std::string& name) const;
    int getNodeIndexExgnd(const std::string& name) const;
    int getNodeNum() const;
    int getNodeNumExgnd() const;
    void printNodes() const;

   private:
    std::vector<std::string> nodes;
};

#endif  // SPICIAL_NODES_H
