#include <vector>
#include <algorithm>
#include <cstddef>
#include <cmath>
#include "Node.h"
#include "Edge.h"
#include "Graph.h"
using namespace std;

Graph::Graph(){
}

void Graph::insertNode(int j){
    Node* n = new Node(j);
    this->nodelist.push_back(n);
}

Edge* Graph::insertBaseEdge(Node* u, Node* v, int weight, int trees, int max){
    Edge* e = new Edge(u, v, weight, trees, this->nodelist.size(), max);
    u->addEdge(e);
    v->addEdge(e); 
    this->edgelist.push_back(e);
    return e;
}

void Graph::deleteEdge(Edge* e){
    e->usable = 0;
    this->edgelist.pop_back();
}

void Graph::roundWeights(Edge* e, int trees){
    double intermediate;
    int outofrange = 1;
    for (int i = 1; i <= trees; ++i){
        intermediate = e->weight[0] / (this->epsilon * pow(2, i));
        if (intermediate > 0)
            e->weight[i] = (ceil(intermediate));
        else
            e->weight[i] = (outofrange);
    }
}

// Locate and delete an edge in the edgelist of the graph where we have no index (for Wikigraphs) in O(m) time
Edge* Graph::locateEdge(Node* u, Node* v){
    Edge* e;
    int i = 0;
    while (i < this->edgelist.size()){
        e = this->edgelist[i];
        if (e->nodelist[0]->ID == u->ID){
            if (e->nodelist[1]->ID == v->ID){ 
                std::swap(this->edgelist[i], this->edgelist.back());
                return e;
            }
        }
        else if (e->nodelist[0]->ID == v->ID){
            if (e->nodelist[1]->ID == u->ID){ 
                std::swap(this->edgelist[i], this->edgelist.back());
                return e;
            }
        }
        i = i + 1;
    }
    return NULL;
}
