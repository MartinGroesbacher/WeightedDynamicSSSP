#include <vector>
#include <cstddef>
#include <queue>
#include <cmath> 
#include <algorithm>
#include "Node.h"
#include "Edge.h"
#include "Graph.h"
#include "ASES.h"
using namespace std;

ASES::ASES(){
}

// Preprocessing
void ASES::beginSSSP(){
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
    // Build log nW trees of depth 2^i using Dijkstra 
        for (int i = 1; i <= this->trees; i++){
            this->currentworktree = i;
            s->treeLevel = 0;
            s->treeLevels[i] = 0;
            this->dynamic = 0;
            dijkstra(s, i);                    
	}	   
}

// To recompute only the ith tree from scratch, no other trees are affected
void ASES::beginASSSP(int app){

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
    
} 

Edge* ASES::deleteEdge(Edge* e){
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

void ASES::deletionUpdate(Node* u, int app){

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
    int edgeindex = 0; // For ASES, we iterate over all inedges of w starting from index 0
    int substitute = 0;
    int recomputation = 0;
    int l_min = this->g1->nodelist.size() * MAX_WEIGHT; 
    int l_minindex = 0;
    while(!deletionQ.empty()){
        w = deletionQ.front();
        deletionQ.pop();
        edgeindex = 0;
        l_min = this->g1->nodelist.size() * MAX_WEIGHT;
        l_minindex = 0;
        if (!w->ev[app].empty()){
            while(edgeindex < w->ev[app].size()){
                substitute = 0;
                if (w->ev[app][edgeindex]->usable){
                    e = w->ev[app][edgeindex];
                    if (w->ID == e->nodelist[0]->ID)
                        v = e->nodelist[1];
                    else
                        v = e->nodelist[0];
                    if (v->treeLevels[app] + e->weight[app] < l_min){
                        l_min = v->treeLevels[app] + e->weight[app];
                        l_minindex = edgeindex;
                    }
                }
                edgeindex = edgeindex + 1;
            }

            // Now pick the edge at index l_minindex as the new tree edge for w and increase w's tree level accordingly
            e = w->ev[app][l_minindex];
            if (w->ID == e->nodelist[0]->ID)
                v = e->nodelist[1];
            else
                v = e->nodelist[0];
            if (v->treeLevels[app] + e->weight[app] == w->treeLevels[app])
                substitute = 1;
            updateParentChildIndex(v, e, w, app, l_minindex);
            w->treeLevels[app] = v->treeLevels[app] + e->weight[app];

            if (substitute == 0) {
		if (w->treeLevels[app] > pow(2, app)){
                    w->treeLevels[app] = this->g1->nodelist.size() * this->MAX_WEIGHT; // If a vertex's level increments beyond the app'th tree's depth, we must set it to "infinity"
                }
                // Insert only the children of w into Q 
                if (!w->children[app].empty()){
                    for (int j = 0; j < w->children[app].size(); ++j){
                        w->children[app][j]->parent[app] = NULL; 
                        deletionQ.push(w->children[app][j]);
                        requeues[w->children[app][j]->ID] = requeues[w->children[app][j]->ID] + 1;
                    }
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
