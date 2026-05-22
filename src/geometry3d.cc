//
// Created by Can Demet on 04/04/2026.
// niet meer alles in engine.cc zetten
//
#include "geometry3d.h"
#include <map>
#include <utility>
#include <set>

// snelle forward declaration, zou best in aparte header moeten TODO
Matrix scaleFigure(double scale);
Matrix translate(const Vector3D& vector);
void applyTransformation(Figure& figure, const Matrix& m);
Matrix rotateY(double angle);
Matrix rotateZ(double angle);



Figure createCube() {
    Figure cube;
    cube.points = {
        Vector3D::point( 1, -1, -1),
        Vector3D::point(-1,  1, -1),
        Vector3D::point( 1,  1,  1),
        Vector3D::point(-1, -1,  1),
        Vector3D::point( 1,  1, -1),
        Vector3D::point(-1, -1, -1),
        Vector3D::point( 1, -1,  1),
        Vector3D::point(-1,  1,  1)
    };


    cube.faces = {
        Face{{0, 4, 2, 6}},
        Face{{4, 1, 7, 2}},
        Face{{1, 5, 3, 7}},
        Face{{5, 0, 6, 3}},
        Face{{6, 2, 7, 3}},
        Face{{0, 5, 1, 4}}
    };
    return cube;
}

Figure createTetrahedron() {
    Figure tetra;

    tetra.points = {
        Vector3D::point(1, -1, -1),
        Vector3D::point(-1, 1, -1),
        Vector3D::point(1, 1, 1),
        Vector3D::point(-1, -1, 1)
    };

    tetra.faces = {
        Face{{0, 1, 2}},
        Face{{1, 3, 2}},
        Face{{0, 3, 1}},
        Face{{0, 2, 3}}
    };
    return tetra;
}

Figure createOctahedron() {
    Figure octa;
    octa.points = {
        Vector3D::point(1, 0,   0),
        Vector3D::point(-1, 0,  0),
        Vector3D::point(0, -1, 0),
        Vector3D::point(0, 1,  0),
        Vector3D::point(0, 0,   1),
        Vector3D::point(0, 0,   -1)
    };

    octa.faces = {
        Face{{0, 3, 4}},
        Face{{3, 1, 4}},
        Face{{1, 2, 4}},
        Face{{2, 0, 4}},
        Face{{3, 0, 5}},
        Face{{1, 3, 5}},
        Face{{2, 1, 5}},
        Face{{0, 2, 5}},
    };

    return octa;
}

Figure createIcosahedron() {
    Figure Icosahedron;

    const double PI = std::acos(-1.0);

    Icosahedron.points = {
        Vector3D::point(0, 0, std::sqrt(5.0) / 2.0),
        Vector3D::point(std::cos(0 * 2 * PI / 5), std::sin(0 * 2 * PI / 5), 0.5),
        Vector3D::point(std::cos(1 * 2 * PI / 5), std::sin(1 * 2 * PI / 5), 0.5),
        Vector3D::point(std::cos(2 * 2 * PI / 5), std::sin(2 * 2 * PI / 5), 0.5),
        Vector3D::point(std::cos(3 * 2 * PI / 5), std::sin(3 * 2 * PI / 5), 0.5),
        Vector3D::point(std::cos(4 * 2 * PI / 5), std::sin(4 * 2 * PI / 5), 0.5),
        Vector3D::point(std::cos(PI / 5 + 0 * 2 * PI / 5), std::sin(PI / 5 + 0 * 2 * PI / 5), -0.5),
        Vector3D::point(std::cos(PI / 5 + 1 * 2 * PI / 5), std::sin(PI / 5 + 1 * 2 * PI / 5), -0.5),
        Vector3D::point(std::cos(PI / 5 + 2 * 2 * PI / 5), std::sin(PI / 5 + 2 * 2 * PI / 5), -0.5),
        Vector3D::point(std::cos(PI / 5 + 3 * 2 * PI / 5), std::sin(PI / 5 + 3 * 2 * PI / 5), -0.5),
        Vector3D::point(std::cos(PI / 5 + 4 * 2 * PI / 5), std::sin(PI / 5 + 4 * 2 * PI / 5), -0.5),
        Vector3D::point(0, 0, -std::sqrt(5.0) / 2.0)
    };

    Icosahedron.faces = {
        Face{{0, 1, 2}},
        Face{{0, 2, 3}},
        Face{{0, 3, 4}},
        Face{{0, 4, 5}},
        Face{{0, 5, 1}},
        Face{{1, 6, 2}},
        Face{{2, 6, 7}},
        Face{{2, 7, 3}},
        Face{{3, 7, 8}},
        Face{{3, 8, 4}},
        Face{{4, 8, 9}},
        Face{{4, 9, 5}},
        Face{{5, 9, 10}},
        Face{{5, 10, 1}},
        Face{{1, 10, 6}},
        Face{{11, 7, 6}},
        Face{{11, 8, 7}},
        Face{{11, 9, 8}},
        Face{{11, 10, 9}},
        Face{{11, 6, 10}}
    };

    return Icosahedron;
}

