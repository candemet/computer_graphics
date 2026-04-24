#include "l_parser/l_parser.h"
#include "utils/easy_image.h"
#include "utils/ini_configuration.h"
#include "utils/vector3d.h"
#include "geometry3d.h"
#include "zbuffer.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <stack>
#include <stdexcept>
#include <string>

struct Color {
    double red;
    double green;
    double blue;

    Color() : red(0), green(0), blue(0) {}
    Color(double r, double g, double b) : red(r), green(g), blue(b) {}
};

struct Point2D {
    double x;
    double y;
};

struct Line2D {
    Point2D p1;
    Point2D p2;
    img::Color color;
    double z1;
    double z2;
};


typedef std::list<Line2D> Lines2D;

// Input: List of Line2D
// Output: xMin, xMax, yMin, yMax (respectively)
std::list<double> min_max_finder(const Lines2D &lines) {
    std::list<double> values;
    if (lines.empty()) {
        return values;
    }
    double xMin = lines.front().p1.x;
    double xMax = lines.front().p1.x;
    double yMin = lines.front().p1.y;
    double yMax = lines.front().p1.y;

    for (const Line2D &line: lines) {
        xMin = std::min(std::min(xMin, line.p1.x), line.p2.x);
        xMax = std::max(std::max(xMax, line.p1.x), line.p2.x);
        yMin = std::min(std::min(yMin, line.p1.y), line.p2.y);
        yMax = std::max(std::max(yMax, line.p1.y), line.p2.y);
    }
    values.push_back(xMin);
    values.push_back(xMax);
    values.push_back(yMin);
    values.push_back(yMax);
    return values;
}

img::EasyImage draw2DLines(const Lines2D &lines, const int size, const img::Color& color) {
    if (lines.empty()) {
        return img::EasyImage();
    }
    if (size == 0) {
        return img::EasyImage();
    }

    std::list<double> bounds = min_max_finder(lines);
    double xMin = bounds.front();
    bounds.pop_front();
    double xMax = bounds.front();
    bounds.pop_front();
    double yMin = bounds.front();
    bounds.pop_front();
    double yMax = bounds.front();
    bounds.pop_front();

    double xRange = xMax - xMin;
    double yRange = yMax - yMin;

    double maxRange = std::max(xRange, yRange);

    if (maxRange <= 1e-12) {
        return img::EasyImage(1, 1, color);
    }

    double imageX = static_cast<double>(size) * (xRange / maxRange);
    double imageY = static_cast<double>(size) * (yRange / maxRange);

    // scaling factor d
    double d = 0.95 * static_cast<double>(size) / maxRange;

    // shift
    double DCx = d * (xMin + xMax) / 2.0;
    double DCy = d * (yMin + yMax) / 2.0;

    unsigned int width = std::max(1, static_cast<int>(std::lround(imageX)));
    unsigned int height = std::max(1, static_cast<int>(std::lround(imageY)));


    double dx = (static_cast<double>(width) / 2.0) - DCx;
    double dy = (static_cast<double>(height) / 2.0) - DCy;
    // double dx = (imageX / 2.0) - DCx;
    // double dy = (imageY / 2.0) - DCy;

    img::EasyImage image(width, height, color);

    int w = static_cast<int>(width);
    int h = static_cast<int>(height);

    for (auto& line: lines) {
        int x0 = static_cast<int>(std::lround(line.p1.x * d + dx));
        int y0 = static_cast<int>(std::lround(line.p1.y * d + dy));
        int x1 = static_cast<int>(std::lround(line.p2.x * d + dx));
        int y1 = static_cast<int>(std::lround(line.p2.y * d + dy));

        if ((x0 < 0 && x1 < 0) || (x0 >= w && x1 >= w) ||
            (y0 < 0 && y1 < 0) || (y0 >= h && y1 >= h)) {
            continue;
        }

        x0 = std::clamp(x0, 0, w - 1);
        y0 = std::clamp(y0, 0, h - 1);
        x1 = std::clamp(x1, 0, w - 1);
        y1 = std::clamp(y1, 0, h - 1);


        image.draw_line(
        static_cast<unsigned int>(x0),
        static_cast<unsigned int>(y0),
        static_cast<unsigned int>(x1),
        static_cast<unsigned int>(y1),
        line.color
        );
    }
    return image;
}

// helper funtion for generate LSystem2D
void draw_recursive(const LParser::LSystem2D& system, char c, unsigned int depth,
    double& x, double& y, double& angle, double angle_incr, Lines2D& lines, img::Color color,
    std::stack<std::tuple<double, double, double>>& stateStack) {

    if (depth == 0) {
        double newX = x + cos(angle);
        double newY = y + sin(angle);

        if (system.draw(c)) {
            lines.push_back({{x, y}, {newX, newY}, color});
        }
        x = newX;
        y = newY;
    } else {
        for (char ch : system.get_replacement(c)) {
            if (ch == '+') {
                angle += angle_incr;
            }
            else if (ch == '-') {
                angle -= angle_incr;
            }
            else if (ch == '(') {
                stateStack.push(std::make_tuple(x, y, angle));
            }
            else if (ch == ')') {
                x = std::get<0>(stateStack.top());
                y = std::get<1>(stateStack.top());
                angle = std::get<2>(stateStack.top());
                stateStack.pop();
            }
            else {
                draw_recursive(system, ch, depth - 1, x, y, angle, angle_incr, lines, color, stateStack);
            }
        }
    }
}

