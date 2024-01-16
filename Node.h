#pragma once
#include <vector>
using namespace std;
class Edge;
class Node
{
    public:
    int ID;
    int treeLevel; // Used only for sortations
    int visited;
    int affected; 
    std::vector<int> timestamps;
    std::vector<Node*> parent;
    std::vector<vector<Node*>> spgparents;
    std::vector<vector<Node*>> children; 
    std::vector<Edge*> intreeedge;
    std::vector<int> intreeedgeindex; // e[v]
    std::vector<Edge*> inedges;
    std::vector<vector<Edge*>> ev; // ε^-(v): Sorted list of inedges according to tree level for each vertex in each approximation depth
    std::vector<vector<Edge*>> spginedges; // pointers to all inedges that belong to at least one shortest path for every approximation depth 
    std::vector<int> treeLevels; // also used in RR for distances from source

    Node(int id);
    void addEdge(Edge* e);
    void addSPGedge(Edge* e, int app);
    void deleteInedge(Edge* e);
    void deleteSPGedge(Edge* e, int app);
    void addChild(Node* child, int app);
    void deleteChild(Node* child, int app);
    void deleteParent(Node* parent, int app);
};
