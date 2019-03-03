/***
 main.cpp
 Assignment-1: Cartoonify

 Name: Davis, Ben

 Collaborators: N/A

 Project Summary: The 'generate_points' function in this file is a rescursive implementation of the Bezier curve algorithm.  Midpoints were calculated using the parameterization method.  In order to do this, a function calculated the coefficients for the proper level of Pascal's Triangle.
 ***/


#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <vector>
#include <iostream>
#include <math.h>
using namespace std;

const int WINDOW_X = 1080;
const int WINDOW_Y = 830;

class Vertex {
    GLfloat x, y;
public:
    Vertex(GLfloat, GLfloat);
    GLfloat get_y() const { return y; };
    GLfloat get_x() const { return x; };

    friend Vertex operator+(const Vertex& v0, const Vertex& v1);
    friend Vertex operator*(GLfloat s, const Vertex& v0);
};

Vertex::Vertex(GLfloat X, GLfloat Y) {
    x = X;
    y = Y;
}

vector<Vertex> generate_points(vector<Vertex>, vector<Vertex>, int, int = 0);
Vertex calculate_point(vector<Vertex>, vector<int>, GLfloat);
void draw_curve(vector<Vertex>);
vector<int> nth_level_of_pascal_triangle(int);
Vertex convert_coordinates(int, int);

Vertex operator+(const Vertex& v0, const Vertex& v1) {
    return Vertex(v0.get_x() + v1.get_x(), v0.get_y() + v1.get_y());
}
                  
Vertex operator*(GLfloat s, const Vertex& v0) {
    return Vertex(s * v0.get_x(), s * v0.get_y());
}

void setup() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// Moved n_iter to this function
vector<Vertex> generate_points(vector<Vertex> control_points, vector<Vertex> generated_points, int n_iter, int z) {
    if (z > n_iter) return generated_points;
    else {
        GLfloat t = (float)z / (float)n_iter;
        vector<int> pascal_coefficients = nth_level_of_pascal_triangle((int) control_points.size() - 1);
        Vertex new_point = calculate_point(control_points, pascal_coefficients, t);
        generated_points.push_back(new_point);
        return generate_points(control_points, generated_points, n_iter, z + 1);
    }
}

// Calculates point using the parameterization approach (which needs nth level of pascal's triangle)
Vertex calculate_point(vector<Vertex> control_points, vector<int> pascal_coefficients, GLfloat t) {
    Vertex calculated_point = Vertex(0,0);
    int c = (int) control_points.size() - 1;
    for (int m = 0; m <= c; m++) {
        GLfloat constant = (GLfloat) pow(1-t, c-m) * (GLfloat) pow(t, m) * (GLfloat) pascal_coefficients[m];
        calculated_point = calculated_point + (constant * control_points[m]);
    }
    
    return calculated_point;
}

void draw_curve(vector<Vertex> control_points) {
    vector<Vertex> generated_points;
    generated_points = generate_points(control_points, generated_points, 1000);

    glBegin(GL_LINES);
        for (int i = 0; i < generated_points.size() - 1; i++) {
            glVertex2f(generated_points[i].get_x(), generated_points[i].get_y());
            glVertex2f(generated_points[i+1].get_x(), generated_points[i+1].get_y());
        }
    glEnd();
}

// This function is based on the approach outlined here: https://math.stackexchange.com/questions/1154955/is-there-an-equation-that-represents-the-nth-row-in-pascals-triangle
vector<int> nth_level_of_pascal_triangle(int n) {
    vector<int> pascal_coefficients;
    pascal_coefficients.push_back(1);
    for (int i = 1; (n / i) >= 2; i++) {
        int coeff = 1;
        for (int j = n; j > (n - i); j--) {
            coeff *= ((GLfloat) j / (GLfloat) (n - j + 1));
        }
        pascal_coefficients.push_back((int) coeff);
    }
    
    // Utilizes symmetry to finish off the sequence
    vector<int> temp;
    int first_half = (int) pascal_coefficients.size();
    for (int k = 0; k < first_half; k++) {
        temp.push_back(pascal_coefficients[k]);
    }
    for (int l = first_half - 1; l >= 0; l--) {
        if ((n % 2 == 0) && (l == first_half - 1)) {
            continue;
        } else {
            temp.push_back(pascal_coefficients[l]);
        }
    }
    pascal_coefficients = temp;
    vector<int>().swap(temp); // free the memory associated with temp
    
    return pascal_coefficients;
}

