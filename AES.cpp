#include <vector>
#include <cstddef>
#include <queue>
#include <cmath> 
#include <algorithm>
#include "Node.h"
#include "Edge.h"
#include "Graph.h"
#include "AES.h"
using namespace std;

AES::AES(){
}

struct InedgesCompare {
    AES* es1;
    InedgesCompare(AES* es){
        es1 = es;
    } 
    inline bool operator()(const Edge* l, const Edge* r) {
	if (l->nodelist[0]->ID == r->nodelist[0]->ID)
	    return l->treevaluereverse[es1->currentworktree] < r->treevaluereverse[es1->currentworktree];
	else if (l->nodelist[0]->ID == r->nodelist[1]->ID)
	    return l->treevaluereverse[es1->currentworktree] < r->treevalue[es1->currentworktree];
	else if (l->nodelist[1]->ID == r->nodelist[0]->ID)
	    return l->treevalue[es1->currentworktree] < r->treevaluereverse[es1->currentworktree];
	else if (l->nodelist[1]->ID == r->nodelist[1]->ID)
	    return l->treevalue[es1->currentworktree] < r->treevalue[es1->currentworktree];
	else
	    return l->treevalue[es1->currentworktree] < r->treevalue[es1->currentworktree];
    	}
};

Edge* AES::insertDynamicEdge(Node* u, Node* v, int weight, int trees){
    Edge* e;
    e = this->g1->insertBaseEdge(u, v, weight, trees, this->MAX_WEIGHT);
    if (e == NULL)
        return e;
    else{
        this->g1->roundWeights(e, trees);
        for (int i = 1; i <= trees; ++i){
            // We add e to the inedge lists of u and v in the ith tree only if its within the depth of the tree and update the index accordingly
            if (v->treeLevels[i] + e->weight[i] <= pow(2, i)){
                u->ev[i].push_back(e);
                e->positions[i][0] = u->ev[i].size() - 1;
                dynamic = 1;
                if (updateLevelsOnInsertion(v, e, u, i, this->dynamic) == 1)
                    dijkstra(u, i);
                dynamic = 0;
            }
            if (u->treeLevels[i] + e->weight[i] <= pow(2, i)){
                v->ev[i].push_back(e);
                e->positions[i][0] = v->ev[i].size() - 1;
                dynamic = 1;
                if (updateLevelsOnInsertion(u, e, v, i, this->dynamic) == 1)
                    dijkstra(v, i);
                dynamic = 0;
            }
        }
        return e;
    }
}

void AES::sortTrees(Node* s, int app){

    Edge* e;
    std::queue<Node*> treeQ;
    treeQ.push(s);
    Node* w;
    while(!treeQ.empty()){
        w = treeQ.front();
        treeQ.pop();
        if (!w->ev[app].empty()){
	        std::sort(w->ev[app].begin(), w->ev[app].end(), InedgesCompare(this));
                w->intreeedge[app] = w->ev[app][0];
                w->intreeedgeindex[app] = 0;
                // Now that the inedge list for w in the ith tree is sorted we can update the pointers on w's inedges for the ith tree
                for (int i = 0; i < w->ev[app].size(); ++i){
                    e = w->ev[app][i];
		    if (e->nodelist[0]->ID == w->ID){
		        e->positions[app][0] = i;
		    }
		    else if (e->nodelist[1]-> ID == w->ID){
		        e->positions[app][1] = i;
		    }
		}
        }
        for (Node* n: w->children[app]){
            treeQ.push(n);
        }
    }
}

// Preprocessing
void AES::beginSSSP(){
    Edge* e;

    for (Node* n: this->g1->nodelist){
        n->treeLevel = this->g1->nodelist.size() * this->MAX_WEIGHT;
        n->treeLevels = std::vector<int>(this->trees + 1, (this->g1->nodelist.size() * this->MAX_WEIGHT));
        n->parent = std::vector<Node*>(this->trees + 1);
        n->children = std::vector<vector<Node*>>(this->trees + 1);
        n->ev = std::vector<vector<Edge*>>(this->trees + 1);
        n->intreeedge = std::vector<Edge*>(this->trees + 1);
        n->intreeedgeindex = std::vector<int>(this->trees + 1);
    }

    Node* s = this->s;
    // Build log nW trees of depth 2^i using Dijkstra and build sortations of inedges for every tree
        for (int i = 1; i <= this->trees; i++){
            this->currentworktree = i;
            s->treeLevel = 0;
            s->treeLevels[i] = 0;
            this->dynamic = 0;
            dijkstra(s, i);         
            sortTrees(s, i);            
	}	   
}

