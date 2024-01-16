#pragma once
#include "RR.h"
using namespace std;
class Node;
class Edge;
class ARR : public RR
{
    public:
    ARR();
    Edge* insertDynamicEdge(Node* u, Node* v, int weight, int trees);
    Edge* deleteEdge(Edge* e);
    void deletionUpdate(Node* u, int app);
    int query(Node* u, int app);
    void beginSSSP();
};
