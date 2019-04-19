#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <cstdlib>
#include <string>

using namespace std;

class Vertex {
private:
	/*Variables*/
	double x; // x position
	double y; // y position

public:
	/*Constructors*/
	Vertex() {
		x = 0.;
		y = 0.;
	}
	Vertex(double x1, double y1) {
		x = x1;
		y = y1;
	}
	/*Members*/
	double getX() {
		return x;
	}
	double getY() {
		return y;
	}
	void setX(double xx) {
		x = xx;
	}
	void setY(double yy) {
		y = yy;
	}
	void setXY(double xx, double yy) {
		x = xx;
		y = yy;
	}
	double getLength(Vertex v2) {
		double length;
		length = (x - v2.getX())*(x - v2.getX()) + (y - v2.getY())*(y - v2.getY());
		length = sqrt(length);
		return length;
	}
	bool equals(Vertex v3) {
		if ((x == v3.getX()) && (y == v3.getY())) {
			return true;
		}
		else {
			return false;
		}
	}
};



/*Edge type can be used to implement different boundary conditions*/
class Edge {
private:
	/*Variables*/
	Vertex v1;
	Vertex v2;
	int type; // type used to identify where the edge is in the domain
	// Assigning 0 to the edges which are in the interior
	int count = 0; //Used to delete obsolete edges during bisection of triangles
	//Edge of type 0 becomes obsolete when count becomes 2
	//Edge of type Non Zero becomes obsolte when count becomes 1
	vector<Vertex> oppVer; //Opposite vertex to track the triangles of this edge
public:
	/*Constructors*/
	Edge(Vertex vv1, Vertex vv2) {
		v1 = vv1;
		v2 = vv2;
	}
	Edge(Vertex vv1, Vertex vv2, int t) {
		v1 = vv1;
		v2 = vv2;
		type = t;
	}
	/*Members*/
	int getType() {
		return type;
	}
	void setType(int typ) {
		type = typ;
	}
	double getLength() {
		double length = v1.getLength(v2);
		return length;
	}
	vector<Vertex> getVertices() {
		vector<Vertex> ver;
		ver.push_back(v1);
		ver.push_back(v2);
		return ver;
	}
	Vertex getVertex1() {
		return v1;
	}
	Vertex getVertex2() {
		return v2;
	}
	Vertex getMidpoint() {
		Vertex midpoint;
		midpoint.setXY(0.5*(v1.getX() + v2.getX()), 0.5*(v1.getY() + v2.getY()));
		return midpoint;
	}
	void addOppVertices(Vertex ver) {
		if (oppVer.size() == 0) {
			oppVer.push_back(ver);
		}
		else {
			int l = 1;
			for (int d = 0;d < oppVer.size();d++) {
				if (oppVer[d].equals(ver)) {
					l = 0;
				}
			}
			if (l == 1) {
				oppVer.push_back(ver);
			}
		}
	}
	vector<Vertex> getOppVertices() {
		return oppVer;
	}
	void incrementCount() {
		count = count + 1;
	}
	int getCount() {
		return count;
	}
	void clearOppVertex(Vertex ver) {
		for (int i = 0;i < oppVer.size();i++) {
			if (ver.equals(oppVer[i])) {
				oppVer.erase(oppVer.begin() + i);
			}
		}
	}
	bool equals(Edge e1) {
		int i = 0;
		vector<Vertex> e1v = e1.getVertices();
		Vertex e1v1 = e1v[0];
		Vertex e1v2 = e1v[1];
		if ((v1.equals(e1v1)) && (v2.equals(e1v2))) {
			i = 1;
		}
		else {
			i = 0;
		}

		if (i == 0) {
			if ((v1.equals(e1v2)) && (v2.equals(e1v1))) {
				i = 1;
			}
			else {
				i = 0;
			}
		}

		if (i == 1) {
			return true;
		}
		else {
			return false;
		}
	}
};




