# 4T-LE-Refinement
Creates refinement of a given 2D Triangular Mesh using the 4T LE Refinement method (Plaza, A., Suárez, J.P. & Padrón, M.A. Engineering with Computers (2005) 20: 323. https://doi.org/10.1007/s00366-004-0301-7).
The file contains 3 classes of types Vertex, Edge and Triangle.
The code reads the Initial Mesh from ".msh" file created using GMSH (http://gmsh.info/) version (gmsh-3.0.6-Windows64).
The name of the input mesh in the code is "Mesh.msh".
The output is also a ".msh" file which contains the updated mesh after the desired triangles have been refined.
The name of the output mesh is "NewMesh.msh".