Lines2D generate_LSystem_2D(LParser::LSystem2D& lSystem, img::Color lineColor) {
    Lines2D lines;

    //std::set<char> alphabet = lSystem.get_alphabet();

    double angle_incr = lSystem.get_angle();
    double current_angle = lSystem.get_starting_angle();

    unsigned int nrIterations = lSystem.get_nr_iterations();

    // draw(char c) returns boolean
    // get_replacement(char c) returns string
    double x = 0, y = 0;

    double angle_rad = angle_incr * M_PI / 180.0;
    double current_angle_rad = current_angle * M_PI / 180.0;

    std::stack<std::tuple<double, double, double>> stateStack;

    for (char c: lSystem.get_initiator()) {
        if (c == '+') {
            current_angle_rad += angle_rad;
        }
        else if (c == '-') {
            current_angle_rad -= angle_rad;
        }
        else if (c == '(') {
            stateStack.push(std::make_tuple(x, y, current_angle_rad));
        }
        else if (c == ')') {
            x = std::get<0>(stateStack.top());
            y = std::get<1>(stateStack.top());
            current_angle_rad = std::get<2>(stateStack.top());
            stateStack.pop();
        }
        else {
            draw_recursive(lSystem, c, nrIterations, x, y, current_angle_rad, angle_rad, lines, lineColor, stateStack);
        }
    }
    return lines;
}
/// Session 2

// struct Face {
//     std::vector<int> point_indexes;
// };
//
// struct Figure {
//     std::vector<Vector3D> points;
//     std::vector<Face> faces;
//     img::Color color;
// };
//
// using Figures3D = std::vector<Figure>;

Matrix scaleFigure(double scale) {
    Matrix m;
    m(1, 1) = scale;
    m(2, 2) = scale;
    m(3, 3) = scale;
    return m;
}

Matrix rotateX(double angle) {
    const double a = angle * M_PI / 180.0;
    const double c = std::cos(a);
    const double s = std::sin(a);

    Matrix m;
    m(2,2) = c;
    m(2,3) = s;
    m(3,2) = -s;
    m(3,3) = c;
    return m;
}

Matrix rotateY(const double angle) {
    const double a = angle * M_PI / 180.0;
    const double c = std::cos(a);
    const double s = std::sin(a);

    Matrix m;
    m(1,1) = c;
    m(1,3) = -s;
    m(3,1) = s;
    m(3,3) = c;
    return m;
};

Matrix rotateZ(const double angle) {
    const double a = angle * M_PI / 180.0;
    const double c = std::cos(a);
    const double s = std::sin(a);

    Matrix m;
    m(1,1) = c;
    m(1,2) = s;
    m(2,1) = -s;
    m(2,2) = c;
    return m;
}

Matrix translate(const Vector3D &vector) {
    Matrix m;
    m(4,1) = vector.x;
    m(4,2) = vector.y;
    m(4,3) = vector.z;
    return m;
}

Matrix figureTransform(const double scale,
                       const double rotX,
                       const double rotY,
                       const double rotZ,
                       const Vector3D &center) {
    // Volgorde: scale -> Rx -> Ry -> Rz -> translate
    return scaleFigure(scale) * rotateX(rotX) * rotateY(rotY) * rotateZ(rotZ) * translate(center);
}

void applyTransformation(Figure &figure, Matrix &matrix) {
    for (Vector3D &point : figure.points) {
        point *= matrix;
    }
}

void applyTransformation(Figures3D &figures, Matrix &matrix) {
    for (Figure &f : figures) {
        applyTransformation(f, matrix);
    }
}

void toPolar(const Vector3D &point, double &theta, double &phi, double &r) {
    r = std::sqrt(point.x * point.x + point.y * point.y + point.z * point.z);
    if (r == 0.0) {
        theta = 0.0;
        phi = 0.0;
        return;
    }
    theta = std::atan2(point.y, point.x);
    phi = std::acos(point.z / r);
}

Matrix eyePointTrans(const Vector3D &eyepoint) {
    double theta, phi, r;
    toPolar(eyepoint, theta, phi, r);

    Matrix m;
    m(1,1) = -std::sin(theta);
    m(1,2) = -std::cos(theta) * std::cos(phi);
    m(1,3) = std::cos(theta) * std::sin(phi);

    m(2,1) = std::cos(theta);
    m(2,2) = -std::sin(theta) * std::cos(phi);
    m(2,3) = std::sin(theta) * std::sin(phi);

    m(3,1) = 0.0;
    m(3,2) = std::sin(phi);
    m(3,3) = std::cos(phi);

    m(4,1) = 0.0;
    m(4,2) = 0.0;
    m(4,3) = -r;
    return m;
}

