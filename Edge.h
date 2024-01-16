#pragma once
#include <vector>
using namespace std;
class Node;
class Graph;
class Edge
{
    public:
    std::vector<int> recomputations;
    std::vector<int> weight;
    std::vector<int> treevalue;
    std::vector<int> treevaluereverse;
    int usable;
    Graph* g1;
    Node* isPrimeEdgeFrom; 
    Edge* mappedPrimeEdge;
    std::vector<Node*> nodelist;
    std::vector<vector<int>> positions;
    std::vector<vector<int>> spgpositions;
    std::vector<vector<int>> timestamps; // to check if this edge still matters in the current SPG graph
    Edge(Node *u, Node* v, int weight, int trees, int n, int max);
};