// In order to obtain coordinates from the reference photo, I used https://inkplant.com/tools/image-coordinates.  This function coverts those coordinates into proper GLfloat values, which made inputting values much easier for me.
Vertex convert_coordinates(int x, int y) {
    GLfloat x_conversion_factor = (GLfloat) WINDOW_X / 2.0;
    GLfloat y_conversion_factor = (GLfloat) WINDOW_Y / 2.0;
    
    GLfloat x_component = ((GLfloat) x - x_conversion_factor) / x_conversion_factor;
    GLfloat y_component = (y_conversion_factor - (GLfloat) y) / y_conversion_factor;
    
    return Vertex(x_component, y_component);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set our color to black (R, G, B)
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(3.0f);
    
    vector<vector<vector<int>>> drawings{
        // Cord and plug
        { {395, 215}, {395, 206} },
        { {387, 215}, {387, 206} },
        { {381, 200}, {391, 211}, {402, 200} },
        { {383, 200}, {383, 176}, {400, 176}, {400, 200} },
        { {416, 155}, {414, 491}, {532, 515}, {584, 460} },
        { {391, 181}, {391, 88}, {398, 88}, {405, 78}, {415, 115} },
        { {393, 199}, {399, 196} },
        // Laptop
        { {579, 440}, {574, 371} },
        { {574, 371}, {645, 350} },
        { {645, 350}, {646, 414} },
        { {646, 414}, {579, 440} },
        { {587, 444}, {647, 453} },
        { {600, 456}, {645, 462} },
        { {587, 444}, {573, 452}, {584, 454} },
        { {584, 464}, {589, 467}, {600, 457} },
        { {580, 459}, {574, 458}, {596, 448} },
        { {595, 450}, {603, 448}, {601, 455} },
        { {603, 442}, {648, 421} },
        { {616, 445}, {633, 436} },
        { {646, 454}, {653, 449} },
        { {646, 414}, {668, 420} },
        { {687, 426}, {710, 425}, {686, 438} },
        // Left Hand
        { {685, 442}, { 685, 401 }, {665, 430}, {653, 429} },
        { {685, 442}, {664, 459}, {658, 486}, {617, 464} },
        { {674, 430}, {671, 436}, {675, 438} },
        { {675, 439}, {646, 429}, {648, 434} },
        { {652, 438}, {636, 428}, {635, 438} },
        { {666, 447}, {659, 429}, {647, 445}, {626, 433}, {615, 439}, {639, 451}, {654, 446}, {658, 451} },
        // Right hand
        { {448, 130}, {442, 103} },
        { {442, 103}, {403, 113}, {423, 131} },
        { {423, 131}, {433, 122}, {422, 117} },
        { {432, 104 }, {413, 101} },
        { {407, 102 }, {395, 111}, {405, 119} },
        { {403, 120}, {402, 131} },
        { {402, 131}, {413, 137}, {418, 129} },
        { {403, 120}, {413, 118} },
        { {406, 134}, {401, 141}, {408, 143} },
        { {424, 131}, {430, 134}, {423, 138} },
        { {408, 143}, {420, 136}, {423, 138} },
        { {408, 143}, {404, 159}, {420, 155}, {430, 152}, {423, 138} },
        { {438, 146}, {433, 152}, {426, 148} },
        // Right foot
        { {473, 698}, {475, 716}, {492, 719}, {542, 681} },
        { {534, 680}, {498, 699} },
        { {498, 698}, {477, 706}, {475, 696}, {495, 693} },
        { {489, 688}, {485, 690}, {485, 696} },
        { {484, 688}, {476, 690}, {481, 697} },
        { {479, 697}, {466, 694}, {483, 686} },
        { {481, 684}, {500, 683}, {505, 694} },
        { {485, 679}, {502, 680}, {511, 690} },
        { {516, 672}, {521, 674}, {524, 686} },
        { {500, 675}, {513, 673}, {518, 687} },
        // Left foot
        { {603, 700}, {653, 734}, {669, 717} },
        { {613, 701}, {630, 704}, {641, 717} },
        { {641, 715}, {660, 723}, {664, 709}, {653, 714}, {647, 709} },
        { {656, 711}, {649, 703} },
        { {654, 702}, {665, 706}, {665, 713} },
        { {665, 713}, {672, 706}, {661, 700} },
        { {665, 713}, {660, 714}, {656, 711} },
        { {662, 701}, {644, 699}, {641, 715} },
        { {662, 700}, {640, 696}, {635, 713} },
        { {650, 689}, {632, 692}, {623, 705} },
        { {650, 689}, {661, 700} },
        // Body
        { {615, 460}, {626, 522}, {614, 603}, {639, 668} },
        { {615, 668}, {639, 666}, {651, 678}, {650, 688} },
        { {650, 688}, {602, 711}, {531, 697}, {530, 658}, {490, 675} },
        { {546, 680}, {529, 673}, {517, 650}, {501, 665} },
        { {503, 658}, {531, 651}, {595, 526} },
        { {594, 486}, {500, 578}, {475, 565}, {505, 553} },
        { {490, 675}, {483, 666}, {496, 658} },
        { {496, 658}, {483, 595}, {500, 591}, {506, 585}, {497, 571}, {479, 560}, {493, 542}, },
        { {495, 545}, {482, 499}, {484, 486}, {495, 484}, {496, 473}, {479, 459}, {475, 448}, {478, 438}, {488, 435} },
        { {487, 447}, {515, 375}, {514, 328}, {499, 310} },
        { {492, 427}, {478, 377}, {483, 333}, {470, 281} },
        { {502, 313}, {447, 129} },
        { {473, 280}, {454, 266}, {435, 151} },
        { {502, 300}, {516, 304}, {530, 394}, {560, 446} },
        { {535, 388}, {567, 336}, {562, 322} },
        { {562, 373}, {574, 327}, {568, 320} },
        { {515, 318}, {523, 323}, {528, 316} },
        { {539, 332}, {553, 329} },
        { {527, 324}, {531, 303} },
        { {560, 325}, {573, 269} },
        { {550, 303}, {523, 305}, {517, 293}, {522, 262} },
        { {529, 282}, {538, 279}, {547, 283} },
        { {573, 276}, {578, 283}, {593, 264}, {591, 253}, {583, 254} },
        { {570, 249}, {567, 259}, {556, 258}, {560, 249} },
        { {534, 250}, {522, 255}, {527, 246} },
        { {556, 236}, {576, 235} },
        { {547, 238}, {541, 241}, {533, 228} },
        { {548, 250}, {558, 278}, {537, 277}, {520, 272}, {545, 263}, {546, 250} },
        { {585, 254}, {592, 246}, {591, 230} },
        { {591, 230}, {582, 224} },
        { {582, 224}, {524, 221}, {528, 234} },
        { {610, 359}, {604, 327}, {597, 364} },
        { {564, 308}, {581, 349}, {580, 315}, {603, 345} },
        { {579, 248}, {580, 223} },
        //Glasses
        { {552, 250}, {551, 261}, {563, 269}, {576, 264}, {580, 252} },
        { {552, 250}, {558, 241}, {567, 239}, {575, 241}, {580, 252} },
        { {545, 247}, {523, 233}, {513, 239}, {514, 254} },
        { {514, 254}, {517, 262}, {525, 266}, {538, 260}, {546, 247} },
        // Crown
        { {590, 225}, {594, 214} },
        { {594, 214}, {587, 209} },
        { {572, 205}, {565, 201}, {560, 205} },
        { {541, 205}, {536, 204}, {532, 208} },
        { {524, 234}, {525, 218} },
        { {567, 212}, {580, 215} },
        { {580, 215}, {606, 182} },
        { {566, 214}, {583, 192}, {606, 182} },
        { {562, 211}, {544, 165} },
        { {544, 165}, {542, 212} },
        { {542, 212}, {562, 211} },
        { {537, 212}, {496, 193} },
        { {496, 193}, {526, 218} },
        { {526, 218}, {537, 212} },
        { {525, 220}, {505, 225} },
        { {505, 225}, {525, 224} },
        { {592, 221}, {615, 222} },
        { {615, 222}, {592, 226} },
        // Wall
        { {625, 589}, {676, 571} },
        { {493, 634}, {314, 704} },
        // Outlet
        { {364, 615}, {364, 645} },
        { {364, 615}, {339, 626} },
        { {339, 626}, {338, 657} },
        { {338, 657}, {364, 645} },
        { {349, 632}, {349, 638} },
        { {354, 631}, {354, 637} },
        { {359, 631}, {358, 625}, {350, 623}, {344, 628}, {344, 637} },
        { {344, 637}, {347, 650}, {354, 650}, {361, 639}, {359, 631} }
    };
    
    for (int i = 0; i < drawings.size(); i++) {
        vector<Vertex> curve;
        for (int j = 0; j < drawings[i].size(); j++) {
            curve.push_back(convert_coordinates(drawings[i][j][0], drawings[i][j][1]));
        }
        draw_curve(curve);
    }

    glutSwapBuffers();
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_X, WINDOW_Y);
    glutCreateWindow("Assignment 1");
    setup();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
