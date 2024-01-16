#pragma once
#include "EStree.h"
using namespace std;
class Node;
class Edge;
class MES : public EStree
{
    public:
    MES();
    Edge* deleteEdge(Edge* e);
    void deletionUpdate(Node* u, int app);
};

