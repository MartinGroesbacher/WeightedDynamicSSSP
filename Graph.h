#pragma once
#include <vector>
using namespace std;
class Node;
class Edge;
class Graph
{
    public:
    std::vector<Node*> nodelist;
    std::vector<Edge*> edgelist;

    Graph();
    int epsilon;
    void insertNode(int j);
    Edge* insertBaseEdge(Node* u, Node* v, int weight, int trees, int max);
    void deleteEdge(Edge* e);
    void roundWeights(Edge* e, int trees); 
    Edge* locateEdge(Node* u, Node* v);
};