Point2D doProjection(const Vector3D &point, const double d = 1.0) {
    if (std::abs(point.z) < 1e-9) {
        // delen door 0
        return {0.0, 0.0};
    }

    return {d * point.x / (-point.z), d * point.y / (-point.z)};
}

Lines2D doProjection( const Figures3D &figs) {
    Lines2D lines;

    for (const Figure &fig :figs) {
        for (const Face &face : fig.faces) {
            if (face.point_indexes.size() < 2) {
                continue;
            }
            for (size_t i = 0; i < face.point_indexes.size(); i++) {
                const int a = face.point_indexes[i];
                const int b = face.point_indexes[(i + 1) % face.point_indexes.size()];

                if ( a < 0 || b < 0) {
                    continue;
                }
                if ( a >= static_cast<int>(fig.points.size()) || b >= static_cast<int>(fig.points.size())) {
                    continue;
                }
                const Vector3D &pa = fig.points[a];
                const Vector3D &pb = fig.points[b];

                lines.push_back({doProjection(pa), doProjection(pb), fig.color, pa.z, pb.z});
            }
        }
    }
    return lines;
}

// beter afronden voor kleuren
static int rounder(double v) {
    if (v < 0.0) v = 0.0;
    if (v > 1.0) v = 1.0;
    return static_cast<int>(lround(v * 255.0));
}

// static img::Color to_img_color(const ini::DoubleTuple &rgb01) {
//     return img::Color(lround(rgb01[0] * 255.0), lround(rgb01[1] * 255.0), lround(rgb01[2] * 255.0));
// }
static img::Color to_img_color(const ini::DoubleTuple &rgb01) {
    return img::Color(rounder(rgb01[0]), rounder(rgb01[1]), rounder(rgb01[2]));
}

static Vector3D tuple_to_point3d(const ini::DoubleTuple &tuple) {
    return Vector3D::point(tuple[0], tuple[1], tuple[2]);
}

static Vector3D tuple_to_vector3d(const ini::DoubleTuple &tuple) {
    return Vector3D::vector(tuple[0], tuple[1], tuple[2]);
}

Figure parseLineDrawingFigure(const ini::Configuration &configuration, const std::string &sectionName) {
    Figure figure;
    figure.color = to_img_color(configuration[sectionName]["color"].as_double_tuple_or_die());

    // points
    const int nrPoints = configuration[sectionName]["nrPoints"].as_int_or_die();
    figure.points.reserve(std::max(0, nrPoints));

    for (int i = 0; i < nrPoints; ++i) {
        const std::string key = "point" + std::to_string(i);
        figure.points.push_back(tuple_to_point3d(configuration[sectionName][key].as_double_tuple_or_die()));
    }

    // lines
    const int nrLines = configuration[sectionName]["nrLines"].as_int_or_die();
    figure.faces.reserve(std::max(0, nrLines));
    for (int i = 0; i < nrLines; ++i) {
        const std::string key = "line" + std::to_string(i);
        ini::IntTuple idx = configuration[sectionName][key].as_int_tuple_or_die();
        Face f;
        f.point_indexes = {idx[0], idx[1]};
        figure.faces.push_back(f);
    }

    // transformation
    const double scale = configuration[sectionName]["scale"].as_double_or_die();
    const double rotX = configuration[sectionName]["rotateX"].as_double_or_die();
    const double rotY = configuration[sectionName]["rotateY"].as_double_or_die();
    const double rotZ = configuration[sectionName]["rotateZ"].as_double_or_die();
    const Vector3D centre = tuple_to_vector3d(configuration[sectionName]["center"].as_double_tuple_or_die());

    Matrix m = figureTransform(scale, rotX, rotY, rotZ, centre);
    applyTransformation(figure, m);

    return figure;
}

static std::string expandLSystem3D(const LParser::LSystem3D& system) {
    std::string current = system.get_initiator();
    const auto& alphabet = system.get_alphabet();

    for (unsigned int i = 0; i < system.get_nr_iterations(); ++i) {
        std::string next;
        for (char c : current) {
            if (alphabet.find(c) != alphabet.end()) {
                next += system.get_replacement(c);
            } else {
                next += c;
            }
        }
        current.swap(next);
    }

    return current;
}

struct Turtle3D {
    Vector3D position;
    Vector3D H;
    Vector3D L;
    Vector3D U;
};

// angle moet in rad zijn!
static void rotateU(Turtle3D& turtle, double angle) {
    const double c = std::cos(angle);
    const double s = std::sin(angle);

    const Vector3D oldH = turtle.H;
    const Vector3D oldL = turtle.L;

    turtle.H = oldH * c + oldL * s;
    turtle.L = oldL * c - oldH * s;
}

