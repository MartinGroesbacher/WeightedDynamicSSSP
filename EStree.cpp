#include <vector>
#include <cstddef>
#include <queue> 
#include <algorithm>
#include "Node.h"
#include "Edge.h"
#include "Graph.h"
#include "EStree.h"
using namespace std;

EStree::EStree(){
}

struct InedgesCompare {
    EStree* es1;
    InedgesCompare(EStree* es){
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

// Preprocessing
void EStree::beginSSSP(){
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
    s->treeLevel = 0;
    s->treeLevels[0] = 0;
    this->dynamic = 0;
    dijkstra(this->s, 0);
    
    // Now all vertices know their tree levels and their tree inedge so we can now create the initially sorted list of inedges ε^-(v) for each vertex
    // This means that we can also set the tree inedges to the first inedge and further update the pointers on the edges that point towards their position in the 
    // inedge list of its incident vertices (these are needed for dynamic insertion)
    this->currentworktree = 0; 
    for (Node* n: this->g1->nodelist){
	if (!n->inedges.empty()){ 
	    std::sort(n->inedges.begin(), n->inedges.end(), InedgesCompare(this));
	    n->intreeedge[0] = n->inedges[0];
	    n->intreeedgeindex[0] = 0;
	    for (int i = 0; i < n->inedges.size(); ++i){
	        e = n->inedges[i];
	        if (e->nodelist[0]->ID == n->ID){
		    e->positions[0][0] = i;
	        }
                else if (e->nodelist[1]-> ID == n->ID){
	            e->positions[0][1] = i;
	        }
	    }
        }
    }	   
}

Edge* EStree::insertDynamicEdge(Node* u, Node* v, int weight, int trees){
    Edge* e;
    e = this->g1->insertBaseEdge(u, v, weight, trees, this->MAX_WEIGHT);
    if (e == NULL)
        return e;
    else {
        e->positions[0][0] = u->inedges.size() - 1;
        e->positions[0][1] = v->inedges.size() - 1;
        this->dynamic = 1;
        if (updateLevelsOnInsertion(v, e, u, 0, this->dynamic) == 1)
            dijkstra(u, 0);
        if (updateLevelsOnInsertion(u, e, v, 0, this->dynamic) == 1)
	    dijkstra(v, 0);        
        this->dynamic = 0;
        return e;
    }
}

Edge* EStree::deleteEdge(Edge* e){
    this->g1->deleteEdge(e);
    Node* u = e->nodelist[0];
    Node* v = e->nodelist[1];
    Node* w;
    int updatetree = 0;
    if (checkTreeEdge(u, e, 0) == 1){
        w = u;
        updatetree = 1;
    }
    else if (checkTreeEdge(v, e, 0) == 1){
        w = v;
        updatetree = 1;
    }
    if (updatetree == 1){
        deletionUpdate(w, 0);
    }
    return e;
}

void EStree::deletionUpdate(Node* u, int app){

    std::queue<Node*> deletionQ;
    std::vector<int> requeues(this->g1->nodelist.size(), 0);
    if (u->treeLevels[app] < (this->g1->nodelist.size() * this->MAX_WEIGHT)){ 
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
        if (!w->inedges.empty()){
            while(edgeindex < w->inedges.size()){
                substitute = 0;
                if (w->inedges[edgeindex]->usable){
                    e = w->inedges[edgeindex];
                    if (w == e->nodelist[0])
                        v = e->nodelist[1];
                    else
                        v = e->nodelist[1];
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
                // Insert w again along with all its children in the ES tree into Q 
                if (!w->children[app].empty()){
                    for (int j = 0; j < w->children[app].size(); ++j){
                        if (w->children[app][j]->treeLevels[app] < (this->g1->nodelist.size() * this->MAX_WEIGHT)){
                            w->children[app][j]->parent[app] = NULL; 
                            deletionQ.push(w->children[app][j]);
                            requeues[w->children[app][j]->ID] = requeues[w->children[app][j]->ID] + 1;
                        }
                    }
                }
                if (w->treeLevels[app] < (this->g1->nodelist.size() * MAX_WEIGHT)){ 
            	    deletionQ.push(w);
                    requeues[w->ID] = requeues[w->ID] + 1;
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
            beginSSSP();
        }
}

int EStree::query(Node* u, int app){

    return u->treeLevels[app];
}
