#ifndef SPICIAL_BRANCHES_H
#define SPICIAL_BRANCHES_H

#include <string>
#include <vector>

class Branches {
   public:
    Branches();
    ~Branches();

    int addBranch(const std::string& newBranch);
    int getBranchIndex(const std::string& name) const;
    int getBranchNum() const;
    void printBranches() const;

   private:
    std::vector<std::string> branches;
};

#endif  // SPICIAL_BRANCHES_H