static void rotateL(Turtle3D& turtle, double angle) {
    const double c = std::cos(angle);
    const double s = std::sin(angle);

    const Vector3D oldH = turtle.H;
    const Vector3D oldU = turtle.U;

    turtle.H = oldH * c + oldU * s;
    turtle.U = oldU * c - oldH * s;
}

static void rotateH(Turtle3D& turtle, double angle) {
    const double c = std::cos(angle);
    const double s = std::sin(angle);

    const Vector3D oldL = turtle.L;
    const Vector3D oldU = turtle.U;

    turtle.L = oldL * c - oldU * s;
    turtle.U = oldL * s + oldU * c;
}


Figure parse3DLSystemFigure(const ini::Configuration &configuration, const std::string &sectionName) {
    Figure figure;
    figure.color = to_img_color(configuration[sectionName]["color"].as_double_tuple_or_die());

    const std::string inputfile = configuration[sectionName]["inputfile"].as_string_or_die();
    std::ifstream lFile(inputfile);

    LParser::LSystem3D system(lFile);
    lFile.close();

    const std::string expanded = expandLSystem3D(system);
    const double angle = system.get_angle() * M_PI / 180.0;

    Turtle3D turtle{
        Vector3D::point(0, 0, 0),
        Vector3D::vector(1, 0, 0),
        Vector3D::vector(0, 1, 0),
        Vector3D::vector(0, 0, 1)
    };

    std::stack<Turtle3D> stack;

    for (char c : expanded) {
        switch (c) {
            case '+':
                rotateU(turtle, angle);
                break;
            case '-':
                rotateU(turtle, -angle);
                break;
            case '^':
                rotateL(turtle, angle);
                break;
            case '&':
                rotateL(turtle, -angle);
                break;
            case '\\':
                rotateH(turtle, angle);
                break;
            case '/':
                rotateH(turtle, -angle);
                break;
            case '|':
                rotateU(turtle, M_PI);
                break;
            case '(':
                stack.push(turtle);
                break;
            case ')':
                if (!stack.empty()) {
                    turtle = stack.top();
                    stack.pop();
                }
                break;
            default:
                if (system.draw(c)) {
                    const Vector3D next = turtle.position + turtle.H;

                    const int idx = static_cast<int>(figure.points.size());
                    figure.points.push_back(turtle.position);
                    figure.points.push_back(next);
                    figure.faces.push_back(Face{{idx, idx + 1}});

                    turtle.position = next;
                } else {
                    turtle.position = turtle.position + turtle.H;
                }
                break;
        }
    }

    const double scale = configuration[sectionName]["scale"].as_double_or_die();
    const double rotX = configuration[sectionName]["rotateX"].as_double_or_die();
    const double rotY = configuration[sectionName]["rotateY"].as_double_or_die();
    const double rotZ = configuration[sectionName]["rotateZ"].as_double_or_die();
    const Vector3D center = tuple_to_vector3d(configuration[sectionName]["center"].as_double_tuple_or_die());

    Matrix matrix = figureTransform(scale, rotX, rotY, rotZ, center);
    applyTransformation(figure, matrix);

    return figure;
}


static Figure parseWireframeFigure(const ini::Configuration& configuration, const std::string& sectionName) {
    const std::string type = configuration[sectionName]["type"].as_string_or_die();

    Figure figure;

    if (type == "LineDrawing") {
        return parseLineDrawingFigure(configuration, sectionName);
    } else if (type == "Cube") {
        figure = createCube();
    } else if (type == "Tetrahedron") {
        figure = createTetrahedron();
    } else if (type == "Octahedron") {
        figure = createOctahedron();
    } else if (type == "Icosahedron") {
        figure = createIcosahedron();
    } else if (type == "Dodecahedron") {
        figure = createDodecahedron();
    } else if (type == "Cone") {
        const int n = configuration[sectionName]["n"].as_int_or_die();
        const double height = configuration[sectionName]["height"].as_double_or_die();
        figure = createCone(n, height);
    } else if (type == "Cylinder") {
        const int n = configuration[sectionName]["n"].as_int_or_die();
        const double height = configuration[sectionName]["height"].as_double_or_die();
        figure = createCylinder(n, height);
    } else if (type == "Sphere") {
        const int n = configuration[sectionName]["n"].as_int_or_die();
        figure = createSphere(1.0, n);
    } else if (type == "Torus") {
        const double R = configuration[sectionName]["R"].as_double_or_die();
        const double r = configuration[sectionName]["r"].as_double_or_die();
        const int n = configuration[sectionName]["n"].as_int_or_die();
        const int m = configuration[sectionName]["m"].as_int_or_die();
        figure = createTorus(r, R, n, m);
    } else if (type == "3DLSystem") {
        return parse3DLSystemFigure(configuration, sectionName);
    } else {
        return figure;
    }

    figure.color = to_img_color(configuration[sectionName]["color"].as_double_tuple_or_die());

    const double scale = configuration[sectionName]["scale"].as_double_or_die();
    const double rotX = configuration[sectionName]["rotateX"].as_double_or_die();
    const double rotY = configuration[sectionName]["rotateY"].as_double_or_die();
    const double rotZ = configuration[sectionName]["rotateZ"].as_double_or_die();
    const Vector3D center = tuple_to_vector3d(configuration[sectionName]["center"].as_double_tuple_or_die());

    Matrix mtx = figureTransform(scale, rotX, rotY, rotZ, center);
    applyTransformation(figure, mtx);

    return figure;
}

