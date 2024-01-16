#include <vector>
#include <algorithm>
#include "Node.h"
#include "Edge.h"
using namespace std;

Node::Node(int id) {
    this->ID = id;
    this->treeLevel = 0;
    this->visited = 0;
    this->affected = 0;
}

void Node::addEdge(Edge* e){
    this->inedges.push_back(e);
}

void Node::addSPGedge(Edge* e, int app){
    this->spginedges[app].push_back(e);
}

void Node::addChild(Node* n, int app){
    this->children[app].push_back(n);
}

// Worst case complexity O(n) but cases are rare
void Node::deleteChild(Node* n, int app){
    std::vector<Node*>::iterator it = std::find(this->children[app].begin(), this->children[app].end(), n);
    if (it != this->children[app].end())
        this->children[app].erase(it);
}

// Worst case complexity O(n) but cases are rare
void Node::deleteParent(Node* n, int app){
    std::vector<Node*>::iterator it = std::find(this->spgparents[app].begin(), this->spgparents[app].end(), n);
    if (it != this->spgparents[app].end())
        this->spgparents[app].erase(it);
}

// Complexity O(1)
void Node::deleteInedge(Edge* e){
    int oldindex = 0;
    Edge* e1;
    if (this->ID == e->nodelist[0]->ID) {
        oldindex = e->positions[0][0];
        e1 = this->inedges.back();
        std::swap(this->inedges[oldindex], this->inedges.back()); 
        e1->positions[0][0] = oldindex;
        this->inedges.pop_back();
    }
    else{
        oldindex = e->positions[0][1];
        e1 = this->inedges.back();
        std::swap(this->inedges[oldindex], this->inedges.back()); 
        e1->positions[0][1] = oldindex;
        this->inedges.pop_back();
    }
}

// Complexity O(1)
void Node::deleteSPGedge(Edge* e, int app){
    int oldindex = 0;
    Edge* e1;
    if (this->ID == e->nodelist[0]->ID) {
        oldindex = e->spgpositions[app][0];
        e1 = this->spginedges[app].back();
        std::swap(this->spginedges[app][oldindex], this->spginedges[app].back()); 
        e1->spgpositions[app][0] = oldindex;
        this->spginedges[app].pop_back();
    }
    else{
        oldindex = e->spgpositions[app][1];
        e1 = this->spginedges[app].back();
        std::swap(this->spginedges[app][oldindex], this->spginedges[app].back()); 
        e1->spgpositions[app][1] = oldindex;
        this->spginedges[app].pop_back();
    }
}