Figure createDodecahedron() {
    Figure dodeca;

    Figure icosa = createIcosahedron();

    for (auto face : icosa.faces) {
        Vector3D point;

        point.x = (icosa.points[face.point_indexes[0]].x + icosa.points[face.point_indexes[1]].x + icosa.points[face.point_indexes[2]].x) / 3.0;
        point.y = (icosa.points[face.point_indexes[0]].y + icosa.points[face.point_indexes[1]].y + icosa.points[face.point_indexes[2]].y) / 3.0;
        point.z = (icosa.points[face.point_indexes[0]].z + icosa.points[face.point_indexes[1]].z + icosa.points[face.point_indexes[2]].z) / 3.0;

        dodeca.points.push_back(point);
    }

    dodeca.faces = {
        Face{{0, 1, 2, 3, 4}},
        Face{{0, 5, 6, 7, 1}},
        Face{{1, 7, 8, 9, 2}},
        Face{{2, 9, 10, 11, 3}},
        Face{{3, 11, 12, 13, 4}},
        Face{{4, 13, 14, 5, 0}},
        Face{{19, 18, 17, 16, 15}},
        Face{{19, 14, 13, 12, 18}},
        Face{{18, 12, 11, 10, 17}},
        Face{{17, 10, 9, 8, 16}},
        Face{{16, 8, 7, 6, 15}},
        Face{{15, 6, 5, 14, 19}}
    };

    return dodeca;
}

const double PI = std::acos(-1.0);

Figure createCone(int n, double height) {
    Figure cone;

    // minstens 3
    if (n < 3) {
        return cone;
    }

    //const double PI = std::acos(-1.0);

    // Basis
    for (int i = 0; i < n; i++) {
        double angle = 2.0 * PI * i / n;
        Vector3D point = Vector3D::point(std::cos(angle), std::sin(angle), 0);
        cone.points.push_back(point);
    }

    // Top
    Vector3D top = Vector3D::point(0, 0, height);
    cone.points.push_back(top);

    // Zijkant
    for (int i = 0; i < n; i++) {
        std::vector<int> triangleFace;
        triangleFace.push_back(i);
        triangleFace.push_back((i + 1) % n);
        triangleFace.push_back(n);
        cone.faces.push_back(Face{triangleFace});
    }

    // Bottom
    std::vector<int> bottomFace;
    for (int i = n - 1; i >= 0; i--) {
        bottomFace.push_back(i);
    }
    cone.faces.push_back(Face{bottomFace});

    return cone;
}

Figure createCylinder(int n, double height) {
    Figure cyl;

    for (int i = 0; i < n; i++) {
        double angle = 2.0 * PI * i / n;
        Vector3D point = Vector3D::point(std::cos(angle), std::sin(angle), 0);
        cyl.points.push_back(point);
    }

    for (int i = 0; i < n; i++) {
        double angle = 2.0 * PI * i / n;
        Vector3D point = Vector3D::point(std::cos(angle), std::sin(angle), height);
        cyl.points.push_back(point);
    }

    std::vector<int> topFace;
    for (int i = n; i < 2 * n; i++) {
        topFace.push_back(i);
    }
    cyl.faces.push_back(Face{topFace});

    std::vector<int> bottomFace;
    for (int i = n - 1; i >= 0; i--) {
        bottomFace.push_back(i);
    }
    cyl.faces.push_back(Face{bottomFace});

    for (int i = 0; i < n; i++) {
        std::vector<int> triangleFace;
        triangleFace.push_back(i);
        triangleFace.push_back((i + 1) % n);
        triangleFace.push_back( n + (i + 1) % n);
        triangleFace.push_back( n + i);
        cyl.faces.push_back(Face{triangleFace});
    }

    return cyl;
}

