#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <mpi.h>

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
	double getX() const{
		return x;
	}
	double getY() const{
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
	bool operator<(const Vertex& v2) const{
		double x2,y2;
		x2 = v2.getX();
		y2 = v2.getY();
		if(x<x2){
			return true;
		}
		else if(x==x2){
			if(y<y2){
				return true;
			}
			else{
				return false;
			}
		}
		else{
			return false;
		}
	}
};



/*Edge type can be used to implement different boundary conditions*/
class Edge{
	private:
	/*Variables*/
	Vertex v1;
	Vertex v2;
	int type; // type used to identify where the edge is in the domain
	// Assigning 0 to the edges which are in the interior
	int count; //Used to delete obsolete edges during bisection of triangles
	//Edge of type 0 becomes obsolete when count becomes 2
	//Edge of type Non Zero becomes obsolte when count becomes 1
	vector<Vertex> oppVer; //Opposite vertex to track the triangles of this edge
	bool mpiEdge; // To detect if it is an MPI Interface Edge or not
	//Only edges of type 0  can become MPI Interface Edges
	int mpiProcessor; //To know the processor of the mpi edge 
	bool mpiEdgeBisected; // To track if the mpiEdge is bisected or not
	int track; //This when set to 1 means it has communicated
	// with the other processor about its bisection
    public:
	/*Constructors*/
	Edge(Vertex vv1,Vertex vv2){
		v1=vv1;
		v2=vv2;
		type =0;
		count = 0;
		mpiEdge = false;
		mpiProcessor = 0;
		mpiEdgeBisected = false;
		track =0;
	}
	Edge(Vertex vv1,Vertex vv2, int t){
		v1=vv1;
		v2=vv2;
		type=t;
		count = 0;
		mpiEdge = false;
		mpiProcessor = 0;
		mpiEdgeBisected = false;
		track =0;
	}
    /*Members*/
    int getType(){
		return type;
	}
	void setType(int typ){
		type=typ;
	}
	double getLength(){
		double length=v1.getLength(v2);
		return length;
	}
	vector<Vertex> getVertices(){
		vector<Vertex> ver;
		ver.push_back(v1);
		ver.push_back(v2);
		return ver;
	}
	Vertex getVertex1(){
		return v1;
	}
	Vertex getVertex2(){
		return v2;
	}
	Vertex getMidpoint(){
		Vertex midpoint;
		midpoint.setXY(0.5*(v1.getX()+v2.getX()),0.5*(v1.getY()+v2.getY()));
		return midpoint;
	}
	void addOppVertices(Vertex ver){
		if(oppVer.size()==0){
			oppVer.push_back(ver);
		}
		else{
			int l=1;
			for(int d=0;d<oppVer.size();d++){
				if(oppVer[d].equals(ver)){
					l=0;
				}
			}
			if(l==1){
				oppVer.push_back(ver);
			}
		}
	}
	vector<Vertex> getOppVertices(){
		return oppVer;
	}
	void incrementCount(){
		count=count+1;
	}
	int getCount(){
		return count;
	}
	void clearOppVertex(Vertex ver){
		for(int i=0;i<oppVer.size();i++){
			if(ver.equals(oppVer[i])){
				oppVer.erase(oppVer.begin()+i);
			}
		}
	}
	bool equals(Edge e1){
		int i=0;
		vector<Vertex> e1v = e1.getVertices();
		Vertex e1v1=e1v[0];
		Vertex e1v2=e1v[1];
		if((v1.equals(e1v1))&&(v2.equals(e1v2))){
			i=1;
		}
		else{
			i=0;
		}
		
		if(i==0){
            if((v1.equals(e1v2))&&(v2.equals(e1v1))){
			i=1;
		    }
		    else{
			i=0;
		    }
		}
		
		if(i==1){
			return true;
		}
		else{
			return false;
		}
	}
	void setmpiEdge(bool des){
		mpiEdge = des;
	}
	bool getmpiEdge(){
		return mpiEdge;
	}
	void setmpiProcessor(int i1){
		mpiProcessor =i1;
	}
	int getmpiProcessor(){
		return mpiProcessor;
	}
	void bisectmpiEdge(bool dess){
		mpiEdgeBisected = dess;
	}
	bool getBisectionStateOfmpiEdge(){
		return mpiEdgeBisected;
	}
	void setTrack (int i1){
		track = i1;
	}
	int getTrack(){
		return track;
	}
};


