#pragma once
#include "EStree.h"
using namespace std;
class Node;
class Edge;
class AES : public EStree
{
    public:
    AES();
    Edge* insertDynamicEdge(Node* u, Node* v, int weight, int trees);
    Edge* deleteEdge(Edge* e);
    void deletionUpdate(Node* u, int app);
    void beginSSSP();
    void beginASSSP(int app);
    void sortTrees(Node* s, int app);
    int query(Node* u, int app);
};
