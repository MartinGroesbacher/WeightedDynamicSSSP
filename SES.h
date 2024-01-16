#pragma once
#include "EStree.h"
using namespace std;
class Node;
class Edge;
class SES : public EStree
{
    public:
    SES();
    void beginSSSP();
    Edge* deleteEdge(Edge* e);
    void deletionUpdate(Node* u, int app);
};