Figure createSphere(const double radius, const int n) {
    Figure sphere = createIcosahedron();

    for (int i = 0; i < n; ++i) {
        std::vector<Face> newFaces;
        newFaces.reserve(sphere.faces.size() * 4);

        std::map<std::pair<int,int>, int> midCache;
        auto midpoint = [&](int a, int b) {
            auto key = std::minmax(a, b);
            if (auto it = midCache.find(key); it != midCache.end()) return it->second;
            int idx = (int) sphere.points.size();
            sphere.points.push_back((sphere.points[a] + sphere.points[b]) * 0.5);
            midCache[key] = idx;
            return idx;
        };

        for (const auto &face : sphere.faces) {
            int i0 = face.point_indexes[0];
            int i1 = face.point_indexes[1];
            int i2 = face.point_indexes[2];
            int m01 = midpoint(i0, i1);
            int m12 = midpoint(i1, i2);
            int m20 = midpoint(i2, i0);
            newFaces.push_back(Face{{i0,  m01, m20}});
            newFaces.push_back(Face{{m01, i1,  m12}});
            newFaces.push_back(Face{{m20, m12, i2 }});
            newFaces.push_back(Face{{m01, m12, m20}});
        }
        sphere.faces = std::move(newFaces);
    }

    for (auto &p : sphere.points) { p.normalise(); p *= radius; }
    return sphere;
}


Figure createTorus(double r, double R, int n, int m) {
    Figure torus;
    if (n < 3) return torus;

    for (int i = 0; i < n; i++) {
        const double u = 2.0 * PI * i / n;
        const double cu = std::cos(u);
        const double su = std::sin(u);

        for (int j = 0; j < m; j++) {
            const double v = 2.0 * PI * j / m;
            const double cv = std::cos(v);
            const double sv = std::sin(v);

            const double x = (r * cv + R) * cu;
            const double y = (r * cv + R) * su;
            const double z = r * sv;

            torus.points.push_back(Vector3D::point(x, y, z));
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx0 = i * m + j;
            int idx1 = ((i + 1) % n) * m + j;
            int idx2 = ((i + 1) % n) * m + ((j + 1) % m);
            int idx3 = i * m + ((j + 1) % m);

            torus.faces.push_back(Face{{idx0, idx1, idx2, idx3}});
        }
    }
    return torus;
}

Figures3D generateFractal(Figure& fig, const int nrIterations, const double scale) {
    Figures3D current;
    current.push_back(fig);

    if (nrIterations <= 0 || scale == 0.0) return current;

    for (int iter = 0; iter < nrIterations; ++iter) {
        Figures3D next;

        for (const Figure& base : current) {
            const int n = static_cast<int>(base.points.size());
            if (n == 0) continue;

            for (int i = 0; i < n; ++i) {
                Figure copy = base;

                // rond origin scalen
                Matrix s = scaleFigure(1.0 / scale);
                applyTransformation(copy, s);

                Vector3D original = base.points[i];
                Vector3D scaled = copy.points[i];
                Vector3D delta = original - scaled;

                Matrix t = translate(delta);
                applyTransformation(copy, t);

                next.push_back(copy);
            }
        }

        current.swap(next);
    }

    return current;
}

// subdivided icosahedron -> sphere (icosphere).
Figure createBuckyBall() {
    const int subdivisions = 1;
    return createSphere(1.0, subdivisions);
}

