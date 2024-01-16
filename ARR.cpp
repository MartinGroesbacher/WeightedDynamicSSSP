#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include "Node.h"
#include "Edge.h"
#include "Graph.h"
#include "ARR.h"
using namespace std;

ARR::ARR(){
}

Edge* ARR::insertDynamicEdge(Node* u, Node* v, int weight, int trees){

    Edge* e;
    e = this->g1->insertBaseEdge(u, v, weight, trees, this->MAX_WEIGHT);
    if (e == NULL)
        return e;
    else {
        e->positions[0][0] = u->inedges.size() - 1;
        e->positions[0][1] = v->inedges.size() - 1;
        g1->roundWeights(e, trees);
        for (int i = 1; i <= trees; i++){
            if (v->treeLevels[i] + e->weight[i] <= pow(2, i)){ 
                if (v->treeLevels[0] + e->weight[0] < u->treeLevels[0]){
                    updateParentChildTime(u, e, v, i, 0);
                    dijkstra(u, i);
                }
                else if (v->treeLevels[0] + e->weight[0] == u->treeLevels[0]){ 
                    updateParentChildEquiDist(u, e, v, i, 0);
                }
                else if (u->treeLevels[0] + e->weight[0] < v->treeLevels[0]){
                    updateParentChildTime(v, e, u, 0, 1); 
                    dijkstra(v, i);
                }
                else if (u->treeLevels[0] + e->weight[0] == v->treeLevels[0]){
                    updateParentChildEquiDist(v, e, u, 0, 1);
                }
            }
        }
        return e;
    }
}

Edge* ARR::deleteEdge(Edge* e){

    this->g1->deleteEdge(e);
    Node* u = e->nodelist[0];
    Node* v = e->nodelist[1];
    Node* w;
    Edge* e1;
    int updatespg = 0;
    int oldindex = 0;  
    u->deleteInedge(e);
    v->deleteInedge(e); 
    for (int i = 1; i <= trees; i++){
        updatespg = 0;  
        // Delete if and only if e's timestamp matches u's and/or v's
        if (!u->timestamps[0] == 0){ // If u's timestamp is still 0 it lies in a disconnected component and hence e is no spg inedge of u
	    if (u->timestamps[0] == e->timestamps[0][0]){
		if (updateOnDeletion(u, e, v, 0) == 1){
		    w = u;
		    updatespg = 1;
		}
		else
		    w = u;
	    }
        }
        if (!v->timestamps[0] == 0) {
	    if (v->timestamps[0] == e->timestamps[0][1]){
		if (updateOnDeletion(v, e, u, 0) == 1){
		    w = v;
		    updatespg = 1; 
		}
		else
		    w = v;
	    }
        }
        if (updatespg == 1){
            deletionUpdate(w, i);
        }
    }
    return e;
}

void ARR::deletionUpdate(Node* u, int app){
    std::queue<Node*> deletionQ;
    std::vector<Node*> affectedVertices;
    if (u->treeLevels[app] >= (this->g1->nodelist.size() * this->MAX_WEIGHT)){ 
    	return;
    }
    else {
        deletionQ.push(u);
    } 
    Node* w;
    Node* v;
    Edge* e;
    int substitute = 0;
    int recomputation = 0;
    // Phase 1: Identify vertices affected by deletion
    while(!deletionQ.empty()){
        w = deletionQ.front();
        deletionQ.pop();
        if (w->spginedges[app].empty()) {
            if (!w->spgparents[app].empty()){
                for (Node* p: w->spgparents[app]){
                    p->deleteChild(u, app);
                }
                w->spgparents[app].clear();
            }
            w->affected = 1;
            affectedVertices.push_back(w);
            // If a vertex is affected, delete its SPG outedges by going over all of its children, deleting their SPG inedges and pushing them into Q
            if (!w->children[app].empty()){
		    for (Node* n: w->children[app]){
		        if (!n->spginedges[app].empty()){ 
				for (Edge* e: n->spginedges[app]){
				    for (Node* m: e->nodelist){ 
				        if (m->ID == w->ID){
				            n->deleteSPGedge(e, app);
				            deletionQ.push(n);
				        }
				    }
				}
			}	
		    }
            }
        }
    }
    // Phase 2: Determine new distances to the affected vertices
    // 2.1: Create Super Source s' and edges from U to A
    this->prime_ID = this->g1->nodelist.size(); 
    this->g1->insertNode(prime_ID);
    Node * s_prime = this->g1->nodelist.back();
    initializeSprime(s_prime);
    int end = this->g1->edgelist.size();

    for (int i = 0; i < affectedVertices.size(); ++i){
        w = affectedVertices[i];
        // w sets its tree level to "infinity" as we cannot be sure that it will be reachable from s after the deletion
        w->treeLevels[app] = this->g1->nodelist.size() * this->MAX_WEIGHT;
        for (Edge* e: w->inedges){
            if (e->nodelist[0]->ID != w->ID)
                v = e->nodelist[0];
            else
                v = e->nodelist[1];
            if(v->affected == 0){
                // If v is unaffected, the new Edge goes from s_prime to the affected vertex w with the weight of the distance level of the unaffected vertex v + the weight of (v,w)
                insertPrimeEdge(s_prime, w, v->treeLevels[app] + e->weight[app], v, e);             
            }
        }
    }
    // 2.2: Dijkstra from s_prime to determine the new distance levels, then delete s_prime and all of its edges from G
    dijkstra(s_prime, app);
    removeSprime(s_prime, end);
}

// Preprocessing
void ARR::beginSSSP(){
    Edge* e;
    this->prime_ID = this->g1->nodelist.size(); 
    
    for (Node* n: this->g1->nodelist){
        n->treeLevel = this->g1->nodelist.size() * this->MAX_WEIGHT;
        n->treeLevels = std::vector<int>(this->trees + 1, (this->g1->nodelist.size() * this->MAX_WEIGHT));
        n->spgparents = std::vector<vector<Node*>>(this->trees + 1);
        n->children = std::vector<vector<Node*>>(this->trees + 1);
        n->affected = 0;
        n->spginedges = std::vector<vector<Edge*>>(this->trees + 1);
        n->timestamps = std::vector<int>(this->trees + 1, 0);
    }
 
    Node* s = this->s;
    for (int i = 1; i <= trees; i++){
            s->treeLevel = 0;
            s->treeLevels[i] = 0;
            dynamic = 0;
            dijkstra(s, i);         
    }	   
}

int ARR::query(Node* u, int app){

    int minapp = this->g1->nodelist.size() * this->MAX_WEIGHT;
    int minappindex = this->trees; // The last tree always contains all distances
    for (int i = 1; i <= this->trees; i++){
        if ((u->treeLevels[i] * this->g1->epsilon * pow(2, i)) < minapp){
	    minapp = (u->treeLevels[i] * this->g1->epsilon * pow(2, i));
	    minappindex = i;
	}
    }
    return u->treeLevels[minappindex];
}
