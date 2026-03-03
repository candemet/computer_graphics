#include "utils/easy_image.h"
#include "utils/ini_configuration.h"
#include "l_parser/l_parser.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <list>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stack>

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

    double imageX = size * (xRange / std::max(xRange, yRange));
    double imageY = size * (yRange / std::max(xRange, yRange));

    // scaling factor d
    double d = 0.95 * imageX / xRange;

    // shift
    double DCx = d * (xMin + xMax) / 2;
    double DCy = d * (yMin + yMax) / 2;

    double dx = (imageX / 2) - DCx;
    double dy = (imageY / 2) - DCy;

    img::EasyImage image(lround(imageX), lround(imageY), color);

    for (auto& line: lines) {
        unsigned int x0 = lround(line.p1.x * d + dx);
        unsigned int y0 = lround(line.p1.y * d + dy);
        unsigned int x1 = lround(line.p2.x * d + dx);
        unsigned int y1 = lround(line.p2.y * d + dy);
        image.draw_line(x0, y0, x1, y1, line.color);
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