Figures3D parseWireframeFigures(const ini::Configuration &configuration) {
    Figures3D figs;

    const int nrFigures = configuration["General"]["nrFigures"].as_int_or_die();
    figs.reserve(std::max(0, nrFigures));

    for (int i = 0; i < nrFigures; ++i) {
        const std::string sectionName = "Figure" + std::to_string(i);
        figs.push_back(parseWireframeFigure(configuration, sectionName));
    }

    return figs;
}


// final
img::EasyImage generate_wireframe_image(const ini::Configuration &configuration) {
    const int size = configuration["General"]["size"].as_int_or_die();
    const img::Color bgColor = to_img_color(configuration["General"]["backgroundcolor"].as_double_tuple_or_die());

    Figures3D figures = parseWireframeFigures(configuration);

    Vector3D eye = tuple_to_point3d(configuration["General"]["eye"].as_double_tuple_or_die());
    Matrix eyeMatrix = eyePointTrans(eye);
    applyTransformation(figures, eyeMatrix);

    Lines2D lines = doProjection(figures);
    return draw2DLines(lines, size, bgColor);
}

/////////SESSION 4
static void drawZBufLine(ZBuffer& zbuf, img::EasyImage& image,
    int x0, int y0, double z0, int x1, int y1, double z1, const img::Color &color) {

    if ((x0 > x1) || ((x0 == x1) && (y0 > y1))) {
        std::swap(x0, x1);
        std::swap(y0, y1);
        std::swap(z0, z1);
    }

    auto try_plot = [&](int x, int y, double t) {
        if (x < 0 || y < 0) return;
        if (x >= static_cast<int>(image.get_width()) || y >= static_cast<int>(image.get_height())) return;

        const double invz = (1.0 - t) * (1.0 / z0) + t * (1.0 / z1);
        if (invz < zbuf[x][y]) {
            zbuf[x][y] = invz;
            image(static_cast<unsigned int>(x), static_cast<unsigned int>(y)) = color;
        }
    };

    if (x0 == x1) {
        const int dy = y1 - y0;
        const int steps = std::abs(dy);

        if (steps == 0) {
            try_plot(x0, y0, 0.0);
            return;
        }
        const int sign = (dy > 0) ? 1 : -1;
        for (int i = 0; i <= steps; i++) {
            const double t = static_cast<double>(i) / steps;
            try_plot(x0, y0 + i * sign, t);
        }
    }
    else if (y0 == y1) {
        const int dx = x1 - x0;
        const int steps = std::abs(dx);
        for (int i = 0; i <= steps; i++) {
            const double t  = (steps == 0) ? 0.0 : static_cast<double>(i) / steps;
            try_plot(x0 + i, y0, t);
        }
    }
    else {
        const double m = static_cast<double>(y1 - y0) / static_cast<double>(x1 - x0);

        if (-1.0 <= m && m <= 1.0) {
            const int steps = x1 - x0;
            for (int i = 0; i <= steps; i++) {
                const int x = x0 +i;
                const int y = static_cast<int>(std::lround(y0 + m * i));
                const double t = static_cast<double>(i) / steps;
                try_plot(x, y, t);
            }
        }
        else if (m > 1.0) {
            const int steps = y1 - y0;
            for (int i = 0; i <= steps; i++) {
                const int y = y0 + i;
                const int x = static_cast<int>(std::lround(x0 + (i/m)));
                const double t = static_cast<double>(i) / steps;
                try_plot(x, y, t);
            }
        }
        else {
            // m < -1
            const int steps = y0 - y1;
            for (int i = 0; i <= steps; i++) {
                const int y = y0 - i;
                const int x = static_cast<int>(std::lround(x0 - (i/m)));
                const double t = static_cast<double>(i) / steps;
                try_plot(x, y, t);
            }
        }
    }
}