// append a cube (centered at `center`, with side length `size`) into `target`.
static void appendCubeToFigure(Figure &target, const Vector3D &center, double size) {
    double half = size / 2.0;

    const double cx = center.x;
    const double cy = center.y;
    const double cz = center.z;

    // 8 corners
    std::vector<Vector3D> corners;
    corners.reserve(8);
    corners.push_back(Vector3D::point(cx + half, cy + half, cz + half));
    corners.push_back(Vector3D::point(cx - half, cy + half, cz + half));
    corners.push_back(Vector3D::point(cx - half, cy - half, cz + half));
    corners.push_back(Vector3D::point(cx + half, cy - half, cz + half));
    corners.push_back(Vector3D::point(cx + half, cy + half, cz - half));
    corners.push_back(Vector3D::point(cx - half, cy + half, cz - half));
    corners.push_back(Vector3D::point(cx - half, cy - half, cz - half));
    corners.push_back(Vector3D::point(cx + half, cy - half, cz - half));

    int baseIdx = static_cast<int>(target.points.size());
    for (const auto &p : corners) target.points.push_back(p);

    // faces
    target.faces.push_back(Face{{baseIdx + 0, baseIdx + 1, baseIdx + 2, baseIdx + 3}}); // +Z
    target.faces.push_back(Face{{baseIdx + 4, baseIdx + 7, baseIdx + 6, baseIdx + 5}}); // -Z
    target.faces.push_back(Face{{baseIdx + 0, baseIdx + 3, baseIdx + 7, baseIdx + 4}}); // +X
    target.faces.push_back(Face{{baseIdx + 1, baseIdx + 5, baseIdx + 6, baseIdx + 2}}); // -X
    target.faces.push_back(Face{{baseIdx + 0, baseIdx + 4, baseIdx + 5, baseIdx + 1}}); // +Y
    target.faces.push_back(Face{{baseIdx + 3, baseIdx + 2, baseIdx + 6, baseIdx + 7}}); // -Y
}

static void addMengerRecursive(Figure &acc, const Vector3D &center, double size, int iter) {
    if (iter <= 0) {
        appendCubeToFigure(acc, center, size);
        return;
    }

    double newSize = size / 3.0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            for (int k = -1; k <= 1; ++k) {
                int zeroCount = (i == 0) + (j == 0) + (k == 0);
                if (zeroCount >= 2) continue;
                Vector3D newCenter = Vector3D::point(
                    center.x + i * newSize,
                    center.y + j * newSize,
                    center.z + k * newSize
                );
                addMengerRecursive(acc, newCenter, newSize, iter - 1);
            }
        }
    }
}

Figure createMengerSponge(int nrIterations) {
    Figure sponge;
    addMengerRecursive(sponge, Vector3D::point(0,0,0), 2.0, nrIterations);
    return sponge;
}

// bollen & cylinders
static Matrix orientPlusZTo(const Vector3D& dirIn) {
    Vector3D d = dirIn;
    d.normalise();
    double phi   = std::acos(std::clamp(d.z, -1.0, 1.0));  // polar angle from +z
    double theta = std::atan2(d.y, d.x);

    return rotateY(phi * 180.0 / M_PI) * rotateZ(theta * 180.0 / M_PI);
}

Figures3D thickenWireframe(const Figure &wire, double radius, int n, int m) {
    Figures3D out;

    std::set<std::pair<int, int>> edges;
    for (const auto& f : wire.faces) {
        const auto& idx = f.point_indexes;
        if (idx.size() == 2) {
            edges.insert({std::min(idx[0], idx[1]), std::max(idx[0], idx[1])});
        } else {
            for (size_t i = 0; i < idx.size(); ++i) {
                int a = idx[i];
                int b = idx[(i + 1) %idx.size()];
                edges.insert({std::min(a, b), std::max(a, b)});
            }
        }
    }

    // copy materialen
    auto copyM = [&](Figure& f) {
        f.color                 = wire.color;
        f.ambientReflection     = wire.ambientReflection;
        f.diffuseReflection     = wire.diffuseReflection;
        f.specularReflection    = wire.specularReflection;
        f.reflectionCoefficient = wire.reflectionCoefficient;
    };

    for (auto& e : edges) {
        int a = e.first;
        int b = e.second;

        const Vector3D& pa = wire.points[a];
        const Vector3D& pb = wire.points[b];
        Vector3D dir = pb - pa;
        double len = dir.length();

        if (len < 1e-9) continue;

        Figure cyl = createCylinder(n, len / radius);
        copyM(cyl);

        Matrix M = scaleFigure(radius) * orientPlusZTo(dir) * translate(pa);
        applyTransformation(cyl, M);
        out.push_back(std::move(cyl));
    }

    for (const auto& p : wire.points) {
        Figure sph = createSphere(radius, m);
        copyM(sph);
        applyTransformation(sph, translate(p));
        out.push_back(std::move(sph));
    }

    return out;
}
