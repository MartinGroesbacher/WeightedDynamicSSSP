#include <vector>
#include <cstddef>
#include <queue> 
#include <algorithm>
#include "Node.h"
#include "Edge.h"
#include "Graph.h"
#include "MES.h"
using namespace std;

MES::MES(){
}

Edge* MES::deleteEdge(Edge* e){
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

void MES::deletionUpdate(Node* u, int app){

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
    int l_min = this->g1->nodelist.size() * this->MAX_WEIGHT; 
    int l_minindex = 0;
    while(!deletionQ.empty()){
        w = deletionQ.front();
        deletionQ.pop();
        edgeindex = w->intreeedgeindex[app];
        l_min = this->g1->nodelist.size() * this->MAX_WEIGHT;
        l_minindex = 0;
        if (!w->inedges.empty()){
            while(edgeindex < w->inedges.size()){
                substitute = 0;
                if (w->inedges[edgeindex]->usable){
                    e = w->inedges[edgeindex];
                    if (w == e->nodelist[0])
                        v = e->nodelist[1];
                    else
                        v = e->nodelist[0];                        
                    if (findSubstitute(v, e, w, app, edgeindex) == 1){
                        substitute = 1;
                        break;
                    }
                    else {
                        if (v->treeLevels[app] + e->weight[app] < l_min){
                            l_min = v->treeLevels[app] + e->weight[app];
                            l_minindex = edgeindex;
                        }
                    }
                }
                edgeindex = edgeindex + 1;
            }

            // No substitute found->keep scanning until the original index e[w]
            if (substitute == 0){
                edgeindex = 0;
                while (edgeindex < w->intreeedgeindex[app]){
                    if (w->inedges[edgeindex]->usable){
                        e = w->inedges[edgeindex];
                        if (w == e->nodelist[0])
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
                e = w->inedges[l_minindex];
                if (w == e->nodelist[0])
                    v = e->nodelist[1];
                else
                    v = e->nodelist[0];                   
		updateParentChildIndex(v, e, w, app, l_minindex);
                w->treeLevels[app] = v->treeLevels[app] + e->weight[app];               
                // Insert only the children of w into Q 
                if (!w->children[app].empty()){
                    for (int j = 0; j < w->children[app].size(); ++j){
                         if (w->children[app][j]->treeLevels[app] < (this->g1->nodelist.size() * this->MAX_WEIGHT)){
                             w->children[app][j]->parent[app] = NULL; 
                             deletionQ.push(w->children[app][j]);
                             requeues[w->children[app][j]->ID] = requeues[w->children[app][j]->ID] + 1;
                        }
                    }
                }
                // Check for recomputation threshold exceedings
                if (deletionQ.size() > maxqueuesize){
                    recomputation = 1;
                    goto recomputefromscratch;
                }
                for (int j = 0; j < w->children[app].size(); ++j){
                    if (requeues[w->children[app][j]->ID] > requeuelimit[0]){
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
            this->beginSSSP();
        }
}