// copy van draw2DLines() maar voor buff
// TODO: herschrijven zodat DRY
img::EasyImage draw2DLinesZBuffered(const Lines2D &lines, const int size, const img::Color& color) {
    if (lines.empty()) {
        return img::EasyImage();
    }
    if (size == 0) {
        return img::EasyImage();
    }

    std::list<double> bounds = min_max_finder(lines);
    double xMin = bounds.front();
    bounds.pop_front();
    double xMax = bounds.front();
    bounds.pop_front();
    double yMin = bounds.front();
    bounds.pop_front();
    double yMax = bounds.front();
    bounds.pop_front();

    double xRange = xMax - xMin;
    double yRange = yMax - yMin;

    double maxRange = std::max(xRange, yRange);

    if (maxRange <= 1e-12) {
        return img::EasyImage(1, 1, color);
    }

    double imageX = static_cast<double>(size) * (xRange / maxRange);
    double imageY = static_cast<double>(size) * (yRange / maxRange);

    // scaling factor d
    double d = 0.95 * static_cast<double>(size) / maxRange;

    // shift
    double DCx = d * (xMin + xMax) / 2.0;
    double DCy = d * (yMin + yMax) / 2.0;

    unsigned int width = std::max(1, static_cast<int>(std::lround(imageX)));
    unsigned int height = std::max(1, static_cast<int>(std::lround(imageY)));


    double dx = (static_cast<double>(width) / 2.0) - DCx;
    double dy = (static_cast<double>(height) / 2.0) - DCy;
    // double dx = (imageX / 2.0) - DCx;
    // double dy = (imageY / 2.0) - DCy;

    img::EasyImage image(width, height, color);

    int w = static_cast<int>(width);
    int h = static_cast<int>(height);

    ZBuffer zbuff(w, h);

    for (auto& line: lines) {
        int x0 = static_cast<int>(std::lround(line.p1.x * d + dx));
        int y0 = static_cast<int>(std::lround(line.p1.y * d + dy));
        int x1 = static_cast<int>(std::lround(line.p2.x * d + dx));
        int y1 = static_cast<int>(std::lround(line.p2.y * d + dy));

        if ((x0 < 0 && x1 < 0) || (x0 >= w && x1 >= w) ||
            (y0 < 0 && y1 < 0) || (y0 >= h && y1 >= h)) {
            continue;
        }

        x0 = std::clamp(x0, 0, w - 1);
        y0 = std::clamp(y0, 0, h - 1);
        x1 = std::clamp(x1, 0, w - 1);
        y1 = std::clamp(y1, 0, h - 1);


        drawZBufLine(zbuff, image, x0, y0, line.z1, x1, y1, line.z2, line.color);
    }
    return image;
}

// TODO: ook een copy
img::EasyImage generate_zbuffered_wireframe_image(const ini::Configuration &configuration) {
    const int size = configuration["General"]["size"].as_int_or_die();
    const img::Color bgColor = to_img_color(configuration["General"]["backgroundcolor"].as_double_tuple_or_die());

    Figures3D figures = parseWireframeFigures(configuration);

    Vector3D eye = tuple_to_point3d(configuration["General"]["eye"].as_double_tuple_or_die());
    Matrix eyeMatrix = eyePointTrans(eye);
    applyTransformation(figures, eyeMatrix);

    Lines2D lines = doProjection(figures);
    return draw2DLinesZBuffered(lines, size, bgColor);
}

/////////////////////////// Session5

static std::vector<Face> triangulateFace(Face &face) {
    std::vector<Face> result;
    if (face.point_indexes.size() < 3) return result;

    int p0 = face.point_indexes[0];
    for (int i = 1; i + 1 < face.point_indexes.size(); i++) {
        result.push_back(Face{{p0, face.point_indexes[i], face.point_indexes[i + 1]}});
    }
    return result;
}

static Figure triangulateFigure(Figure &fig) {
    Figure result = fig; //kleur
    result.faces.clear();

    for (Face &face: fig.faces) {
        if (face.point_indexes.size() < 3) continue;
        std::vector<Face> triangles = triangulateFace(face);
        result.faces.insert(result.faces.end(), triangles.begin(), triangles.end());
    }
    return result;
}

Figures3D triangulateFigures3D(Figures3D &figures) {
    Figures3D result;
    result.reserve(figures.size());
    for (Figure &fig : figures) {
        result.push_back(triangulateFigure(fig));
    }
    return result;
}

struct Parameters {
    int width, height;
    double d, dx, dy;
};

// draw2DLines inspiratie
Parameters computeProjectionParamters(Figures3D &figs, int size) {
    double xMin = std::numeric_limits<double>::infinity();
    double xMax = -xMin;
    double yMin = xMin;
    double yMax = -xMin;
    bool found = false;
    constexpr double eps = 1e-9;


    for (Figure &fig: figs) {
        for (Face &face : fig.faces) {
            if (face.point_indexes.size() != 3)  continue;

            for (int idx : face.point_indexes) {
                if (idx < 0 || idx >= fig.points.size()) continue;

                Vector3D p3 = fig.points[idx];
                if (p3.z >= -eps) continue;

                Point2D p2 = doProjection(p3, 1.0);
                xMin = std::min(xMin, p2.x);
                xMax = std::max(xMax, p2.x);
                yMin = std::min(yMin, p2.y);
                yMax = std::max(yMax, p2.y);
                found = true;
            }
        }
    }

    Parameters pp{};
    if (!found || size <= 0) return pp;

    double xRange = xMax - xMin;
    double yRange = yMax - yMin;
    double maxRange = std::max(xRange, yRange);
    if (maxRange <= 1e-12) return pp;

    double imageX = static_cast<double>(size) * (xRange / maxRange);
    double imageY = static_cast<double>(size) * (yRange / maxRange);

    pp.width = std::max(1, static_cast<int>(std::ceil(imageX)));
    pp.height = std::max(1, static_cast<int>(std::ceil(imageY)));
    pp.d = 0.95 * static_cast<double>(size) / maxRange;


    double dcx = pp.d * (xMin + xMax) / 2.0;
    double dcy = pp.d * (yMin + yMax) / 2.0;

    pp.dx = (static_cast<double>(pp.width) / 2.0) - dcx;
    pp.dy = (static_cast<double>(pp.height) / 2.0) - dcy;

    return pp;
}