class Triangle {
private:
	Vertex v1;
	Vertex v2;
	Vertex v3;
	int newVertices = 0; // Used during refinement
	vector<Vertex> newNodes; // Used during refinement
	bool refine = false; //Used during refinement
public:
	/*Constructor*/
	Triangle(Vertex vv1, Vertex vv2, Vertex vv3) {
		v1 = vv1;
		v2 = vv2;
		v3 = vv3;
	}
	/*Members*/
	vector<Vertex> getVertices() {
		vector<Vertex> ver;
		ver.push_back(v3);
		ver.push_back(v1);
		ver.push_back(v2);
		return ver;
	}
	Vertex getCentroid() {
		double x1, y1, x2, y2, x3, y3, xc, yc;
		x1 = v1.getX();
		y1 = v1.getY();
		x2 = v2.getX();
		y2 = v2.getY();
		x3 = v3.getX();
		y3 = v3.getY();
		xc = (x1 + x2 + x3) / 3.0;
		yc = (y1 + y2 + y3) / 3.0;
		Vertex ver(xc, yc);
		return ver;
	}
	vector<Edge> getEdges() {
		Edge e12(v1, v2);
		Edge e23(v2, v3);
		Edge e31(v3, v1);
		vector<Edge> ed;
		ed.push_back(e12);
		ed.push_back(e23);
		ed.push_back(e31);
		return ed;
	}
	double getArea() {
		double x1, y1, x2, y2, x3, y3, area;
		x1 = v1.getX();
		y1 = v1.getY();
		x2 = v2.getX();
		y2 = v2.getY();
		x3 = v3.getX();
		y3 = v3.getY();
		area = 0.5*(x2*y3 - x3 * y2 - x1 * y3 + x1 * y2 + x3 * y1 - x2 * y1);
		area = fabs(area);
		return area;
	}
	Edge getLongestEdge() {
		Edge e12(v1, v2);
		Edge e23(v2, v3);
		Edge e31(v3, v1);
		Edge le = e12;
		if (le.getLength() < e23.getLength()) {
			le = e23;
		}
		if (le.getLength() < e31.getLength()) {
			le = e31;
		}
		return le;
	}
	void addNewNode(Vertex v) {
		if (newNodes.size() == 0) {
			newNodes.push_back(v);
		}
		else {
			int l = 1;
			for (int j = 0;j < newNodes.size();j++) {
				if (newNodes[j].equals(v)) {
					l = 0;
				}
			}
			if (l == 1) {
				newNodes.push_back(v);
			}
		}
		newVertices = newNodes.size();
	}
	int getNewVertices() {
		return newVertices;
	}
	void setRefine(bool dec) {
		refine = dec;
	}
	bool getRefine() {
		return refine;
	}
	Edge getEdgeOfPoint(Vertex ver) { //Used to find the edge on which a point (midpoint) lies
		Vertex v12, v23, v31;
		v12.setXY(0.5*(v1.getX() + v2.getX()), 0.5*(v1.getY() + v2.getY()));
		v23.setXY(0.5*(v2.getX() + v3.getX()), 0.5*(v2.getY() + v3.getY()));
		v31.setXY(0.5*(v3.getX() + v1.getX()), 0.5*(v3.getY() + v1.getY()));
		if (v12.equals(ver)) {
			Edge e12(v1, v2);
			return e12;
		}
		if (v23.equals(ver)) {
			Edge e23(v2, v3);
			return e23;
		}
		if (v31.equals(ver)) {
			Edge e31(v3, v1);
			return e31;
		}
	}
	Vertex getOppVertexofPoint(Vertex ver) { //Obtaining vertex of triangle which is opposite to an edge using the edge's midpoint
		Vertex v12, v23, v31, v;
		v12.setXY(0.5*(v1.getX() + v2.getX()), 0.5*(v1.getY() + v2.getY()));
		v23.setXY(0.5*(v2.getX() + v3.getX()), 0.5*(v2.getY() + v3.getY()));
		v31.setXY(0.5*(v3.getX() + v1.getX()), 0.5*(v3.getY() + v1.getY()));
		if (v12.equals(ver)) {
			v = v3;
		}
		if (v23.equals(ver)) {
			v = v1;
		}
		if (v31.equals(ver)) {
			v = v2;
		}
		return v;
	}
	vector<Vertex> getOrderedNewNodes() { //Ordered in a way that the first node is the longest edge midpoint
		vector<Vertex> ver;
		if (newVertices > 0) {
			Vertex longPoint = this->getLongestEdge().getMidpoint();
			ver.push_back(longPoint);
			for (int n = 0;n < newNodes.size();n++) {
				if (!(longPoint.equals(newNodes[n]))) {
					ver.push_back(newNodes[n]);
				}
			}
		}
		return ver;
	}
	vector<Vertex> getMidpointsOfEdges() {  //Used to obtain midpoints of edges of a triangle
		vector<Vertex> ver;
		Vertex v12, v23, v31;
		v12.setXY(0.5*(v1.getX() + v2.getX()), 0.5*(v1.getY() + v2.getY()));
		v23.setXY(0.5*(v2.getX() + v3.getX()), 0.5*(v2.getY() + v3.getY()));
		v31.setXY(0.5*(v3.getX() + v1.getX()), 0.5*(v3.getY() + v1.getY()));
		ver.push_back(v12);
		ver.push_back(v23);
		ver.push_back(v31);
		return ver;
	}
	Vertex getThirdVertex(Vertex vv1, Vertex vv2) {
		Vertex ver;
		ver.setXY(0.5*(vv1.getX() + vv2.getX()), 0.5*(vv1.getY() + vv2.getY()));
		Vertex ver1 = this->getOppVertexofPoint(ver);
		return ver1;
	}
	bool equals(Triangle t) {
		Vertex ver = t.getCentroid();
		Vertex ver1 = this->getCentroid();
		return ver1.equals(ver);
	}
};

