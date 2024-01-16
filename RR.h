#pragma once
#include "DynamicSSSP.h"
using namespace std;
class Node;
class Edge;
class RR : public DynamicSSSP
{
    public:
    RR();
    Node* s;
    int prime_ID;
    Edge* insertDynamicEdge(Node* u, Node* v, int weight, int trees);
    Edge* insertPrimeEdge(Node* s_prime, Node* v, int weight, Node* from, Edge* mappedEdge);
    Edge* deleteEdge(Edge* e);
    void deletionUpdate(Node* u, int app);
    int query(Node* u, int app);
    void beginSSSP();
    void initializeSprime(Node* s_prime);
    void dijkstra(Node* s, int app);
    void removeSprime(Node* s_prime, int end);
    void beginASSSP(int app); 
};
