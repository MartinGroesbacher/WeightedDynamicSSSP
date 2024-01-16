#pragma once
#include <vector>
#include "DynamicSSSP.h"
using namespace std;
class Node;
class Edge;
class EStree : public DynamicSSSP
{
    public:
    EStree();
    Node* s;
    int currentworktree;
    std::vector<int> requeuelimit;
    int maxqueuesize;
    Edge* insertDynamicEdge(Node* u, Node* v, int weight, int trees);
    Edge* deleteEdge(Edge* e);
    void deletionUpdate(Node* u, int app);
    int query(Node* u, int app);
    void beginSSSP();
    void beginASSSP(int app); 
    void sortTrees(Node* s, int app);
    
};

