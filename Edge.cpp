#include <vector>
#include "Node.h"
#include "Edge.h"
using namespace std;

Edge::Edge(Node* u, Node* v, int weight, int trees, int n, int max){
    this->nodelist.push_back(u);
    this->nodelist.push_back(v);
    this->weight = std::vector<int>(trees + 1, 0);
    this->weight[0] = weight;
    this->treevalue = std::vector<int>(trees + 1, n * max); 
    this->treevaluereverse = std::vector<int>(trees + 1, n * max);
    this->usable = 1;
    this->recomputations = std::vector<int>(trees + 1, 0);
    this->positions = vector<vector<int>>(trees + 1, vector<int>(2, n)); // We initialize positions with n as a vertex can have at most n-1 neighbors
    this->spgpositions = vector<vector<int>>(trees + 1, vector<int>(2, n));
    this->timestamps = vector<vector<int>>(trees + 1, vector<int>(2, 0));
}
