#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include "Node.h"
#include "Edge.h"
#include "Graph.h"
#include "RR.h"
using namespace std;

RR::RR(){
}

Edge* RR::insertDynamicEdge(Node* u, Node* v, int weight, int trees){

    Edge* e;
    e = this->g1->insertBaseEdge(u, v, weight, trees, this->MAX_WEIGHT);
    if (e == NULL)
        return e;
    else {
        e->positions[0][0] = u->inedges.size() - 1;
        e->positions[0][1] = v->inedges.size() - 1;
        if (v->treeLevels[0] + e->weight[0] < u->treeLevels[0]){
            updateParentChildTime(u, e, v, 0, 0);
            dijkstra(u, 0);
        }
        else if (v->treeLevels[0] + e->weight[0] == u->treeLevels[0]){ 
            updateParentChildEquiDist(u, e, v, 0, 0);
        }
        else if (u->treeLevels[0] + e->weight[0] < v->treeLevels[0]){
            updateParentChildTime(v, e, u, 0, 1); 
            dijkstra(v, 0);
        }
        else if (u->treeLevels[0] + e->weight[0] == v->treeLevels[0]){
            updateParentChildEquiDist(v, e, u, 0, 1);
        }
        return e;
    }
}

// Inserts a directed edge from s_prime to an affected vertex
Edge* RR::insertPrimeEdge(Node* s_prime, Node* v, int weight, Node* from, Edge* mappedEdge){

    Edge* e = new Edge(s_prime, v, weight, this->trees, this->g1->nodelist.size(), this->MAX_WEIGHT);
    // If this edge yields a shortest path in the new SPG graph, we need the original Node and Edge it was derived from
    e->isPrimeEdgeFrom = from;
    e->mappedPrimeEdge = mappedEdge; 
    e->weight = std::vector<int>(this->trees + 1, weight);
    s_prime->addEdge(e); 
    this->g1->edgelist.push_back(e);
    return e;
}

Edge* RR::deleteEdge(Edge* e){

    this->g1->deleteEdge(e);
    Node* u = e->nodelist[0];
    Node* v = e->nodelist[1];
    Node* w;
    Edge* e1;
    int updatespg = 0;
    int oldindex = 0;  
    u->deleteInedge(e);
    v->deleteInedge(e);   
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
        deletionUpdate(w, 0);
    }
    return e;
}

void RR::initializeSprime(Node* s_prime){

        s_prime->treeLevel = 0;
        s_prime->treeLevels = std::vector<int>(this->trees + 1, 0);
        s_prime->spgparents = std::vector<vector<Node*>>(this->trees + 1);
        s_prime->children = std::vector<vector<Node*>>(this->trees + 1);
        s_prime->affected = 0;
        s_prime->spginedges = std::vector<vector<Edge*>>(this->trees + 1);
        s_prime->timestamps = std::vector<int>(this->trees + 1, 0);
}

class Comp 
{ 
public: 
	bool operator () (const Node* l, const Node* r) { 
		return l->treeLevel > r->treeLevel; 
	} 
};

