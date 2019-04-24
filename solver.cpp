#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
using namespace std;
int main(){
	vector<string> phyName;
	ifstream inFile;
	inFile.open("Mesh.msh");
	if (!inFile.is_open()) {
		cout << "File didn't open\n";
		exit(EXIT_FAILURE);
	}
	string c;
	while(inFile>>c){
		if(c.compare("$PhysicalNames")==0){
			int i1,i2;
			inFile >> i1;
			for(int i=0;i<i1;i++){
				inFile>>i2>>i2;
				string c1;
				inFile>>c1;
				phyName.push_back(c1);
			}
			break;
		}
	}
	inFile.close();
	
	double **phy;
	phy = (double**)malloc(phyName.size()*sizeof(double*));
	for(int i=0;i<phyName.size();i++){
		phy[i] = (double*)malloc(2*sizeof(double));
	}
	inFile.open("BoundaryConditions.txt");
	if (!inFile.is_open()) {
		cout << "File didn't open\n";
		exit(EXIT_FAILURE);
	}
	
	//Reading corresponding values for boundaries
	// and domain. Same order as .msh file has to be followed 
	int trk =0;
	while(inFile>>c){
		string c1(&c[1],&c[5]);
		double i1,i2;
		if(c1.compare("Inte")==0){
			inFile>>i1>>i2;
			phy[trk][0] = i1;
			phy[trk][1] = i2;
		}
		if(c1.compare("Flux")==0){
			inFile>>i1;
			phy[trk][0] = i1;
			phy[trk][1] = 0.;
		}
		if(c1.compare("Conv")==0){
			inFile>>i1>>i2;
			phy[trk][0] = i1;
			phy[trk][1] = i2;
		}
		if(c1.compare("Temp")==0){
			inFile>>i1;
			phy[trk][0] = i1;
			phy[trk][1] = 0.;
		}
		trk = trk+1;
	}
	inFile.close();
	
	inFile.open("Mesh.msh");
	if (!inFile.is_open()) {
		cout << "File didn't open\n";
		exit(EXIT_FAILURE);
	}
	double** nodes;
	int nn; //No of nodes
	int** elms;
	int nel; //No of elements
	while(inFile>>c){
		if(c.compare("$Nodes")==0){
			inFile>>nn;
			nodes = (double**)malloc(nn*sizeof(double*));
			for(int i=0;i<nn;i++){
				nodes[i] = (double*)malloc(2*sizeof(double));
			}
			for(int i=0;i<nn;i++){
				int i1;
				double j1,j2,j3;
				inFile>>i1>>j1>>j2>>j3;
				nodes[i][0] = j1;
				nodes[i][1] = j2;
			}
		}
		if(c.compare("$Elements")==0){
			inFile>>nel;
			elms = (int**)malloc(nel*sizeof(int*));
			for(int i=0;i<nel;i++){
				elms[i] = (int*)malloc(5*sizeof(int));
			}
			for(int i=0;i<nel;i++){
				int i1,i2,i3,i4,i5,i6,i7,i8;
				inFile>>i1>>i2>>i3>>i4>>i5;
				if(i2==1){
					inFile>>i6>>i7;
					elms[i][0] = i2; // Edge or triangle (1 or 2)
					elms[i][1] = i4; //Type number
					elms[i][2] = i6; //Node 1
					elms[i][3] = i7; //Node 2
					elms[i][4] = 0;
				}
				else{
					inFile>>i6>>i7>>i8;
					elms[i][0] = i2; // Edge or triangle (1 or 2)
					elms[i][1] = i4; //Type number
					elms[i][2] = i6; //Node 1
					elms[i][3] = i7; //Node 2
					elms[i][4] = i8; //Node 3
				}
			}
			break;
		}
	}
	inFile.close();
	
	//Things to remember
	//Type number details in phy[TypeNumber-1] and phyName[TypeNumber-1]
	//Node details are in nodes[NodeNumber-1]
	
	//Solver part
	//[a] {t} = {b}
	//Applying Gauss seidel Iterative Scheme
	double converg = 0.001; //Desired level of convergence
	double** a;
	a = (double**)malloc(nn*sizeof(double*));
	for(int i=0;i<nn;i++){
		a[i] = (double*)malloc(nn*sizeof(double));
		for(int j=0;j<nn;j++){
			a[i][j] =0.;
		}
	}
	double** b;
	b = (double**)malloc(nn*sizeof(double*));
	for(int i=0;i<nn;i++){
		b[i] = (double*)malloc(1*sizeof(double));
		b[i][0] =0.;
	}
	double** t;
	t = (double**)malloc(nn*sizeof(double*));
	for(int i=0;i<nn;i++){
		t[i] = (double*)malloc(1*sizeof(double));
		t[i][0] =0.;
	}
	int** tcheck;
	tcheck = (int**)malloc(nn*sizeof(int*));
	for(int i=0;i<nn;i++){
		tcheck[i] = (int*)malloc(1*sizeof(int));
		tcheck[i][0] = 0;
	}
	
	int notri=0; //No of triangles
	for(int i=0;i<nel;i++){
		int type = elms[i][1];
		string name = phyName[type-1];
		string c2(&name[1],&name[5]);
		if(c2.compare("Inte")==0){
			notri = notri+1;
			int nd1,nd2,nd3;
			nd1 = elms[i][2];
			nd2 = elms[i][3];
			nd3 = elms[i][4];
			double x1,x2,x3,y1,y2,y3;
			x1 = nodes[nd1-1][0];
			y1 = nodes[nd1-1][1];
			x2 = nodes[nd2-1][0];
			y2 = nodes[nd2-1][1];
			x3 = nodes[nd3-1][0];
			y3 = nodes[nd3-1][1];
			double f1,f2,f3,b1,b2,b3,c1,c2,c3;
			f1 = x2*y3-x3*y2;
			f2 = x3*y1-x1*y3;
			f3 = x1*y2-x2*y1;
			b1 = y2-y3;
			b2 = y3-y1;
			b3 = y1-y2;
			c1 = x3-x2;
			c2 = x1-x3;
			c3 = x2-x1;
			double area = 0.5*(x2*y3-x3*y2-x1*y3+x1*y2+x3*y1-x2*y1);
			double keff = phy[type-1][0]; //Thermal Diffusivity
			double qeff = phy[type-1][1]; //Heat Generation
			keff = keff/(4.*area);
			a[nd1-1][nd1-1] = a[nd1-1][nd1-1]+keff*(b1*b1+c1*c1);
			a[nd1-1][nd2-1] = a[nd1-1][nd2-1]+keff*(b1*b2+c1*c2);
			a[nd1-1][nd3-1] = a[nd1-1][nd3-1]+keff*(b1*b3+c1*c3);
			a[nd2-1][nd1-1] = a[nd2-1][nd1-1]+keff*(b1*b2+c1*c2);
			a[nd2-1][nd2-1] = a[nd2-1][nd2-1]+keff*(b2*b2+c2*c2);
			a[nd2-1][nd3-1] = a[nd2-1][nd3-1]+keff*(b2*b3+c2*c3);
			a[nd3-1][nd1-1] = a[nd3-1][nd1-1]+keff*(b1*b3+c1*c3);
			a[nd3-1][nd2-1] = a[nd3-1][nd2-1]+keff*(b2*b3+c2*c3);
			a[nd3-1][nd3-1] = a[nd3-1][nd3-1]+keff*(b3*b3+c3*c3);
			qeff = qeff*(area/3.);
			b[nd1-1][0] = b[nd1-1][0]+qeff;
			b[nd2-1][0] = b[nd2-1][0]+qeff;
			b[nd3-1][0] = b[nd3-1][0]+qeff;
		}
		if(c2.compare("Flux")==0){
			double q0eff = phy[type-1][0];
			int nd1,nd2;
			nd1 = elms[i][2];
			nd2 = elms[i][3];
			double x1,x2,y2,y1;
			x1 = nodes[nd1-1][0];
			y1 = nodes[nd1-1][1];
			x2 = nodes[nd2-1][0];
			y2 = nodes[nd2-1][1];
			double len = (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);
			len = sqrt(len);
			q0eff = 0.5*q0eff*len;
			b[nd1-1][0] = b[nd1-1][0]+q0eff;
			b[nd2-1][0] = b[nd2-1][0]+q0eff;
		}
		if(c2.compare("Conv")==0){
			double h = phy[type-1][0]; //h for the element
			double t_inf = phy[type-1][1]; //Ambient Temp
			int nd1,nd2;
			nd1 = elms[i][2];
			nd2 = elms[i][3];
			double x1,x2,y2,y1;
			x1 = nodes[nd1-1][0];
			y1 = nodes[nd1-1][1];
			x2 = nodes[nd2-1][0];
			y2 = nodes[nd2-1][1];
			double len = (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);
			len = sqrt(len);
			//Contribution to LHS
			double h1eff = (h/6.)*len;
			a[nd1-1][nd1-1] = a[nd1-1][nd1-1]+h1eff*2;
			a[nd1-1][nd2-1] = a[nd1-1][nd2-1]+h1eff;
			a[nd2-1][nd1-1] = a[nd2-1][nd1-1]+h1eff;
			a[nd2-1][nd2-1] = a[nd2-1][nd2-1]+h1eff*2;
			//Contribution to RHS
			double h2eff = 0.5*h*t_inf*len;
			b[nd1-1][0] = b[nd1-1][0]+h2eff;
			b[nd2-1][0] = b[nd2-1][0]+h2eff;
		}
		if(c2.compare("Temp")==0){
			double tem = phy[type-1][0];
			int nd1,nd2;
			nd1 = elms[i][2];
			nd2 = elms[i][3];
			tcheck[nd1-1][0] = 1;
			tcheck[nd2-1][0] = 1;
			t[nd1-1][0] = tem;
			t[nd2-1][0] = tem;
		}
	}
	
	double tcon =10.;
	//Solver might actually need diagonal dominant condition 
	while(tcon>converg){
		double tcon_sum =0.;
		for(int i=0;i<nn;i++){
			if(tcheck[i][0]==0){
				double aT =0.; //a*T
				for(int j=0;j<nn;j++){
					if(i!=j){
						aT =aT+a[i][j]*t[j][0];	
					}
				}
				double t_old = t[i][0]; //Previous t[i][0]
				t[i][0] = (1./a[i][i])*(b[i][0]-aT);
				tcon_sum = tcon_sum+(t[i][0]-t_old)*(t[i][0]-t_old);
			}
		}
		tcon = sqrt(tcon_sum/nn);
	}
	
	//Used to decide which triangle has to be refined
	bool* trefine;
	trefine = (bool*)malloc(sizeof(bool)*nel);
	for(int i=0;i<nel;i++){
		trefine[i] =false;
	}
	double* flTx;
	double* flTy;
	flTx = (double*)malloc(nel*sizeof(double));
	flTy = (double*)malloc(nel*sizeof(double));
	double gTxmax,gTxavg,gTymax,gTyavg;
	gTxmax =0.;
	gTxavg =0.;
	gTyavg =0.;
	gTymax =0.;
	for(int i=0;i<nel;i++){
		int type = elms[i][1];
		string name = phyName[type-1];
		string c2(&name[1],&name[5]);
		if(c2.compare("Inte")==0){
			int nd1,nd2,nd3;
			nd1 = elms[i][2];
			nd2 = elms[i][3];
			nd3 = elms[i][4];
			double x1,x2,x3,y1,y2,y3;
			x1 = nodes[nd1-1][0];
			y1 = nodes[nd1-1][1];
			x2 = nodes[nd2-1][0];
			y2 = nodes[nd2-1][1];
			x3 = nodes[nd3-1][0];
			y3 = nodes[nd3-1][1];
			double t1,t2,t3,b1,b2,b3,c1,c2,c3;
			t1 = t[nd1-1][0];
			t2 = t[nd2-1][0];
			t3 = t[nd3-1][0];
			b1 = y2-y3;
			b2 = y3-y1;
			b3 = y1-y2;
			c1 = x3-x2;
			c2 = x1-x3;
			c3 = x2-x1;
			double area = 0.5*(x2*y3-x3*y2-x1*y3+x1*y2+x3*y1-x2*y1);
			double ke,fTx,fTy; //fTx-flux x, fTy-flux y
			ke = phy[type-1][0];
			fTx = (0.5/area)*(b1*t1+b2*t2+b3*t3)*(-ke);
			fTx = fabs(fTx);
			flTx[i] = fTx;
			fTy = (0.5/area)*(c1*t1+c2*t2+c3*t3)*(-ke);
			fTy = fabs(fTy);
			flTy[i] = fTy;
			if(gTxmax<fTx){
				gTxmax = fTx;
			}
			if(gTymax<fTy){
				gTymax = fTy;
			}
			gTxavg = gTxavg+fTx;
			gTyavg = gTyavg+fTy;
		}
	}
	gTxavg = gTxavg/notri;
	gTyavg = gTyavg/notri;
	
	double xmetr,ymetr; //xmetr,ymetr are metrics to refine triangles
	xmetr = 0.5*(gTxavg+gTxmax);
	ymetr = 0.5*(gTyavg+gTymax);
	for(int i=0;i<nel;i++){
		if(elms[i][0]==2){
			if(flTx[i]>xmetr){
				trefine[i] =true;
			}
			if(flTy[i]>ymetr){
				trefine[i] = true;
			}
		}	
	}
	
	//Creating output file
	ofstream outFile;
	outFile.open("SMesh.msh");
	inFile.open("Mesh.msh");
	if (!inFile.is_open()) {
		cout << "File didn't open at 366\n";
		exit(EXIT_FAILURE);
	}
	while(getline(inFile,c)){
		istringstream iss(c);
		string li;
		outFile<<c<<endl;
		iss>>li;
		if(li.compare("$Elements")==0){
			break;
		}
	}
	inFile.close();
	outFile<<nel<<endl;
	for(int i=0;i<nel;i++){
		if(elms[i][0]==1){
			outFile<<i+1<<" "<<elms[i][0]<<" 2 "<<elms[i][1]<<" ";
			outFile<<elms[i][1]+1<<" "<<elms[i][2]<<" ";
			outFile<<elms[i][3]<<endl;
		}
		else{
			outFile<<i+1<<" "<<elms[i][0]<<" 2 "<<elms[i][1]<<" ";
			outFile<<1<<" "<<elms[i][2]<<" "<<elms[i][3]<<" "<<elms[i][4];
			if(trefine[i]){
				outFile<<" true"<<endl;
			}
			else{
				outFile<<" false"<<endl;
			}
		}
	}
	outFile<<"$EndElements"<<endl;
	outFile.close();
}