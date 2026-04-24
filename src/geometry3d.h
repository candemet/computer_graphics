//
// Created by Can Demet on 04/04/2026.
//

#ifndef ENGINE_GEOMETRY3D_H
#define ENGINE_GEOMETRY3D_H

#include "utils/easy_image.h"
#include "utils/vector3d.h"

#include <vector>

struct Face {
    std::vector<int> point_indexes;
};

struct Figure {
    std::vector<Vector3D> points;
    std::vector<Face> faces;
    img::Color color;
};

using Figures3D = std::vector<Figure>;

Figure createCube();
Figure createTetrahedron();
Figure createOctahedron();
Figure createIcosahedron();
Figure createDodecahedron();
Figure createCone(int n, double height);
Figure createCylinder(int n, double height);
Figure createSphere(const double r, const int n);
Figure createTorus(double r, double R, int n, int m);


#endif //ENGINE_GEOMETRY3D_H