static void draw_zbuf_triag(
    ZBuffer& zBuffer,
    img::EasyImage& image,
    const Vector3D& A,
    const Vector3D& B,
    const Vector3D& C,
    double d,
    double dx,
    double dy,
    const img::Color& color
) {

    constexpr double eps = 1e-9;
    if (A.z >= -eps || B.z >= -eps || C.z >= -eps) return;


    double ax = d * A.x / (-A.z) + dx,  ay = d * A.y / (-A.z) + dy;
    double bx = d * B.x / (-B.z) + dx,  by = d * B.y / (-B.z) + dy;
    double cx = d * C.x / (-C.z) + dx,  cy = d * C.y / (-C.z) + dy;

    const Vector3D w = Vector3D::cross(B - A, C - A);
    const double k = Vector3D::dot(w, A);
    if (std::abs(k) < eps) return;

    const double dzdx = w.x / (-d * k);
    const double dzdy = w.y / (-d * k);
    const double xG = (ax + bx + cx) / 3.0;
    const double yG = (ay + by + cy) / 3.0;
    const double invZG = 1.0 / (3.0 * A.z) + 1.0 / (3.0 * B.z) + 1.0 / (3.0 * C.z);

    const int minX = std::max(0, (int)std::floor(std::min({ax, bx, cx})));
    const int maxX = std::min((int)image.get_width()  - 1,(int)std::ceil (std::max({ax, bx, cx})));
    const int minY = std::max(0, (int)std::floor(std::min({ay, by, cy})));
    const int maxY = std::min((int)image.get_height() - 1,(int)std::ceil (std::max({ay, by, cy})));

    if (minX > maxX || minY > maxY) return;

    const double area = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    if (std::abs(area) < eps) return;
    const double invArea = 1.0 / area;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            const double alpha = ((bx - x) * (cy - y) - (by - y) * (cx - x)) * invArea;
            const double beta  = ((cx - x) * (ay - y) - (cy - y) * (ax - x)) * invArea;
            const double gamma = 1.0 - alpha - beta;

            const bool insidePos = (alpha >= 0.0 && beta >= 0.0 && gamma >= 0.0);
            const bool insideNeg = (alpha <= 0.0 && beta <= 0.0 && gamma <= 0.0);
            if (!(insidePos || insideNeg)) continue;

            double invZ = 1.0001 * invZG + (x - xG) * dzdx + (y - yG) * dzdy;
            if (invZ < zBuffer[x][y]) {
                zBuffer[x][y] = invZ;
                image(static_cast<unsigned int>(x), static_cast<unsigned int>(y)) = color;
            }
        }
    }
}

img::EasyImage generate_zbuffering_image(const ini::Configuration &configuration) {
    int size = configuration["General"]["size"].as_int_or_die();
    img::Color bgColor = to_img_color(configuration["General"]["backgroundcolor"].as_double_tuple_or_die());

    Figures3D figures = parseWireframeFigures(configuration);

    Vector3D eye = tuple_to_point3d(configuration["General"]["eye"].as_double_tuple_or_die());
    Matrix eyeMatrix = eyePointTrans(eye);
    applyTransformation(figures, eyeMatrix);

    Figures3D triFigures = triangulateFigures3D(figures);

    Parameters pp = computeProjectionParamters(triFigures, size);
    img::EasyImage image(pp.width, pp.height, bgColor);
    ZBuffer zbuf(pp.width, pp.height);


    for (Figure &fig : triFigures) {
        for (Face &face : fig.faces) {
            if (face.point_indexes.size() != 3) continue;

            int i0 = face.point_indexes[0];
            int i1 = face.point_indexes[1];
            int i2 = face.point_indexes[2];
            if (i0 < 0 || i1 < 0 || i2 < 0) continue;
            if (i0 >= static_cast<int>(fig.points.size()) ||
                i1 >= static_cast<int>(fig.points.size()) ||
                i2 >= static_cast<int>(fig.points.size())) continue;

            const Vector3D &A = fig.points[i0];
            const Vector3D &B = fig.points[i1];
            const Vector3D &C = fig.points[i2];

            const Vector3D n = Vector3D::cross(B - A, C - A);
            if (Vector3D::dot(n, A) >= 0.0) continue;


            draw_zbuf_triag(zbuf, image, A, B, C, pp.d, pp.dx, pp.dy, fig.color);
        }
    }

    return image;
}

