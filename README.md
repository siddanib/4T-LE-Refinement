# 4T-LE-Refinement
Refines a given 2D Triangular Mesh using the 4T LE Refinement method (Plaza, A., Suárez, J.P. & Padrón, M.A. Engineering with Computers (2005) 20: 323. https://doi.org/10.1007/s00366-004-0301-7).
The file contains 3 classes of types Vertex, Edge and Triangle.


SerialRefinement_without_Solver.cpp and MPI_Refinement_without_Solver.cpp read the Initial Mesh from "Mesh.msh" file created using GMSH (http://gmsh.info/) version (gmsh-3.0.6-Windows64). The entire domain is refined. Desired triangles which are to be refined can be mentioned near lines 436 and 488 respectively in the codes. The output of these codes is "NewMesh.msh" which can be read by GMSH.


Serial_Refinement_with_Solver.cpp and MPI_Refinement_with_Solver.cpp read the Initial Mesh from "SMesh.msh", which contains "true" beside every triangle that has to be refined. This "SMesh.msh" file is an output from the solver.cpp. The output of these codes is "SNewMesh.msh" which can be read by GMSH. 
