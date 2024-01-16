#pragma once
#include "AES.h"
using namespace std;
class Node;
class Edge;
class ASES : public AES
{
    public:
    ASES();
    void beginSSSP();
    void beginASSSP(int app);
    Edge* deleteEdge(Edge* e);
    void deletionUpdate(Node* u, int app);
};