//////////////////////////////////////////////////////////////////////////////
img::EasyImage generate_color_rectangle(unsigned int width, unsigned int height)
{
    img::EasyImage image(width, height);
    for(unsigned int i = 0; i < width; i++)
    {
        for(unsigned int j = 0; j < height; j++)
        {
            image(i, j).red = i;
            image(i, j).green = j;
            image(i, j).blue = (i + j) % 256;
        }
    }
    return image;
}

img::EasyImage generate_image(const ini::Configuration &configuration)
{
	std::string type = configuration["General"]["type"].as_string_or_die();

	if (type == "IntroColorRectangle")
	{
		int width = configuration["ImageProperties"]["width"].as_int_or_die();
		int height = configuration["ImageProperties"]["height"].as_int_or_die();
		return generate_color_rectangle(width, height);
	}
    else if (type == "2DLSystem")
    {
        int size = configuration["General"]["size"].as_int_or_die();
        ini::DoubleTuple bg_vec = configuration["General"]["backgroundcolor"].as_double_tuple_or_die();
        img::Color bgColor(lround(bg_vec[0] * 255), lround(bg_vec[1] * 255), lround(bg_vec[2] * 255));

        std::string inputfile = configuration["2DLSystem"]["inputfile"].as_string_or_die();
        ini::DoubleTuple col_vec = configuration["2DLSystem"]["color"].as_double_tuple_or_die();
        img::Color lineColor(lround(col_vec[0] * 255), lround(col_vec[1] * 255), lround(col_vec[2] * 255));

        std::ifstream lFile(inputfile);
        if (!lFile.is_open()) {
            // relative path problemen
            std::vector<std::string> prefixes = {"l_systems/", "lsystem2D/", "../l_systems/", "../lsystem2D/"};
            for (const auto& prefix : prefixes) {
                lFile.open(prefix + inputfile);
                if (lFile.is_open()) break;
            }
        }
        if (!lFile.is_open()) {
            std::cerr << "could not open: " << inputfile << std::endl;
            return img::EasyImage();
        }

        LParser::LSystem2D lSystem(lFile);
        lFile.close();

        Lines2D lines = generate_LSystem_2D(lSystem, lineColor);
        return draw2DLines(lines, size, bgColor);
    }
    else if (type == "Wireframe") {
        return generate_wireframe_image(configuration);
    }
    else if (type == "ZBufferedWireframe") {
        return generate_zbuffered_wireframe_image(configuration);
    }
    else if (type == "ZBuffering") {
        return generate_zbuffering_image(configuration);
    }

	return img::EasyImage();
}

int main(int argc, char const* argv[])
{
        int retVal = 0;
        try
        {
                std::vector<std::string> args = std::vector<std::string>(argv+1, argv+argc);
                if (args.empty()) {
                        std::ifstream fileIn("filelist");
                        std::string filelistName;
                        while (std::getline(fileIn, filelistName)) {
                                args.push_back(filelistName);
                        }
                }
                for(std::string fileName : args)
                {
                        ini::Configuration conf;
                        try
                        {
                                std::ifstream fin(fileName);
                                if (fin.peek() == std::istream::traits_type::eof()) {
                                    std::cout << "Ini file appears empty. Does '" <<
                                    fileName << "' exist?" << std::endl;
                                    continue;
                                }
                                fin >> conf;
                                fin.close();
                        }
                        catch(ini::ParseException& ex)
                        {
                                std::cerr << "Error parsing file: " << fileName << ": " << ex.what() << std::endl;
                                retVal = 1;
                                continue;
                        }

                        img::EasyImage image = generate_image(conf);
                        if(image.get_height() > 0 && image.get_width() > 0)
                        {
                                std::string::size_type pos = fileName.rfind('.');
                                if(pos == std::string::npos)
                                {
                                        //filename does not contain a '.' --> append a '.bmp' suffix
                                        fileName += ".bmp";
                                }
                                else
                                {
                                        fileName = fileName.substr(0,pos) + ".bmp";
                                }
                                try
                                {
                                        std::ofstream f_out(fileName.c_str(),std::ios::trunc | std::ios::out | std::ios::binary);
                                        f_out << image;

                                }
                                catch(std::exception& ex)
                                {
                                        std::cerr << "Failed to write image to file: " << ex.what() << std::endl;
                                        retVal = 1;
                                }
                        }
                        else
                        {
                                std::cout << "Could not generate image for " << fileName << std::endl;
                        }
                }
        }
        catch(const std::bad_alloc &exception)
        {
    		//When you run out of memory this exception is thrown. When this happens the return value of the program MUST be '100'.
    		//Basically this return value tells our automated test scripts to run your engine on a pc with more memory.
    		//(Unless of course you are already consuming the maximum allowed amount of memory)
    		//If your engine does NOT adhere to this requirement you risk losing points because then our scripts will
		//mark the test as failed while in reality it just needed a bit more memory
                std::cerr << "Error: insufficient memory" << std::endl;
                retVal = 100;
        }
        return retVal;
}