void RR::dijkstra(Node* s, int app){

    this->dyndijkstras = this->dyndijkstras + 1;
    int index;
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
		        if (app == 0){
				if (e->nodelist[0]->ID != currentNode->ID){
				    n = e->nodelist[0]; // currentNode is at [1]
				}
				else {
				    n = e->nodelist[1]; // currentNode is at [0]
				}
				if (currentNode->treeLevels[app] + e->weight[app] < n->treeLevels[app]){
				    n->timestamps[app] = n->timestamps[app] + 1;
				    if (!n->spgparents[app].empty()){
					for (Node* p: n->spgparents[app]){
					    p->deleteChild(n, app);
					}
				    }
				    // If currentNode is s_prime, we need to set the original Node and Edge as SPG parent and SPG inedge
				    if (currentNode->ID == this->prime_ID) {
				        if (e->mappedPrimeEdge->nodelist[0]->ID != n->ID)
				            updateOnPrimeEdgeNew(n, e, 0, 1);
				        else
				            updateOnPrimeEdgeNew(n, e, 0, 0);				         
				    }
				    else{
				        if (e->nodelist[0]->ID != n->ID)
				            updateRelationsNew(n, e, currentNode, 0, 1);
				        else
				            updateRelationsNew(n, e, currentNode, 0, 0);
				    }
				    n->treeLevels[app] = currentNode->treeLevels[app] + e->weight[app]; // If currentNode is s_prime, 0+e->weight equals e->isPrimeEdgeFrom->treeLevels[app] + e->mappedPrimeEdge->weight[app]
				    n->treeLevel = n->treeLevels[app]; 
				    pq.push(n); 
			        }
			        else if (currentNode->treeLevels[app] + e->weight[app] == n->treeLevels[app]){   
			            if (currentNode->ID == this->prime_ID){
			                if (e->mappedPrimeEdge->nodelist[0]->ID != n->ID)
			                    updateOnPrimeEdgeEquiDist(n, e, 0, 1);
			                else
			                    updateOnPrimeEdgeEquiDist(n, e, 0, 0);			                    
			            }
			            else{		            
				        if (e->nodelist[0]->ID != n->ID)
				            updateRelationsEquiDist(n, e, currentNode, 0, 1);
				        else
				            updateRelationsEquiDist(n, e, currentNode, 0, 0);
				    }
			        }
		        }
		        else{
		            if (e->nodelist[0]->ID != currentNode->ID){
				n = e->nodelist[0]; // currentNode is at [1]
		            }
			    else {
		                n = e->nodelist[1]; // currentNode is at [0]
			    }
		            if (e->weight[app] == 0)
		                this->g1->roundWeights(e, this->trees);
		            if (currentNode->treeLevels[app] + e->weight[app] <= pow(2, app)){
				if (currentNode->treeLevels[app] + e->weight[app] < n->treeLevels[app]){
				    n->timestamps[app] = n->timestamps[app] + 1;
				    if (!n->spgparents[app].empty()){
					for (Node* p: n->spgparents[app]){
					    p->deleteChild(n, app);
					}
				    }
				    if (currentNode->ID == this->prime_ID) {
				        if (e->mappedPrimeEdge->nodelist[0]->ID != n->ID)
				            updateOnPrimeEdgeNew(n, e, 0, 1);
				        else
				            updateOnPrimeEdgeNew(n, e, 0, 0);				         
				    }
				    else{
				        if (e->nodelist[0]->ID != n->ID)
				            updateRelationsNew(n, e, currentNode, 0, 1);
				        else
				            updateRelationsNew(n, e, currentNode, 0, 0);
				    }
				    n->treeLevels[app] = currentNode->treeLevels[app] + e->weight[app]; 
				    n->treeLevel = n->treeLevels[app]; 
				    pq.push(n); 
			        }
			        else if (currentNode->treeLevels[app] + e->weight[app] == n->treeLevels[app]){   
			            if (currentNode->ID == this->prime_ID){
			                if (e->mappedPrimeEdge->nodelist[0]->ID != n->ID)
			                    updateOnPrimeEdgeEquiDist(n, e, 0, 1);
			                else
			                    updateOnPrimeEdgeEquiDist(n, e, 0, 0);
			            }
			            else{		            
				        if (e->nodelist[0]->ID != n->ID)
				            updateRelationsEquiDist(n, e, currentNode, 0, 1);
				        else
				            updateRelationsEquiDist(n, e, currentNode, 0, 0);
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

void RR::deletionUpdate(Node* u, int app){
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

void RR::removeSprime(Node* s_prime, int end){
    this->g1->nodelist.pop_back(); // s_prime is always the last vertex added to G
    int nend = this->g1->edgelist.size();
    int range = nend - end; // we pop this number of times from the end of G's edge list to remove all edges of s_prime from G
    for (int i = 0; i < range; ++i){
        this->g1->edgelist.pop_back();
    }
}

// Preprocessing
void RR::beginSSSP(){
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
    s->treeLevel = 0;
    s->treeLevels[0] = 0;
    dijkstra(s, 0); 	   
}

int RR::query(Node* u, int app){

    return u->treeLevels[app];
}
