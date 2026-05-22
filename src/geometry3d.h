//
// Created by Can Demet on 04/04/2026.
//

#ifndef ENGINE_GEOMETRY3D_H
#define ENGINE_GEOMETRY3D_H

#include "utils/easy_image.h"
#include "utils/vector3d.h"
#include "zbuffer.h"

#include <vector>
#include <string>
#include <memory>


struct UV {
    double u = 0.0, v = 0.0;
};

struct ColorD {
    double red, green, blue;
    ColorD() : red(0), green(0), blue(0) {}
    ColorD(double r, double g, double b) : red(r), green(g), blue(b) {}
};

struct Light {
    ColorD ambient;
    ColorD diffuse;
    ColorD specular;

    bool infinity = true;
    Vector3D direction;
    Vector3D location;
    double spotAngle = 90.0;

    // schaduwen
    ZBuffer shadowMask;
    Matrix eye;
    double d = 0.0, dx = 0.0, dy = 0.0;
};

using Lights3D = std::vector<Light>;

struct Face {
    std::vector<int> point_indexes;
};

struct Figure {
    std::vector<Vector3D> points;
    std::vector<Face> faces;
    std::vector<UV> uvs;

    ColorD color;
    ColorD ambientReflection;
    ColorD diffuseReflection;
    ColorD specularReflection;
    double reflectionCoefficient = 0.0;

    std::string texturePath;
    std::shared_ptr<img::EasyImage> texture;

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

Figures3D generateFractal(Figure& fig, const int nrIterations, const double scale);
Figure createBuckyBall();
Figure createMengerSponge(int nrIterations);

Figures3D thickenWireframe(const Figure& wire, double radius, int n, int m);

#endif //ENGINE_GEOMETRY3D_H
