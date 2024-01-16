#include <vector>
#include <cstddef>
#include <queue>
#include <cmath>
#include "Node.h"
#include "Edge.h"
#include "Graph.h"
#include "DynamicSSSP.h"
using namespace std;

DynamicSSSP::DynamicSSSP(){
}

int DynamicSSSP::updateLevelsOnInsertion(Node* u, Edge* e, Node* v, int app, int dynamic){

	if (u->treeLevels[app] + e->weight[app] < v->treeLevels[app]){
            if (v->parent[app] != NULL)
                v->parent[app]->deleteChild(v, app);
            u->addChild(v, app);
            v->parent[app] = u;
            v->intreeedge[app] = e;
            if (dynamic == 1){
	        if (e->nodelist[0]->ID == v->ID){
		    v->intreeedgeindex[app] = e->positions[app][0];
                }
	        else if (e->nodelist[1]->ID == v->ID){
		    v->intreeedgeindex[app] = e->positions[app][1];
	        }
	    }
            v->treeLevels[app] = u->treeLevels[app] + e->weight[app];
            v->treeLevel = v->treeLevels[app]; 
            return 1;
        }
        return 0;
}

int DynamicSSSP::checkTreeEdge(Node* u, Edge* e, int app){

    if (u->intreeedge[app] == e)
        return 1;
    return 0;
}

void DynamicSSSP::resetTreeValues(Edge* e, int app){

    e->treevalue[app] = this->g1->nodelist.size() * this->MAX_WEIGHT;
    e->treevaluereverse[app] = this->g1->nodelist.size() * this->MAX_WEIGHT;
}

void DynamicSSSP::updateTreeValues(Node* u, Edge* e, Node* v, int app){

    if (e->nodelist[0]->ID != u->ID){
	v = e->nodelist[0]; // u is at [1]
	// If the treevalue/reverse was "infinity" before, it was not maintained by the app'th tree yet so we have to add e to the app'th inedges of v now
	if (e->treevaluereverse[app] == this->g1->nodelist.size() * this->MAX_WEIGHT){
	    v->ev[app].push_back(e);
	    e->positions[app][0] = v->ev[app].size() - 1;
	}
	e->treevaluereverse[app] = u->treeLevels[app] + e->weight[app]; // if v is the first node, e=(v,u) so the reverse value is (u,v), the inedge of v
    }
    else {
	v = e->nodelist[1]; // u is at [0]
	if (e->treevalue[app] == this->g1->nodelist.size() * this->MAX_WEIGHT){
	    v->ev[app].push_back(e);
	    e->positions[app][1] = v->ev[app].size() - 1;
	}
	e->treevalue[app] = u->treeLevels[app] + e->weight[app];
    }
}

class Comp 
{ 
public: 
	bool operator () (const Node* l, const Node* r) { 
		return l->treeLevel > r->treeLevel; 
	} 
};

void DynamicSSSP::dijkstra(Node* s, int app){

    this->dyndijkstras = this->dyndijkstras + 1;
    std::priority_queue<Node*, vector<Node*>, Comp> pq;
    pq.push(s);
    Node* n;
    Node* currentNode = s;
    // u = currentNode, v = n
    while(!pq.empty()){
        currentNode = pq.top();
        pq.pop();
        if (currentNode->visited < this->dyndijkstras) {
            if (!currentNode->inedges.empty()){
		for (Edge* e: currentNode->inedges){
		    if (!e->usable == 0){
		        if (app == 0){
				if (e->nodelist[0]->ID != currentNode->ID){
				    n = e->nodelist[0]; 
				    e->treevaluereverse[app] = currentNode->treeLevels[app] + e->weight[app];
				}
				else {
				    n = e->nodelist[1]; 
				    e->treevalue[app] = currentNode->treeLevels[app] + e->weight[app];
				}
				if (updateLevelsOnInsertion(currentNode, e, n, app, this->dynamic) == 1){
				    pq.push(n);
				}				      
		        }
		        else{
		            if (e->weight[app] == 0)
		                this->g1->roundWeights(e, this->trees);
		            if (currentNode->treeLevels[app] + e->weight[app] <= pow(2, app)){
		                // In the case of a recomputation of the app'th tree, we need to reset the treevalue and treevaluereverse
		                if (e->recomputations[app] < global_recomputation[app]){
		                    resetTreeValues(e, app);
		                }
		                e->recomputations[app] = global_recomputation[app];
		                if (e->nodelist[0]->ID != currentNode->ID){
				    n = e->nodelist[0]; 
				    updateTreeValues(currentNode, e, n, app);
				}
				else {
				    n = e->nodelist[1]; 
		                    updateTreeValues(currentNode, e, n, app);
		                }
				if (updateLevelsOnInsertion(currentNode, e, n, app, this->dynamic) == 1){
				    pq.push(n); 
			        }
			    }    
		        }
		    }
		}
	    }
            currentNode->visited = this->dyndijkstras;
        }
    }
}

