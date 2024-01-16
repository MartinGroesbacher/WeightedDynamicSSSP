#pragma once
#include "AES.h"
using namespace std;
class Node;
class Edge;
class AMES : public AES
{
    public:
    AMES();
    Edge* deleteEdge(Edge* e);
    void deletionUpdate(Node* u, int app);
};