class Triangle{
	private:
	Vertex v1;
	Vertex v2;
	Vertex v3;
	int newVertices; // Used during refinement
	vector<Vertex> newNodes; // Used during refinement
	bool refine; //Used during refinement
	int count; //Used to delete triangles
	//Obsolete triangles have count==1
	public:
	/*Constructor*/
	Triangle(Vertex vv1,Vertex vv2,Vertex vv3){
		v1=vv1;
		v2=vv2;
		v3=vv3;
		newVertices =0;
		refine = false;
		count =0;
	}
	/*Members*/
	vector<Vertex> getVertices(){
		vector<Vertex> ver;
		ver.push_back(v3);
		ver.push_back(v1);
		ver.push_back(v2);
		return ver;
	}
	Vertex getCentroid(){
		double x1,y1,x2,y2,x3,y3,xc,yc;
		x1=v1.getX();
		y1=v1.getY();
		x2=v2.getX();
		y2=v2.getY();
		x3=v3.getX();
		y3=v3.getY();
		xc=(x1+x2+x3)/3.0;
		yc=(y1+y2+y3)/3.0;
		Vertex ver(xc,yc);
		return ver;
	}
	vector<Edge> getEdges(){
		Edge e12(v1,v2);
	    Edge e23(v2,v3);
	    Edge e31(v3,v1);
		vector<Edge> ed;
		ed.push_back(e12);
		ed.push_back(e23);
		ed.push_back(e31);
		return ed;
	}
	double getArea(){
		double x1,y1,x2,y2,x3,y3,area;
		x1=v1.getX();
		y1=v1.getY();
		x2=v2.getX();
		y2=v2.getY();
		x3=v3.getX();
		y3=v3.getY();
		area=0.5*(x2*y3-x3*y2-x1*y3+x1*y2+x3*y1-x2*y1);
		area =fabs(area);
		return area;
	}
	Edge getLongestEdge(){
	    Edge e12(v1,v2);
	    Edge e23(v2,v3);
	    Edge e31(v3,v1);
		Edge le = e12;
		if(le.getLength() < e23.getLength()){
			le=e23;
		}
		if(le.getLength() < e31.getLength()){
			le=e31;
		}
		return le;
	}
	void addNewNode(Vertex v){
		if(newNodes.size()==0){
			newNodes.push_back(v);
		}
		else{
			int l=1;
			for(int j=0;j<newNodes.size();j++){
				if(newNodes[j].equals(v)){
					l=0;
				}
			}
			if(l==1){
				newNodes.push_back(v);
			}
		}
		newVertices = newNodes.size();
	}
	int getNewVertices(){
		return newVertices;
	}
	void setRefine(bool dec){
		refine=dec;
	}
	bool getRefine(){
		return refine;
	}
	Edge getEdgeOfPoint(Vertex ver){ //Used to find the edge on which a point (midpoint) lies
		Vertex v12,v23,v31;
		v12.setXY(0.5*(v1.getX()+v2.getX()),0.5*(v1.getY()+v2.getY()));
		v23.setXY(0.5*(v2.getX()+v3.getX()),0.5*(v2.getY()+v3.getY()));
		v31.setXY(0.5*(v3.getX()+v1.getX()),0.5*(v3.getY()+v1.getY()));
		if(v12.equals(ver)){
			Edge e12(v1,v2);
			return e12;
		}
		if(v23.equals(ver)){
			Edge e23(v2,v3);
			return e23;
		}
		if(v31.equals(ver)){
			Edge e31(v3,v1);
			return e31;
		}
	}
	Vertex getOppVertexofPoint(Vertex ver){ //Obtaining vertex of triangle which is opposite to an edge using the edge's midpoint
		Vertex v12,v23,v31,v;
		v12.setXY(0.5*(v1.getX()+v2.getX()),0.5*(v1.getY()+v2.getY()));
		v23.setXY(0.5*(v2.getX()+v3.getX()),0.5*(v2.getY()+v3.getY()));
		v31.setXY(0.5*(v3.getX()+v1.getX()),0.5*(v3.getY()+v1.getY()));
		if(v12.equals(ver)){
			v=v3;
		}
		if(v23.equals(ver)){
			v=v1;
		}
		if(v31.equals(ver)){
			v=v2;
		}
		return v;
	}
	vector<Vertex> getOrderedNewNodes(){ //Ordered in a way that the first node is the longest edge midpoint
		vector<Vertex> ver;
		if(newVertices>0){
			Vertex longPoint=this->getLongestEdge().getMidpoint();
			ver.push_back(longPoint);
			for(int n=0;n<newNodes.size();n++){
				if(!(longPoint.equals(newNodes[n]))){
					ver.push_back(newNodes[n]);
				}
			}
		}
		return ver;
	}
	vector<Vertex> getMidpointsOfEdges(){  //Used to obtain midpoints of edges of a triangle
		vector<Vertex> ver;
		Vertex v12,v23,v31;
		v12.setXY(0.5*(v1.getX()+v2.getX()),0.5*(v1.getY()+v2.getY()));
		v23.setXY(0.5*(v2.getX()+v3.getX()),0.5*(v2.getY()+v3.getY()));
		v31.setXY(0.5*(v3.getX()+v1.getX()),0.5*(v3.getY()+v1.getY()));
		ver.push_back(v12);
		ver.push_back(v23);
		ver.push_back(v31);
		return ver;
	}
	Vertex getThirdVertex(Vertex vv1,Vertex vv2){
		Vertex ver;
		ver.setXY(0.5*(vv1.getX()+vv2.getX()),0.5*(vv1.getY()+vv2.getY()));
		Vertex ver1=this->getOppVertexofPoint(ver);
		return ver1;
	}
	bool equals(Triangle t){
		Vertex ver=t.getCentroid();
		Vertex ver1=this->getCentroid();
		return ver1.equals(ver);
	}
	void incrementCount(){
		count =count+1;
	}
	int getCount(){
		return count;
	}
};