void DynamicSSSP::updateParentChildIndex(Node* u, Edge* e, Node* v, int app, int index){

    u->addChild(v, app);
    v->parent[app] = u;
    v->intreeedge[app] = e;
    v->intreeedgeindex[app] = index;
}

int DynamicSSSP::findSubstitute(Node* u, Edge* e, Node* v, int app, int index){

    if (u->treeLevels[app] + e->weight[app] == v->treeLevels[app]){
        updateParentChildIndex(u, e, v, app, index);
        return 1;
    }
    return 0;
}

void DynamicSSSP::updateRelationsNew(Node* u, Edge* e, Node* v, int app, int u_or_v){

    v->addChild(u, app);
    u->spgparents[app].clear();
    u->spgparents[app].push_back(v);
    u->spginedges[app].clear();
    u->addSPGedge(e, app); 
    if (u_or_v == 0){
        e->spgpositions[app][0] = 0; 
        e->timestamps[app][0] = u->timestamps[app]; 
    }
    else{
        e->spgpositions[app][1] = 0; 
        e->timestamps[app][1] = u->timestamps[app]; 
    }       
    u->treeLevels[app] = v->treeLevels[app] + e->weight[app]; 
}

void DynamicSSSP::updateParentChildTime(Node* u, Edge* e, Node* v, int app, int u_or_v){

    u->timestamps[app] = u->timestamps[app] + 1; // Now we know the active "class" of spg inedges without having to delete the old ones
    if (!u->spgparents[app].empty()){
        for (Node* p: u->spgparents[app]){
            p->deleteChild(u, app);
        }
    }
    updateRelationsNew(u, e, v, app, u_or_v);    
}

void DynamicSSSP::updateParentChildEquiDist(Node* u, Edge* e, Node* v, int app, int u_or_v){

    int index = 0;
    u->spgparents[app].push_back(v);
    v->addChild(u, app);
    u->addSPGedge(e, app);
    index = u->spginedges[app].size() - 1;
    if (u_or_v == 0){
        e->spgpositions[app][0] = index; 
        e->timestamps[app][0] = u->timestamps[app]; 
    }
    else{
        e->spgpositions[app][1] = index; 
        e->timestamps[app][1] = u->timestamps[app]; 
    }
}

int DynamicSSSP::updateOnDeletion(Node* u, Edge* e, Node* v, int app){
    if (u->spginedges[app].size() > 1){
        u->deleteSPGedge(e, app);
        u->deleteParent(v, app);
        v->deleteChild(u, app);
        return 0;
    }
    else{
        u->spginedges[app].clear();
        u->deleteParent(v, app);
        v->deleteChild(u, app);
        return 1;
    }
}

void DynamicSSSP::updateOnPrimeEdgeNew(Node* v, Edge* e, int app, int u_or_v){

    e->isPrimeEdgeFrom->addChild(v, app);
    v->spgparents[app].clear();
    v->spgparents[app].push_back(e->isPrimeEdgeFrom);
    v->spginedges[app].clear();
    v->addSPGedge(e->mappedPrimeEdge, app);
    if (u_or_v == 1){ 
	e->mappedPrimeEdge->spgpositions[app][1] = 0; 
	e->mappedPrimeEdge->timestamps[app][1] = v->timestamps[app]; 
    }
    else {
	e->mappedPrimeEdge->spgpositions[app][0] = 0; 
	e->mappedPrimeEdge->timestamps[app][0] = v->timestamps[app]; 
    } 
}

void DynamicSSSP::updateOnPrimeEdgeEquiDist(Node* v, Edge* e, int app, int u_or_v){

    int index = 0;
    e->isPrimeEdgeFrom->addChild(v, app);
    v->spgparents[app].push_back(e->isPrimeEdgeFrom);
    v->addSPGedge(e->mappedPrimeEdge, app);
    index = v->spginedges[app].size() - 1;
    if (u_or_v == 1){ 
	e->mappedPrimeEdge->spgpositions[app][1] = index; 
	e->mappedPrimeEdge->timestamps[app][1] = v->timestamps[app]; 
    }
    else {
	e->mappedPrimeEdge->spgpositions[app][0] = index; 
	e->mappedPrimeEdge->timestamps[app][0] = v->timestamps[app]; 
    } 
}

void DynamicSSSP::updateRelationsEquiDist(Node* u, Edge* e, Node* v, int app, int u_or_v){

    int index = 0;
    v->addChild(u, app);
    u->spgparents[app].push_back(v);
    u->addSPGedge(e, app); 
    index = u->spginedges[app].size() - 1;
    if (u_or_v == 0){
        e->spgpositions[app][0] = 0; 
        e->timestamps[app][0] = u->timestamps[app]; 
    }
    else{
        e->spgpositions[app][1] = 0; 
        e->timestamps[app][1] = u->timestamps[app]; 
    }       
}