int main() {
	vector<Vertex> vertices;
	vector<Edge> edg;
	vector<Triangle> trgl;

	ifstream inFile;
	inFile.open("Mesh.txt");
	if (!inFile.is_open()) {
		cout << "File didn't open\n";
		exit(EXIT_FAILURE);
	}

	string c;
	while (inFile >> c) {
		if (c.compare("$Nodes") == 0) {
			int nn, i = 0, j; // Number of nodes-nn, j is dummy
			inFile >> nn;
			for (i = 0;i < nn;i++) {
				inFile >> j;
				double x, y, z;
				inFile >> x >> y >> z;
				Vertex v(x, y);
				vertices.push_back(v);
			}
		}
		if (c.compare("$Elements") == 0) {
			int nel, i, j, k, l, q, m, n, p; // Number of elements - nel
			inFile >> nel;
			for (i = 0;i < nel;i++) {
				inFile >> j >> k; //k  - 1 is edge, 2 is triangle
				if (k == 1) {
					inFile >> q;
					for (int w = 0;w < q;w++) {
						int q1; //Dummy variable
						if (w == 0) {
							inFile >> l; //Type number of the edge
						}
						else {
							inFile >> q1;
						}
					}
					inFile >> m >> n;
					Edge e(vertices[m - 1], vertices[n - 1], l);
					edg.push_back(e);
				}
				else {
					inFile >> q;
					for (int w = 0;w < q;w++) {
						int q1;
						inFile >> q1;
					}
					inFile >> m >> n >> p;
					Triangle tre(vertices[m - 1], vertices[n - 1], vertices[p - 1]);
					trgl.push_back(tre);
				}

			}
		}
	}
	inFile.close();
	cout << vertices.size() << endl;
	cout << "No of Non-Zero Type Edges in the Initial Mesh " << edg.size() << endl;
	cout << "No of Triangles in the Initial Mesh " << trgl.size() << endl;

	//Test case setting two triangles to refine
	trgl[0].setRefine(true);
	trgl[1].setRefine(true);
	trgl[2].setRefine(true);
	//trgl[3].setRefine(true);

	for (int i = 0;i < trgl.size();i++) {
		Triangle t = trgl[i];
		vector<Edge> ed = t.getEdges();
		vector<Vertex> verr = t.getVertices();
		for (int j = 0;j < 3;j++) {
			Edge edd = ed[j];
			Vertex veer = verr[j];
			int l = 1;
			for (int k = 0;k < edg.size();k++) {
				if (edd.equals(edg[k])) {
					l = 0;
					edg[k].addOppVertices(veer);
					break;
				}
			}
			if (l == 1) {
				vector<Vertex> ver = edd.getVertices();
				Edge eg(ver[0], ver[1], 0); // Assigning type 0
				eg.addOppVertices(veer);
				edg.push_back(eg);
			}
		}
	}
	// Adding of opposite vertices to edges is done
	// Next, running through refinement of triangles (Use opposite vertex)
	// Followed by deleting of old edges and triangles, and
	//adding new edges and triangles to respective vectors
	for (int i = 0;i < trgl.size();i++) {
		if (trgl[i].getRefine()) {
			//See paper to write this portion of the code
			vector<Vertex> ver = trgl[i].getMidpointsOfEdges();
			for (int j = 0;j < 3;j++) {
				trgl[i].addNewNode(ver[j]); //Adding the midpoints to triangles
				//Loop to collect all triangles getting affected by this node
				bool decision = true;
				Triangle t = trgl[i];//Current triangle
				while (decision) {
					//Finding the edge from edg which contains this point
					int k; //To track edge number
					for (k = 0;k < edg.size();k++) {
						if (ver[j].equals(edg[k].getMidpoint())) {
							break;
						}
					}
					//Found the edge
					//Getting opposite vertices from this edge to construct neighbouring triangle
					vector<Vertex> oppVer = edg[k].getOppVertices();
					if (oppVer.size() > 2) {
						cout << "Opposite Vertices count is more than 2" << endl;
					}
					vector<Triangle> neigTri; //To store neighbouring triangle
					if (oppVer.size() == 2) {
						Triangle Tri1(edg[k].getVertex1(), edg[k].getVertex2(), oppVer[0]);
						Triangle Tri2(edg[k].getVertex1(), edg[k].getVertex2(), oppVer[1]);
						if (Tri1.equals(t)) {
							neigTri.push_back(Tri2);
						}
						if (Tri2.equals(t)) {
							neigTri.push_back(Tri1);
						}
					}
					if (neigTri.size() == 0) {
						decision = false;
					}
					//To find this triangle in trgl
					int l; //To track triangle number
					if (neigTri.size() == 1) {
						for (l = 0;l < trgl.size();l++) {
							if (neigTri[0].equals(trgl[l])) {
								break;
							}
						}
						trgl[l].addNewNode(ver[j]);
						decision = !(ver[j].equals(trgl[l].getLongestEdge().getMidpoint()));
						if (decision) {
							//Perform bisection of the longest edge of this neighbouring triangle
							trgl[l].addNewNode(trgl[l].getLongestEdge().getMidpoint());
							//Update vertex ver[j] to the new point which has been created
							ver[j] = trgl[l].getLongestEdge().getMidpoint();
							//Update the current triangle to trgl[l]
							t = trgl[l];
						}
					}
				}
			}
		}
	}
	//Updating all the triangles with bisected edges
	for (int m = 0;m < trgl.size();m++) {
		if (trgl[m].getNewVertices() > 0) {
			//If only 1 new node has been added 
			if (trgl[m].getNewVertices() == 1) {
				Vertex longVer = trgl[m].getLongestEdge().getMidpoint();
				//Creating vertex opposite to the midpoint of the longest edge
				Vertex OpplongVer = trgl[m].getOppVertexofPoint(longVer);
				//Creating and add the new edge of type 0,which bisects the triangle to edg
				Edge newEdge(longVer, OpplongVer, 0);
				Edge longEdge = trgl[m].getLongestEdge();
				newEdge.addOppVertices(longEdge.getVertex1());
				newEdge.addOppVertices(longEdge.getVertex2());
				edg.push_back(newEdge);
				//Creating new triangles with refine as false,false is preassigned and deleting the old one
				Triangle t1(longVer, OpplongVer, longEdge.getVertex1());
				Triangle t2(longVer, OpplongVer, longEdge.getVertex2());
				trgl.push_back(t1);
				trgl.push_back(t2);
				trgl.erase(trgl.begin() + m);
				//Creating new edges (if they aren't already in edg) with their type and adding Opposite Vertex
				int n[3][2] = {};//To keep track of the vector number and type of edge
				Edge e1(longVer, longEdge.getVertex1());
				Edge e2(longVer, longEdge.getVertex2());
				//The 2nd column of n for pre-existing edges gives its type
				//for  new edges (bisected edges) stores checker, which indicates
				//whether the edge already exists or not

				//Creating edges to detect unchanged edges
				//as their oppVertices have to be updated
				Edge oldEdge1(OpplongVer, longEdge.getVertex1());
				Edge oldEdge2(OpplongVer, longEdge.getVertex2());
				int w[2] = {};
				for (int n1 = 0;n1 < edg.size();n1++) {
					if (longEdge.equals(edg[n1])) {
						n[0][0] = n1;
						n[0][1] = edg[n1].getType();
					}
					if (e1.equals(edg[n1])) {
						n[1][0] = n1;
						n[1][1] = 1;
					}
					if (e2.equals(edg[n1])) {
						n[2][0] = n1;
						n[2][1] = 1;
					}
					if (oldEdge1.equals(edg[n1])) {
						w[0] = n1;
					}
					if (oldEdge2.equals(edg[n1])) {
						w[1] = n1;
					}
				}
				//For New edge 1
				if (n[1][1] == 0) { //Means edge e1 doesnt exist
					e1.addOppVertices(OpplongVer);
					e1.setType(n[0][1]);
					edg.push_back(e1);
				}
				else { //Means that edge exist
					edg[n[1][0]].addOppVertices(OpplongVer);
				}
				//For new edge 2
				if (n[2][1] == 0) { //Means edge e2 doesnt exist
					e2.addOppVertices(OpplongVer);
					e2.setType(n[0][1]);
					edg.push_back(e2);
				}
				else { //Means that edge exist
					edg[n[2][0]].addOppVertices(OpplongVer);
				}

				//Updating oppVertices of old edges
				edg[w[0]].clearOppVertex(longEdge.getVertex2());
				edg[w[0]].addOppVertices(longVer);
				edg[w[1]].clearOppVertex(longEdge.getVertex1());
				edg[w[1]].addOppVertices(longVer);
				//Incrementing count as the longEdge has been used by this triangle
				edg[n[0][0]].incrementCount();
			}
			//If 2 new nodes have been added
			if (trgl[m].getNewVertices() == 2) {
				vector<Vertex> nodes = trgl[m].getOrderedNewNodes();
				Vertex longVer = nodes[0]; // Midpoint of longest edge
				Vertex otherVer = nodes[1]; //Other midpoint of triangle
				Vertex oppLongVer = trgl[m].getOppVertexofPoint(longVer);
				Vertex oppOtherVer = trgl[m].getOppVertexofPoint(otherVer);
				Vertex thirdVer = trgl[m].getThirdVertex(oppLongVer, oppOtherVer);
				//Creating new triangles and deleting old triangles
				Triangle t1(longVer, oppLongVer, oppOtherVer);
				Triangle t2(longVer, oppLongVer, otherVer);
				Triangle t3(longVer, otherVer, thirdVer);
				trgl.push_back(t1);
				trgl.push_back(t2);
				trgl.push_back(t3);
				trgl.erase(trgl.begin() + m);
				//Creating new edges (if they don't exist) and deletion of obsolete edges
				Edge e1(oppLongVer, longVer, 0);
				e1.addOppVertices(oppOtherVer);
				e1.addOppVertices(otherVer);
				edg.push_back(e1);
				Edge e2(otherVer, longVer, 0);
				e2.addOppVertices(oppLongVer);
				e2.addOppVertices(thirdVer);
				edg.push_back(e2);
				int n[6][2] = {};
				vector<Edge> e;
				Edge ee1(oppOtherVer, thirdVer);
				Edge ee2(thirdVer, oppLongVer);
				Edge ee3(oppOtherVer, longVer);
				Edge ee4(longVer, thirdVer);
				Edge ee5(thirdVer, otherVer);
				Edge ee6(otherVer, oppLongVer);
				ee3.addOppVertices(oppLongVer);
				ee4.addOppVertices(oppLongVer);
				ee5.addOppVertices(longVer);
				ee6.addOppVertices(longVer);
				e.push_back(ee1);
				e.push_back(ee2);
				e.push_back(ee3);
				e.push_back(ee4);
				e.push_back(ee5);
				e.push_back(ee6);
				//The 2nd column of n for pre-existing edges gives its type
				//for  new edges (bisected edges) stores checker, which indicates
				//whether the edge already exists or not

				//Creating old edge and updating it's oppVertex
				Edge oldEdge(oppOtherVer, oppLongVer);
				int w = 0; // To track oldEdge from edg
				for (int n1 = 0;n1 < edg.size();n1++) {
					for (int j = 0;j < 6;j++) {
						if (e[j].equals(edg[n1])) {
							if (j < 2) {
								n[j][0] = n1;
								n[j][1] = edg[n1].getType();
							}
							else {
								n[j][0] = n1;
								n[j][1] = 1;
							}
						}
					}
					if (oldEdge.equals(edg[n1])) {
						w = n1;
					}
				}
				for (int j = 2;j < 6;j++) {
					int g = 0;//e[2],e[3] are subdivisions of e[0]
					if (j > 3) { //e[4],e[5] are subdivisions of e[1]
						g = 1;
					}
					if (n[j][1] == 0) { //Means edge doesn't exist
						e[j].setType(n[g][1]);
						edg.push_back(e[j]);
					}
					else {//Means edge exists
						vector<Vertex> vec = e[j].getOppVertices();
						for (int f = 0;f < vec.size();f++) {
							edg[n[j][0]].addOppVertices(vec[f]);
						}
					}
				}
				//Updating oppVertex of oldEdge
				edg[w].clearOppVertex(thirdVer);
				edg[w].addOppVertices(longVer);

				for (int j = 0;j < 2;j++) {
					//Incrementing count for obsolete edges
					edg[n[j][0]].incrementCount();
				}
			}
			//If 3 new nodes have been added
			if (trgl[m].getNewVertices() == 3) {
				vector<Vertex> nodes = trgl[m].getOrderedNewNodes();
				Vertex longVer = nodes[0];
				Vertex otherVer1 = nodes[1];
				Vertex otherVer2 = nodes[2];
				Vertex oppLongVer = trgl[m].getOppVertexofPoint(longVer);
				Vertex oppOtherVer1 = trgl[m].getOppVertexofPoint(otherVer1);
				Vertex oppOtherVer2 = trgl[m].getOppVertexofPoint(otherVer2);
				//Creating new triangles and deleting old triangles
				Triangle t1(oppOtherVer1, longVer, otherVer2);
				Triangle t2(otherVer2, longVer, oppLongVer);
				Triangle t3(oppLongVer, longVer, otherVer1);
				Triangle t4(otherVer1, longVer, oppOtherVer2);
				trgl.push_back(t1);
				trgl.push_back(t2);
				trgl.push_back(t3);
				trgl.push_back(t4);
				trgl.erase(trgl.begin() + m);
				//Creating new edges (if they don't exist) and deletion of obsolete edges
				Edge e1(otherVer2, longVer, 0);
				e1.addOppVertices(oppOtherVer1);
				e1.addOppVertices(oppLongVer);
				edg.push_back(e1);
				Edge e2(oppLongVer, longVer, 0);
				e2.addOppVertices(otherVer2);
				e2.addOppVertices(otherVer1);
				edg.push_back(e2);
				Edge e3(otherVer1, longVer, 0);
				e3.addOppVertices(oppLongVer);
				e3.addOppVertices(oppOtherVer2);
				edg.push_back(e3);
				int n[9][2] = {};
				vector<Edge> e;
				Edge ee1(oppOtherVer1, oppOtherVer2);
				Edge ee2(oppOtherVer2, oppLongVer);
				Edge ee3(oppLongVer, oppOtherVer1);
				Edge ee4(oppOtherVer1, longVer);
				Edge ee5(longVer, oppOtherVer2);
				Edge ee6(oppOtherVer2, otherVer1);
				Edge ee7(otherVer1, oppLongVer);
				Edge ee8(oppLongVer, otherVer2);
				Edge ee9(otherVer2, oppOtherVer1);
				ee4.addOppVertices(otherVer2);
				ee5.addOppVertices(otherVer1);
				ee6.addOppVertices(longVer);
				ee7.addOppVertices(longVer);
				ee8.addOppVertices(longVer);
				ee9.addOppVertices(longVer);

				e.push_back(ee1);
				e.push_back(ee2);
				e.push_back(ee3);
				e.push_back(ee4);
				e.push_back(ee5);
				e.push_back(ee6);
				e.push_back(ee7);
				e.push_back(ee8);
				e.push_back(ee9);
				//The 2nd column of n for pre-existing edges gives its type
				//for  new edges (bisected edges) stores checker, which indicates
				//whether the edge already exists or not
				for (int n1 = 0;n1 < edg.size();n1++) {
					for (int j = 0;j < 9;j++) {
						if (e[j].equals(edg[n1])) {
							if (j < 3) {
								n[j][0] = n1;
								n[j][1] = edg[n1].getType();
							}
							else {
								n[j][0] = n1;
								n[j][1] = 1;
							}
						}
					}
				}
				for (int j = 3;j < 9;j++) {
					int g = 0;//e[3],e[4] are subdivisions of e[0]
					if (j > 4) {//e[5],e[6] are subdivisions of e[1]
						g = 1;
					}
					if (j > 6) {//e[7],e[8] are subdivisions of e[2]
						g = 2;
					}
					if (n[j][1] == 0) {//Means edge doesn't exist
						e[j].setType(n[g][1]);
						edg.push_back(e[j]);
					}
					else {//Means edge exists
						vector<Vertex> vec = e[j].getOppVertices();
						for (int f = 0;f < vec.size();f++) {
							edg[n[j][0]].addOppVertices(vec[f]);
						}
					}
				}
				for (int j = 0;j < 3;j++) {
					//Incrementing count for obsolete edges
					edg[n[j][0]].incrementCount();
				}
			}
			for (int h = 0;h < edg.size();h++) {
				//Deleting of obsolete edge based on type and count
				if (edg[h].getType() == 0) {
					if (edg[h].getCount() == 2) {
						edg.erase(edg.begin() + h);
						h = h - 1;
					}
				}
				else {
					if (edg[h].getCount() == 1) {
						edg.erase(edg.begin() + h);
						h = h - 1;
					}
				}
			}
			m = m - 1; //Better value cannot be taken
		}
	}
	//End of updating triangles
	cout << "No of Edges\t" << edg.size() << endl;
	cout << "No of Triangles\t" << trgl.size() << endl;
	cout << "Edge info\n";
	for (int k = 0;k < edg.size();k++) {
		vector<Vertex> oppVer = edg[k].getOppVertices();
		cout << edg[k].getVertex1().getX() << "\t" << edg[k].getVertex1().getY() << "\t";
		cout << edg[k].getVertex2().getX() << "\t" << edg[k].getVertex2().getY() << "\t" << edg[k].getType() << "\t";
		for (int l = 0;l < oppVer.size();l++) {
			cout << oppVer[l].getX() << "\t" << oppVer[l].getY() << "\t";
		}
		cout << "\n";
	}

	cout << "Triangle info" << endl;
	for (int k = 0;k < trgl.size();k++) {
		vector<Vertex> ver = trgl[k].getVertices();
		for (int j = 0;j < 3;j++) {
			cout << ver[j].getX() << "\t" << ver[j].getY() << "\t";
		}
		cout << "\n";
	}

	/*for(int k=0;k<trgl.size();k++){
		vector<Vertex> ver=trgl[k].getVertices();
		for(int j=0;j<3;j++){
			cout<<ver[j].getX()<<"\t"<<ver[j].getY()<<"\n";
		}
	}*/
}