// To recompute only the ith tree from scratch, no other trees are affected
void AES::beginASSSP(int app){

    for (Node* n: this->g1->nodelist){
        n->treeLevel = this->g1->nodelist.size() * this->MAX_WEIGHT; 
        n->treeLevels[app] = this->g1->nodelist.size() * this->MAX_WEIGHT;
        n->parent[app] = NULL;
        n->ev[app].clear();
        n->children[app].clear();
        n->intreeedge[app] = NULL;
        n->intreeedgeindex[app] = 0;  
    }
    
    currentworktree = app;
    Node* s = this->s;
    s->treeLevel = 0;
    s->treeLevels[app] = 0;
    dijkstra(s, app);  
    
    for (Node* n: this->g1->nodelist){
        if (!n->ev[app].empty()){
            std::sort(n->ev[app].begin(), n->ev[app].end(), InedgesCompare(this));
            n->intreeedge[app] = n->ev[app][0];
            n->intreeedgeindex[app] = 0;
        }
    }  
} 

Edge* AES::deleteEdge(Edge* e){
    this->g1->deleteEdge(e);
    Node* u = e->nodelist[0];
    Node* v = e->nodelist[1];
    Node* w;
    for (int i = 1; i <= this->trees; ++i){
        int updatetree = 0;
        if (checkTreeEdge(u, e, i) == 1){
            w = u;
            updatetree = 1;
        }
        else if (checkTreeEdge(v, e, i) == 1){
            w = v;
            updatetree = 1;
        }
        if (updatetree == 1){
            deletionUpdate(w, i);
        }
    }
    return e;
}

void AES::deletionUpdate(Node* u, int app){

    std::queue<Node*> deletionQ;
    std::vector<int> requeues(this->g1->nodelist.size(), 0);
    if (u->treeLevels[app] <= pow(2, app)){ 
        if (u->parent[app] != NULL){ 
            u->parent[app]->deleteChild(u, app);
            u->parent[app] = NULL;
        }
        deletionQ.push(u);
        requeues[u->ID] = requeues[u->ID] + 1;
    }
    Node* w;
    Node* v;
    Edge* e;
    int edgeindex = 0;
    int substitute = 0;
    int recomputation = 0;
    while(!deletionQ.empty()){
        w = deletionQ.front();
        deletionQ.pop();
        edgeindex = w->intreeedgeindex[app];
        if (!w->ev[app].empty()){
            while(edgeindex < w->ev[app].size()){
                substitute = 0;
                if (w->ev[app][edgeindex]->usable){
                    e = w->ev[app][edgeindex];
                    if (w == e->nodelist[0])
                        v = e->nodelist[1];
                    else
                        v = e->nodelist[0];
                    if (findSubstitute(v, e, w, app, edgeindex) == 1){
                        substitute = 1;
                        break;
                    } 
                }
                edgeindex = edgeindex + 1;
            }

            // No substitute found->w's tree level is increased, e[v] is set back to the beginning of w's inedge list
            if (substitute == 0){
                w->treeLevels[app] = w->treeLevels[app] + 1;
                w->intreeedgeindex[app] = 0;
                // Insert w again along with all its ith children in the ith ES tree into Q 
                if (!w->children[app].empty()){
                    for (int j = 0; j < w->children[app].size(); ++j){
                        w->children[app][j]->parent[app] = NULL; 
                        deletionQ.push(w->children[app][j]);
                        requeues[w->children[app][j]->ID] = requeues[w->children[app][j]->ID] + 1; 
                    }
                }
                if (w->treeLevels[app] <= pow(2, app)){
                    deletionQ.push(w);
                    requeues[w->ID] = requeues[w->ID] + 1;
                }
                else{
                    w->treeLevels[app] = this->g1->nodelist.size() * this->MAX_WEIGHT; // If a vertex's level increments beyond the app'th tree's depth, we must set it to "infinity"
                }   
                // Check for recomputation threshold exceedings
                if (deletionQ.size() > maxqueuesize){
                    recomputation = 1;
                    goto recomputefromscratch;
                }
                for (int j = 0; j < w->children[app].size(); ++j){
                    if (requeues[w->children[app][j]->ID] > requeuelimit[app]){
                        recomputation = 1;
                        goto recomputefromscratch;
                    }
                }
                if (requeues[w->ID] > requeuelimit[app]){
                    recomputation = 1;
                    goto recomputefromscratch;
                }
                w->children[app].clear();
            }
        }
        else{
            // If w was cut from the connected component containing the source, just set its level to infinity
            w->treeLevels[app] = this->g1->nodelist.size() * this->MAX_WEIGHT;
        }
    }
    recomputefromscratch:
        if (recomputation == 1){
            requeues.clear();
            global_recomputation[app] = global_recomputation[app] + 1;
            this->beginASSSP(app);
        }
}

int AES::query(Node* u, int app){

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