int main(int argc, char** argv){
	vector<Vertex> vertices0;
	vector<Edge> edg0;
	vector<Triangle> trgl0;
	MPI_Init(&argc, &argv);
	int num_process; // Number of processes
	MPI_Comm_size(MPI_COMM_WORLD, &num_process);
	int process_num; // Process number
	MPI_Comm_rank(MPI_COMM_WORLD, &process_num);
	MPI_Status status;
	vector<Vertex> vertices;
	vector<Edge> edg,edg2;
	vector<Triangle> trgl,trgl2;
	
	ifstream inFile;
	inFile.open("Mesh.msh");
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
					edg2.push_back(e);
				}
				else {
					inFile >> q;
					for (int w = 0;w < q;w++) {
						int q1;
						inFile >> q1;
					}
					inFile >> m >> n >> p;
					Triangle tre(vertices[m - 1], vertices[n - 1], vertices[p - 1]);
					trgl2.push_back(tre);
				}

			}
		}
	}
	inFile.close();
	//Test case setting two triangles to refine
	for(int i=0;i<trgl2.size();i++){
		trgl2[i].setRefine(true);
	}
	//trgl2[0].setRefine(true);
	//trgl2[1].setRefine(true);
	//trgl2[2].setRefine(true);
	//trgl2[3].setRefine(true);
	int triK, triL; //Used for distribution of triangle vector 
	triK = trgl2.size()/num_process;
	triL = trgl2.size()%num_process;
	//Adding triangles of this processor to trgl
	if(process_num<triL){
		for(int j=process_num*(triK+1);j<=process_num*(triK+1)+triK;j++){
			Triangle tq = trgl2[j];
			trgl.push_back(tq);
		}
	}
	else{
		for(int j=process_num*triK+triL;j<=process_num*triK+triL+triK-1;j++){
			Triangle tq = trgl2[j];
			trgl.push_back(tq);
		}
	}
	trgl2.clear();
	//Addding edges of this processor to edg
	for(int i = 0;i < trgl.size();i++) {
		Triangle t = trgl[i];
		vector<Edge> ed = t.getEdges();
		vector<Vertex> verr = t.getVertices();
		for (int j = 0;j < 3;j++) {
			Edge edd = ed[j];
			Vertex veer = verr[j];
			int l = 1;
			for(int k = 0;k<edg.size();k++){
				if(edd.equals(edg[k])){
					l=0;
					edg[k].addOppVertices(veer);
					break;
				}
			}
			for (int k = 0;k < edg2.size();k++) {
				if (edd.equals(edg2[k])) {
					l = 0;
					edg2[k].addOppVertices(veer);
					edg.push_back(edg2[k]);
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
	edg2.clear();
	//Adding of opposite vertices to edges is done
	
	//Using maps for edges and triangles
	map<Vertex,int> emap; //For edges
	map<Vertex,int> tmap; //For triangles
	
	//Updating emap
	for(int i=0;i<edg.size();i++){
		Vertex va = edg[i].getMidpoint();
		emap[va] = i;
	}
	
	//Updating tmap
	for(int i=0;i<trgl.size();i++){
		Vertex va = trgl[i].getCentroid();
		tmap[va] = i;
	}
	
	
	//Defining which edges of edg are mpiEdge and processor number in mpiProcessor
	for(int i=0;i<edg.size();i++){
		if(edg[i].getType()==0){
			vector<Vertex> verces = edg[i].getOppVertices();
			if(verces.size()==1){
				edg[i].setmpiEdge(true);
			    edg[i].setmpiProcessor(process_num);
			}
			if(verces.size()==0){
				cout<<"Wrong in mpiEdge of processor "<<process_num<<endl;
			}
		}
	}
	// mpiEdges have been detected
	
	//Refinement of triangles in trgl
	for (int i = 0;i < trgl.size();i++) {
		if (trgl[i].getRefine()) {
			vector<Vertex> ver = trgl[i].getMidpointsOfEdges();
			for (int j = 0;j < 3;j++) {
				trgl[i].addNewNode(ver[j]); //Adding the midpoints to triangles
				//Loop to collect all triangles getting affected by this node
				bool decision = true;
				Triangle t = trgl[i];//Current triangle
				while (decision) {
					//Finding the edge from edg which contains this point
					int k; //To track edge number
					k = emap[ver[j]];
					//Found the edge
					//Cheking if the edge is an mpiEdge
					//To set bisection to true
					if(edg[k].getmpiEdge()){
						edg[k].bisectmpiEdge(true);
					}
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
						Vertex va = neigTri[0].getCentroid();
						l = tmap[va];
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
	//Refinement of the triangles in trgl is done
	
	//Refinement of mpiEdges through exchange of information across processors
	vector<Edge> mpiedg;
	for(int i=0;i<edg.size();i++){
		if(edg[i].getmpiEdge()){
			mpiedg.push_back(edg[i]);
		}
	}

	vector<Edge> mpiedgtotal; //Collection of all mpi edges
	//This part has to be in a loop until 
	//there is no mpiedge which is bisected and has track = 0 
	bool deci =true;
	while(deci){
	    //Adding mpiedges of processor 0 to mpiedgtotal
	   if(process_num==0){
		    for(int i=0;i<mpiedg.size();i++){
			   mpiedgtotal.push_back(mpiedg[i]);
		    }
	    }
		
		if(process_num != 0 ){
			int si = mpiedg.size();
			//Sending size of mpiedg
			MPI_Send(&si,1,MPI_INT,0,0,MPI_COMM_WORLD);
			//Sending mpiedg to process 0
			for(int i=0;i<si;i++){
				//Individually sending details of every edge
				Vertex ve1 = mpiedg[i].getVertex1();
				Vertex ve2 = mpiedg[i].getVertex2();
				double X[4];
				X[0] = ve1.getX(); 
				X[1] = ve2.getX();
				X[2] = ve1.getY();
				X[3] = ve2.getY();
				int Y[3];
				Y[0] = mpiedg[i].getmpiProcessor();
				Y[1] = mpiedg[i].getTrack();
				Y[2]=0;//For bisection state
				if(mpiedg[i].getBisectionStateOfmpiEdge()){
					Y[2]=1;
				}
				MPI_Send(&X[0],4,MPI_DOUBLE,0,1,MPI_COMM_WORLD);
				MPI_Send(&Y[0],3,MPI_INT,0,1,MPI_COMM_WORLD);
			}
			//Clearing mpiedg so that updated mpiedg can be received
			mpiedg.clear();
			//Receiving updated mpiedg vector
			for(int i=0;i<si;i++){
				double X[4];
				int Y[3];
				//Don't forget setting mpiEdge to true
				MPI_Recv(&X[0],4,MPI_DOUBLE,0,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				MPI_Recv(&Y[0],3,MPI_INT,0,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				Vertex ve1(X[0],X[2]),ve2(X[1],X[3]);
				Edge ed(ve1,ve2,0);
				ed.setmpiEdge(true);
				ed.setmpiProcessor(Y[0]);
				ed.setTrack(Y[1]);
				if(Y[2]==1){
					ed.bisectmpiEdge(true);
				}
				mpiedg.push_back(ed);
			}
		}
		else{
			//Collecting mpiedgs from all processes
			for(int i=1;i<num_process;i++){
				int j;
				MPI_Recv(&j,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				for(int k=0;k<j;k++){
					double X[4];
					int Y[3];
					//Don't forget setting mpiEdge to true
					MPI_Recv(&X,4,MPI_DOUBLE,i,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
					MPI_Recv(&Y,3,MPI_INT,i,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
					Vertex ve1(X[0],X[2]),ve2(X[1],X[3]);
					Edge ed(ve1,ve2);
					ed.setmpiEdge(true);
					ed.setmpiProcessor(Y[0]);
					ed.setTrack(Y[1]);
					if(Y[2]==1){
						ed.bisectmpiEdge(true);
					}
					mpiedgtotal.push_back(ed);
				}
			}
			//Communication between similar mpiedgs
			for(int i=0;i<mpiedgtotal.size();i++){
				for(int j=0;j<mpiedgtotal.size();j++){
					if(i!=j){
						if(mpiedgtotal[i].equals(mpiedgtotal[j])){
							bool d1 =mpiedgtotal[i].getBisectionStateOfmpiEdge();
							bool d2 =mpiedgtotal[j].getBisectionStateOfmpiEdge();
							if(d1 || d2){
								mpiedgtotal[i].bisectmpiEdge(true);
								mpiedgtotal[i].setTrack(1);
								mpiedgtotal[j].bisectmpiEdge(true);
								mpiedgtotal[j].setTrack(1);
							}
						}
					}
				}
			}
			
			//Redistribution of mpiedg from process 0 to respective processor
			mpiedg.clear(); //This only clears mpiedg info of process 0
			for(int i=0;i<mpiedgtotal.size();i++){
				int j;
				j = mpiedgtotal[i].getmpiProcessor();
				if(j==0){
					mpiedg.push_back(mpiedgtotal[i]);
				}
				else{
					//Individually sending details of every edge
					Vertex ve1 = mpiedgtotal[i].getVertex1();
					Vertex ve2 = mpiedgtotal[i].getVertex2();
					double X[4];
					X[0] = ve1.getX(); 
					X[1] = ve2.getX();
					X[2] = ve1.getY();
					X[3] = ve2.getY();
					int Y[3];
					Y[0] = mpiedgtotal[i].getmpiProcessor();
					Y[1] = mpiedgtotal[i].getTrack();
					Y[2]=0;//For bisection state
					if(mpiedgtotal[i].getBisectionStateOfmpiEdge()){
						Y[2]=1;
					}
					MPI_Send(&X[0],4,MPI_DOUBLE,j,2,MPI_COMM_WORLD);
					MPI_Send(&Y[0],3,MPI_INT,j,2,MPI_COMM_WORLD);
				}
			}
			//Clearing mpiedgtotal
			mpiedgtotal.clear();
		}
		//Relaying information from mpiedg to edg
		//Changing mpiEdgeBisected and track
		for(int i=0;i<mpiedg.size();i++){
			Vertex val = mpiedg[i].getMidpoint();
			int j;
			j = emap[val];
			if(mpiedg[i].getBisectionStateOfmpiEdge()){
				edg[j].bisectmpiEdge(true);
				edg[j].setTrack(1);
			}
		}
		
		//Every processor going through its own mpiedg vector
		//to start refinement based on bisected edges
		for(int i=0;i<mpiedg.size();i++){
			if(mpiedg[i].getBisectionStateOfmpiEdge()){
				Vertex v1 = mpiedg[i].getMidpoint();
				int ej; 
				ej = emap[v1]; //Obtained corresponding edge
				vector<Vertex> ever = edg[ej].getOppVertices();
				Vertex ever1 = edg[ej].getVertex1();
				Vertex ever2 = edg[ej].getVertex2();
				Triangle tj(ever1,ever2,ever[0]);
				Vertex vva = tj.getCentroid();
				int jj;
				jj = tmap[vva];
				trgl[jj].addNewNode(v1);
				if(!(mpiedg[i].equals(trgl[jj].getLongestEdge()))){
					Vertex v2 = trgl[jj].getLongestEdge().getMidpoint();
					trgl[jj].addNewNode(v2);
					//Process is similar to normal refinement
					bool des = true;
					Triangle tt = trgl[jj];
					while(des){
						int k;
						k = emap[v2];
						if(edg[k].getmpiEdge()){
							edg[k].bisectmpiEdge(true);
						}
						
						vector<Vertex> oppVer = edg[k].getOppVertices();
						if(oppVer.size()>2){
							cout<<"Opposite Vertices count is more than 2 in "<<process_num<<endl;
						}
						vector<Triangle> neigTri;
						if(oppVer.size()==2){
						  Triangle Tri1(edg[k].getVertex1(), edg[k].getVertex2(), oppVer[0]);
						  Triangle Tri2(edg[k].getVertex1(), edg[k].getVertex2(), oppVer[1]);
						    if (Tri1.equals(tt)) {
							  neigTri.push_back(Tri2);
						    }
						    if (Tri2.equals(tt)) {
							  neigTri.push_back(Tri1);
						    }							
						}
						if(neigTri.size()==0){
							des = false;
						}
						int l;
					    if (neigTri.size() == 1) {
							Vertex va1 = neigTri[0].getCentroid();
							l = tmap[va1];
						    trgl[l].addNewNode(v2);
						    des = !(v2.equals(trgl[l].getLongestEdge().getMidpoint()));
							if(des){
								trgl[l].addNewNode(trgl[l].getLongestEdge().getMidpoint());
								v2 = trgl[l].getLongestEdge().getMidpoint();
								tt = trgl[l];
							}
					    }						
					}
				}
			}
		}
		
		//Clearing mpiedg and updating it
		mpiedg.clear();
		for(int i=0;i<edg.size();i++){
			if(edg[i].getmpiEdge()){
				mpiedg.push_back(edg[i]);
			}
		}
		
		deci = false;
		for(int i=0;i<mpiedg.size();i++){
			if(mpiedg[i].getBisectionStateOfmpiEdge()){
				if(mpiedg[i].getTrack()==0){
					//Denotes that new mpi Edge
					//has been bisected only in this process
					deci =true;
					break;
				}
			}
		} 
		
		//Every  process sending its deci state to process 0
		//Essential for all processes to run even when 
		//only few proesses come across deci==true
		int gg=0;
		if(deci==true){
			gg=1;
		}
		
		if(process_num != 0){
			MPI_Send(&gg,1,MPI_INT,0,4,MPI_COMM_WORLD);
			MPI_Recv(&gg,1,MPI_INT,0,5,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		}
		else{
			int gg1=0;
			vector<int> glist;
			glist.push_back(gg);
			for(int i=1;i<num_process;i++){
				MPI_Recv(&gg1,1,MPI_INT,i,4,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				glist.push_back(gg1);
			}
			gg1=0;
			for(int i=0;i<glist.size();i++){
				if(glist[i]==1){
					gg1=1;
					break;
				}
			}
			gg =gg1; //For process 0
			for(int i=1;i<num_process;i++){
				MPI_Send(&gg1,1,MPI_INT,i,5,MPI_COMM_WORLD);
			}
		}
		if(gg==1){
			deci = true;
		}
		else{
			deci = false;
		}
	}
	
	//Clearing map of triangles
	tmap.clear();
	
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
				//Creating map location for newEdge
				emap[newEdge.getMidpoint()] = edg.size()-1;
				//Creating new triangles with refine as false,false is preassigned and deleting the old one
				Triangle t1(longVer, OpplongVer, longEdge.getVertex1());
				Triangle t2(longVer, OpplongVer, longEdge.getVertex2());
				trgl.push_back(t1);
				//Creating map location for Triangles
				//tmap[t1.getCentroid()] = trgl.size()-1;
				trgl.push_back(t2);
				//Creating map location for Triangles
				//tmap[t2.getCentroid()] = trgl.size()-1;
				trgl.erase(trgl.begin() + m);
				
				//Incrementing the count of triangle at m
				//trgl[m].incrementCount();
				
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
				//Using maps instead of loops
				n[0][0] = emap[longEdge.getMidpoint()];
				n[0][1] = edg[n[0][0]].getType();
				if(emap.count(e1.getMidpoint())>0){
					n[1][0]=emap[e1.getMidpoint()];
					n[1][1] = 1;
				}
				if(emap.count(e2.getMidpoint())>0){
					n[2][0]=emap[e2.getMidpoint()];
					n[2][1] = 1;
				}
				w[0] = emap[oldEdge1.getMidpoint()];
				w[1] = emap[oldEdge2.getMidpoint()];
				//For New edge 1
				if (n[1][1] == 0) { //Means edge e1 doesnt exist
					e1.addOppVertices(OpplongVer);
					e1.setType(n[0][1]);
					//These properties are coz of MPI
					if(edg[n[0][0]].getmpiEdge()){
						e1.setmpiEdge(true);
						e1.setmpiProcessor(edg[n[0][0]].getmpiProcessor());
					}
					edg.push_back(e1);
					emap[e1.getMidpoint()] = edg.size()-1;
				}
				else { //Means that edge exist
					edg[n[1][0]].addOppVertices(OpplongVer);
				}
				//For new edge 2
				if (n[2][1] == 0) { //Means edge e2 doesnt exist
					e2.addOppVertices(OpplongVer);
					e2.setType(n[0][1]);
					//These properties are coz of MPI
					if(edg[n[0][0]].getmpiEdge()){
						e2.setmpiEdge(true);
						e2.setmpiProcessor(edg[n[0][0]].getmpiProcessor());
					}
					edg.push_back(e2);
					emap[e2.getMidpoint()] = edg.size()-1;
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
				//Creating new triangles and incrementing count of old triangles
				Triangle t1(longVer, oppLongVer, oppOtherVer);
				Triangle t2(longVer, oppLongVer, otherVer);
				Triangle t3(longVer, otherVer, thirdVer);
				trgl.push_back(t1);
				//tmap[t1.getCentroid()]  = trgl.size()-1;
				trgl.push_back(t2);
				//tmap[t2.getCentroid()]  = trgl.size()-1;
				trgl.push_back(t3);
				//tmap[t3.getCentroid()]  = trgl.size()-1;
				trgl.erase(trgl.begin() + m);
				//trgl[m].incrementCount();
				
				//Creating new edges (if they don't exist) and deletion of obsolete edges
				Edge e1(oppLongVer, longVer, 0);
				e1.addOppVertices(oppOtherVer);
				e1.addOppVertices(otherVer);
				edg.push_back(e1);
				emap[e1.getMidpoint()] = edg.size()-1;
				Edge e2(otherVer, longVer, 0);
				e2.addOppVertices(oppLongVer);
				e2.addOppVertices(thirdVer);
				edg.push_back(e2);
				emap[e2.getMidpoint()] = edg.size()-1;
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
				//Using maps instead of loop
				n[0][0] = emap[e[0].getMidpoint()];
				n[0][1] = edg[n[0][0]].getType();
				n[1][0] = emap[e[1].getMidpoint()];
				n[1][1] = edg[n[1][0]].getType();
				if(emap.count(e[2].getMidpoint())>0){
					n[2][0] = emap[e[2].getMidpoint()];
					n[2][1] = 1;
				}
				if(emap.count(e[3].getMidpoint())>0){
					n[3][0] = emap[e[3].getMidpoint()];
					n[3][1] = 1;
				}
				if(emap.count(e[4].getMidpoint())>0){
					n[4][0] = emap[e[4].getMidpoint()];
					n[4][1] = 1;
				}
				if(emap.count(e[5].getMidpoint())>0){
					n[5][0] = emap[e[5].getMidpoint()];
					n[5][1] = 1;
				}
				w = emap[oldEdge.getMidpoint()];

				for (int j = 2;j < 6;j++) {
					int g = 0;//e[2],e[3] are subdivisions of e[0]
					if (j > 3) { //e[4],e[5] are subdivisions of e[1]
						g = 1;
					}
					if (n[j][1] == 0) { //Means edge doesn't exist
						e[j].setType(n[g][1]);
						edg.push_back(e[j]);
						//These properties are coz of MPI
					    if(edg[n[g][0]].getmpiEdge()){
						 e[j].setmpiEdge(true);
						 e[j].setmpiProcessor(edg[n[g][0]].getmpiProcessor());
					    }
						emap[e[j].getMidpoint()] = edg.size()-1;
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
				//tmap[t1.getCentroid()] = trgl.size()-1;
				trgl.push_back(t2);
				//tmap[t2.getCentroid()] = trgl.size()-1;
				trgl.push_back(t3);
				//tmap[t3.getCentroid()] = trgl.size()-1;
				trgl.push_back(t4);
				//tmap[t4.getCentroid()] = trgl.size()-1;
				trgl.erase(trgl.begin() + m);
				//trgl[m].incrementCount();
				//Creating new edges (if they don't exist) and deletion of obsolete edges
				Edge e1(otherVer2, longVer, 0);
				e1.addOppVertices(oppOtherVer1);
				e1.addOppVertices(oppLongVer);
				edg.push_back(e1);
				emap[e1.getMidpoint()] = edg.size()-1;
				Edge e2(oppLongVer, longVer, 0);
				e2.addOppVertices(otherVer2);
				e2.addOppVertices(otherVer1);
				edg.push_back(e2);
				emap[e2.getMidpoint()] = edg.size()-1;
				Edge e3(otherVer1, longVer, 0);
				e3.addOppVertices(oppLongVer);
				e3.addOppVertices(oppOtherVer2);
				edg.push_back(e3);
				emap[e3.getMidpoint()] = edg.size()-1;
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
				n[0][0] = emap[e[0].getMidpoint()];
				n[0][1] = edg[n[0][0]].getType();
				n[1][0] = emap[e[1].getMidpoint()];
				n[1][1] = edg[n[1][0]].getType();
				n[2][0] = emap[e[2].getMidpoint()];
				n[2][1] = edg[n[2][0]].getType();
				if(emap.count(e[3].getMidpoint())>0){
					n[3][0] = emap[e[3].getMidpoint()];
					n[3][1] = 1;
				}
				if(emap.count(e[4].getMidpoint())>0){
					n[4][0] = emap[e[4].getMidpoint()];
					n[4][1] = 1;
				}
				if(emap.count(e[5].getMidpoint())>0){
					n[5][0] = emap[e[5].getMidpoint()];
					n[5][1] = 1;
				}
				if(emap.count(e[6].getMidpoint())>0){
					n[6][0] = emap[e[6].getMidpoint()];
					n[6][1] = 1;
				}
				if(emap.count(e[7].getMidpoint())>0){
					n[7][0] = emap[e[7].getMidpoint()];
					n[7][1] = 1;
				}
				if(emap.count(e[8].getMidpoint())>0){
					n[8][0] = emap[e[8].getMidpoint()];
					n[8][1] = 1;
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
						//These properties are coz of MPI
					    if(edg[n[g][0]].getmpiEdge()){
						 e[j].setmpiEdge(true);
						 e[j].setmpiProcessor(edg[n[g][0]].getmpiProcessor());
					    }
						emap[e[j].getMidpoint()] = edg.size()-1;
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
			m=m-1;
		}
	}
	
	for (int h = 0;h < edg.size();h++) {
		//Deleting of obsolete edge based on type and count
		//mpiEdges become obsolete when their count becomes 1 and not 2
		if (edg[h].getType() == 0) {
			if(edg[h].getmpiEdge()){
				if (edg[h].getCount() == 1) {
				    edg.erase(edg.begin() + h);
					h = h - 1;
				}
			}
			else{
				if (edg[h].getCount() == 2) {
				    edg.erase(edg.begin() + h);
					h = h - 1;
				}
			}
		}
		else {
			if (edg[h].getCount() == 1) {
				edg.erase(edg.begin() + h);
				h = h - 1;
			}
		}
	}
	//End of updating triangles
	
	//Clearing maps for edges
	emap.clear();
	//tmap.clear();
	
	//Transfering non zero edges from all processes to 
	//process 0
	if(process_num != 0){
		int es =0;
		for(int i=0;i < edg.size();i++){
			if(edg[i].getType() != 0){
				es=es+1;
			}
		}
		MPI_Send(&es,1,MPI_INT,0,6,MPI_COMM_WORLD);
		for(int i=0;i<edg.size();i++){
			if(edg[i].getType() != 0){
				Vertex vee1 = edg[i].getVertex1();
				Vertex vee2 = edg[i].getVertex2();
				double X[4];
				X[0]=vee1.getX();
				X[1]=vee2.getX();
				X[2]=vee1.getY();
				X[3]=vee2.getY();
				int Y;
				Y = edg[i].getType();
				MPI_Send(&X[0],4,MPI_DOUBLE,0,7,MPI_COMM_WORLD);
				MPI_Send(&Y,1,MPI_INT,0,77,MPI_COMM_WORLD);
			}
		}
	}
	else{
		for(int i=1;i<num_process;i++){
			int es;
			MPI_Recv(&es,1,MPI_INT,i,6,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			for(int j=0;j<es;j++){
				double X[4];
				int Y;
				MPI_Recv(&X[0],4,MPI_DOUBLE,i,7,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				MPI_Recv(&Y,1,MPI_INT,i,77,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				Vertex vee1(X[0],X[2]);
				Vertex vee2(X[1],X[3]);
				Edge  ev(vee1,vee2,Y);
				edg.push_back(ev);
			}
		}
	}
	
	//Transfering all triangles from processes to
	//process 0
	if(process_num != 0){
		int ts;
		ts = trgl.size();
		MPI_Send(&ts,1,MPI_INT,0,8,MPI_COMM_WORLD);
		for(int i=0;i<trgl.size();i++){
			vector<Vertex> tver = trgl[i].getVertices();
			double X[6];
			X[0] = tver[0].getX();
			X[1] = tver[0].getY();
			X[2] = tver[1].getX();
			X[3] = tver[1].getY();
			X[4] =  tver[2].getX();
			X[5] = tver[2].getY();
			MPI_Send(&X[0],6,MPI_DOUBLE,0,9,MPI_COMM_WORLD);
		}
	}else{
		int ts=0;
		for(int i=1;i<num_process;i++){
			MPI_Recv(&ts,1,MPI_INT,i,8,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			for(int j=0;j<ts;j++){
				double X[6];
				MPI_Recv(&X[0],6,MPI_DOUBLE,i,9,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				Vertex vq1(X[0],X[1]);
				Vertex vq2(X[2],X[3]);
				Vertex vq3(X[4],X[5]);
				Triangle tr(vq1,vq2,vq3);
				trgl.push_back(tr);
			}
		}
		
	}
	
	if(process_num == 0){
		//Creating new output file for the new mesh
		vector<Vertex> newver;
		map<Vertex,int> vmap;
		for(int i=0;i<trgl.size();i++){
			vector<Vertex> vex = trgl[i].getVertices();
			for(int j=0;j<3;j++){
				int l=0;
				if(vmap.count(vex[j])>0){
					l=1;
				}
				if(l==0){
					newver.push_back(vex[j]);
					vmap[vex[j]] = newver.size();
				}
			}
		}
		//Storing all non zero type edges
		vector<Edge> newedg;
		for(int i=0;i<edg.size();i++){
			if(edg[i].getType()!=0){
				newedg.push_back(edg[i]);
			}
		}
		//Retrieving physical name number of surface
		ifstream ifil;
		ifil.open("Mesh.msh");
		string cc;
		int trno; //To track the physical name number of surface
		while(ifil>>cc){
			if(cc.compare("$PhysicalNames")==0){
				int tot;
				ifil>>tot;
				for(int i=0;i<tot;i++){
					int j1,j2;
					string c1;
					ifil>>j1;
					if(j1==1){
						ifil>>j2;
						ifil>>c1;
					}
					if(j1==2){
						ifil>>trno;
						ifil>>c1;
					}
				}
				break;
			}
		}
		ifil.close();
		
		setprecision(10); //Setting precision for output file
		ifstream ifile;
		ifile.open("Mesh.msh");
		if(!ifile.is_open()){
			cout<<"File didn't open 825\n";
			exit(EXIT_FAILURE);
		}
		ofstream ofile; //To create new file
		ofile.open("NewMesh.msh");
		string lin;
		while(getline(ifile,lin)){
			istringstream iss(lin);
			string li;
			ofile<<lin<<endl;
			iss >> li;
			if(li.compare("$EndPhysicalNames")==0){
				break;
			}
		}
		ifile.close();
		ofile<<"$Nodes"<<endl;
		ofile<<newver.size()<<endl;
		for(int i=0;i<newver.size();i++){
			ofile<<vmap[newver[i]]<<" "<<newver[i].getX()<<" "<<newver[i].getY()<<" 0\n";
		}
		ofile<<"$EndNodes"<<endl;
		ofile<<"$Elements"<<endl;
		ofile<<newedg.size()+trgl.size()<<endl;
		for(int i=0;i<newedg.size()+trgl.size();i++){
			if(i<newedg.size()){
				int j1,j2;
				vector<Vertex> vexx = newedg[i].getVertices();
				j1 = vmap[vexx[0]];
				j2 = vmap[vexx[1]];
				ofile<<i+1<<" 1 2 "<<newedg[i].getType();
				ofile<<" "<<newedg[i].getType()+1<<" "<<j1<<" "<<j2<<endl;
			}
			else{
				int j1,j2,j3,jj2,jj3;
				vector<Vertex> vexx = trgl[i-newedg.size()].getVertices();
				j1=vmap[vexx[0]];
				j2=vmap[vexx[1]];
				j3=vmap[vexx[2]];
				double xx1,xx2,xx3,yy1,yy2,yy3,x21,y21,x31,y31,d;
				xx1 = vexx[0].getX();
				yy1 = vexx[0].getY();
				xx2 = vexx[1].getX();
				yy2 = vexx[1].getY();
				xx3 = vexx[2].getX();
				yy3 = vexx[2].getY();
				
				x21 = xx2-xx1;
				y21 = yy2-yy1;
				x31 = xx3-xx1;
				y31 = yy3-yy1;
				d = x21*y31-x31*y21;
				if(d>0){
					jj2=j2;
					jj3 = j3;
				}
				else{
					jj2 = j3;
					jj3 = j2;
				}
				
				ofile<<i+1<<" 2 2 "<<trno<<" "<<1<<" "<<j1;
				ofile<<" "<<jj2<<" "<<jj3<<endl;
			}
		}
		ofile<<"$EndElements"<<endl;
		ofile.close();
	}
	MPI_Finalize();
}
