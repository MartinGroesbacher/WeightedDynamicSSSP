#pragma once
#include <vector>
using namespace std;
class Node;
class Edge;
class Graph;
class DynamicSSSP
{
	public:
	DynamicSSSP();
	Graph* g1;
	int dyndijkstras;
	int dynamic;
	int trees;
	int MAX_WEIGHT;
	std::vector<int> global_recomputation;
	// For ES based
	int updateLevelsOnInsertion(Node* u, Edge* e, Node* v, int app, int dynamic);
	int checkTreeEdge(Node* u, Edge* e, int app);
	void resetTreeValues(Edge* e, int app);
	void updateTreeValues(Node* u, Edge* e, Node* v, int app);
	void dijkstra(Node* s, int app);
	void updateParentChildIndex(Node* u, Edge* e, Node* v, int app, int index);
	int findSubstitute(Node* u, Edge* e, Node* v, int app, int index);
	// For SPG based
	void updateRelationsNew(Node* u, Edge* e, Node* v, int app, int u_or_v);
	void updateParentChildTime(Node* u, Edge* e, Node* v, int app, int u_or_v);
	void updateParentChildEquiDist(Node* u, Edge* e, Node* v, int app, int u_or_v);
	int updateOnDeletion(Node* u, Edge* e, Node* v, int app);
	void updateOnPrimeEdgeNew(Node* v, Edge* e, int app, int u_or_v);
	void updateOnPrimeEdgeEquiDist(Node* v, Edge* e, int app, int u_or_v);
	void updateRelationsEquiDist(Node* u, Edge* e, Node* v, int app, int u_or_v);

};

