/***
Assignment-4: Shading via Illumination and Colors

Name: Davis, Ben

Collaborators: N/A

 Project Summary: In this project, we reuse the scene from assignment 3. This time, nstead of shading with random colors, we use Gouraud shading to color according to the illumination equation:
        Illumination = base_color * (ambient_light + diffuse_light * (n "dot" l)) + specular_light * (n "dot" h)^glossiness
 To do this, normals (n) are generated for every plane that makes up each object in the scene. Then the light source (l) vectors and specular vector (h) calculated. Ambient, diffuse, and specular light, as well as the glossiness factor are all constants that can be tweaked for different shading effects.
 You'll see that objects made up of only one or two base colors look quite dynamic as they are rotated underneath the light source.
***/


#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#pragma GCC diagnostic pop

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>
using namespace std;

int PLANES_IN_A_CUBE = 6;
int POINTS_IN_A_PLANE = 4;
// If a float is < EPSILON or > -EPILSON then it should be 0
float EPSILON = 0.000001;
// Used to determine when to switch directions during scene rotation
int TURNS = 0;
// Theta is the angle to rotate the scene
float THETA = 0.03;

GLfloat AMBIENT = 0.15;
GLfloat DIFFUSE = 0.2;
GLfloat SPECULAR = 0.05;
GLfloat GLOSSINESS = 5;
vector<GLfloat> LIGHT_POS = {-1.5, 5.0, -2.5};
vector<GLfloat> CAMERA_POS = {6.5, 6.5, 6.5};

/**************************************************
 *              Object Model Class                *
 *                                                *
 *  Stores the points of the object as a vector   *
 *  along with the colors and normals for each    *
 *  point. Normals are computed from the points.  *
 *                                                *
 *************************************************/
class ObjectModel {
    vector<GLfloat> _points;
    vector<GLfloat> _normals;
    vector<GLfloat> _base_colors;
    vector<GLfloat> _colors;
public:
    ObjectModel() { };
    vector<GLfloat> get_points() { return _points; };
    vector<GLfloat> get_normals() { return _normals; };
    vector<GLfloat> get_base_colors() { return _base_colors; };
    vector<GLfloat> get_colors() { return _colors; };
    void set_points(vector<GLfloat> points) { _points = points; };
    void set_normals(vector<GLfloat> normals) { _normals = normals; };
    void set_base_colors(vector<GLfloat> base_colors) { _base_colors = base_colors; };
    void set_colors(vector<GLfloat> colors) { _colors = colors; };
};

// The model of the scene
ObjectModel SCENE;

/**************************************************
 *              Utilitie Functions                *
 *************************************************/

// Initializes a square plane of unit lengths
vector<GLfloat> init_plane() {
    vector<GLfloat> vertices = {
        +0.5,   +0.5,   +0.0,
        -0.5,   +0.5,   +0.0,
        -0.5,   -0.5,   +0.0,
        +0.5,   -0.5,   +0.0
    };
    return vertices;
}

// Converts degrees to radians for rotation
float deg2rad(float d) {
    return (d*M_PI) / 180.0;
}

// Converts a vector to an array
GLfloat* vector2array(vector<GLfloat> vec) {
    GLfloat* arr = new GLfloat[vec.size()];
    for (int i = 0; i < vec.size(); i++) {
        arr[i] = vec[i];
    }
    return arr;
}

GLfloat vector_magnitude(GLfloat x, GLfloat y, GLfloat z) {
    return (GLfloat) sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

vector<GLfloat> vector_subtract(vector<GLfloat> A, vector<GLfloat> B) {
    vector<GLfloat> result;
    
    result.push_back(A[0] - B[0]);
    result.push_back(A[1] - B[1]);
    result.push_back(A[2] - B[2]);
    
    return result;
}

vector<GLfloat> vector_add(vector<GLfloat> A, vector<GLfloat> B) {
    vector<GLfloat> result;
    
    result.push_back(A[0] + B[0]);
    result.push_back(A[1] + B[1]);
    result.push_back(A[2] + B[2]);
    
    return result;
}

vector<GLfloat> vector_multiply_scalar(GLfloat s, vector<GLfloat> A) {
    vector<GLfloat> result;
    
    result.push_back(s * A[0]);
    result.push_back(s * A[1]);
    result.push_back(s * A[2]);
    
    return result;
}

vector<GLfloat> unit_vector(vector<GLfloat> a) {
    vector<GLfloat> result;
    
    GLfloat mag = vector_magnitude(a[0], a[1], a[2]);
    
    result.push_back(a[0] / mag);
    result.push_back(a[1] / mag);
    result.push_back(a[2] / mag);
    
    return result;
}

// Converts Cartesian coordinates to homogeneous coordinates
vector<GLfloat> to_homogeneous_coord(vector<GLfloat> cartesian_coords) {
    vector<GLfloat> result;
    for (int i = 0; i < cartesian_coords.size(); i++) {
        result.push_back(cartesian_coords[i]);
        if ((i+1) % 3 == 0) {
            result.push_back(1.0);
        }
    }
    return result;
}

// Converts homogeneous coordinates to Cartesian coordinates
vector<GLfloat> to_cartesian_coord(vector<GLfloat> homogeneous_coords) {
    vector<GLfloat> result;
    for (int i = 0; i < homogeneous_coords.size(); i++) {
        if ((i+1) % 4 == 0) {
            continue;
        } else {
            result.push_back(homogeneous_coords[i]);
        }
    }
    return result;
}

// Definition of a translation matrix
vector<GLfloat> translation_matrix (float dx, float dy, float dz) {
    vector<GLfloat> translate_mat = {
        1.0,    0.0,    0.0,    dx,
        0.0,    1.0,    0.0,    dy,
        0.0,    0.0,    1.0,    dz,
        0.0,    0.0,    0.0,    1.0
    };
    return translate_mat;
}

// Definition of a scaling matrix
vector<GLfloat> scaling_matrix (float sx, float sy, float sz) {
    vector<GLfloat> scale_mat = {
        sx,     0.0,    0.0,    0.0,
        0.0,    sy,     0.0,    0.0,
        0.0,    0.0,    sz,     0.0,
        0.0,    0.0,    0.0,    1.0
    };
    return scale_mat;
}

// Definition of a rotation matrix about the x-axis theta degrees
vector<GLfloat> rotation_matrix_x (float theta) {
    vector<GLfloat> rotate_mat_x = {
        1.0,    0.0,                    0.0,                       0.0,
        0.0,    (float)(cos(theta)),    (float)(-sin(theta)),      0.0,
        0.0,    (float)(sin(theta)),    (float)(cos(theta)),       0.0,
        0.0,    0.0,                    0.0,                       1.0
    };
    return rotate_mat_x;
}


// Definition of a rotation matrix about the y-axis by theta degrees
vector<GLfloat> rotation_matrix_y (float theta) {
    vector<GLfloat> rotate_mat_y = {
        (float)cos(theta),     0.0,     (float)sin(theta),    0.0,
        0.0,                   1.0,     0.0,                  0.0,
        (float)-sin(theta),    0.0,     (float)cos(theta),    0.0,
        0.0,                   0.0,     0.0,                  1.0
    };
    return rotate_mat_y;
}


// Definition of a rotation matrix about the z-axis by theta degrees
vector<GLfloat> rotation_matrix_z (float theta) {
    vector<GLfloat> rotate_mat_z = {
        (float)cos(theta),  (float)-sin(theta), 0.0,    0.0,
        (float)sin(theta),  (float)cos(theta),  0.0,    0.0,
        0.0,                0.0,                1.0,    0.0,
        0.0,                0.0,                0.0,    1.0
    };
    return rotate_mat_z;
}

// Perform matrix multiplication for A B
vector<GLfloat> mat_mult(vector<GLfloat> A, vector<GLfloat> B) {
    vector<GLfloat> result;
    
    for (int b = 0; b < B.size()/4; b++) {
        for (int a = 0; a < 4; a++) {
            float element_wise_sum = 0.0;
            for (int k = 0; k < 4;  k++) {
                float element_wise = A[a*4+k]*B[b*4+k];
                if (element_wise < EPSILON && element_wise > -1.0*EPSILON) {
                    element_wise = 0.0;
                }
                element_wise_sum += element_wise;
            }
            result.push_back(element_wise_sum);
        }
    }
    return result;
}

// Builds a unit cube centered at the origin
vector<GLfloat> build_cube() {
    vector<GLfloat> result;
    // Primitive plane
    vector<GLfloat> a0 = to_homogeneous_coord(init_plane());
    // Construct 6 planes of the cube
    vector<GLfloat> a1 = mat_mult(translation_matrix(0.0,  0.0,  0.5), a0);
    vector<GLfloat> a2 = mat_mult(translation_matrix(0.0,  0.0, -0.5), mat_mult(rotation_matrix_y(deg2rad(180)), a0));
    vector<GLfloat> a3 = mat_mult(translation_matrix(-0.5, 0.0,  0.0), mat_mult(rotation_matrix_y(deg2rad(-90)), a0));
    vector<GLfloat> a4 = mat_mult(translation_matrix(0.5,  0.0,  0.0), mat_mult(rotation_matrix_y(deg2rad(90)), a0));
    vector<GLfloat> a5 = mat_mult(translation_matrix(0.0,  0.5,  0.0), mat_mult(rotation_matrix_x(deg2rad(-90)), a0));
    vector<GLfloat> a6 = mat_mult(translation_matrix(0.0, -0.5,  0.0), mat_mult(rotation_matrix_x(deg2rad(90)), a0));
    
    result.insert(std::end(result), std::begin(a1), std::end(a1));
    result.insert(std::end(result), std::begin(a2), std::end(a2));
    result.insert(std::end(result), std::begin(a3), std::end(a3));
    result.insert(std::end(result), std::begin(a4), std::end(a4));
    result.insert(std::end(result), std::begin(a5), std::end(a5));
    result.insert(std::end(result), std::begin(a6), std::end(a6));
    
    return result;
}


/**************************************************
 *           Generating Surface Normals           *
 *                                                *
 *  Generate the surface normals of the objects   *
 *  using the cross product between two vectors   *
 *  that lie on the surface (plane) of interest.  *
 *  Recall that the direction of the normal to a  *
 *  surface follows the Right Hand Rule.          *
 *                                                *
 *************************************************/

// Performs the cross product between two vectors
vector<GLfloat> cross_product(vector<GLfloat> A, vector<GLfloat> B) {
    vector<GLfloat> C;
    
    // Following the formula for vector cross-product:
    // Cx = Ay*Bz - Az*By
    // Cy = Az*Bx - Ax*Bz
    // Cz = Ax*By - Ay*Bx
    C.push_back(A[1]*B[2] - A[2]*B[1]);
    C.push_back(A[2]*B[0] - A[0]*B[2]);
    C.push_back(A[0]*B[1] - A[1]*B[0]);
    
    return C;
}

// Generates the normals to each surface (plane)
vector<GLfloat> generate_normals(vector<GLfloat> points) {
    vector<GLfloat> normals;
    
    for(int i = 0; i < points.size(); i += 12){
        vector<GLfloat> p0 = { points[i], points[i+1], points[i+2] };
        vector<GLfloat> p1 = { points[i+3], points[i+4], points[i+5] };
        vector<GLfloat> p2 = { points[i+6], points[i+7], points[i+8] };
        
        vector<GLfloat> a = vector_subtract(p1, p0);
        vector<GLfloat> b = vector_subtract(p2, p0);
        
        for(int j = 0; j < POINTS_IN_A_PLANE; j++){
            // x-component
            normals.push_back(unit_vector(cross_product(a,b))[0]);
            // y-component
            normals.push_back(unit_vector(cross_product(a,b))[1]);
            // z-component
            normals.push_back(unit_vector(cross_product(a,b))[2]);
        }
    }
    
    return normals;
}


/**************************************************
 *       Shading via Illumination and Color       *
 *                                                *
 *  Generate the set of colors for each face of   *
 *  the planes that compose the objects in the    *
 *  scene. Based on the light source and surface  *
 *  normals, render the colors of the objects by  *
 *  applying the shading equation.                *
 *                                                *
 *************************************************/

// Performs the dot product between two vectors
GLfloat dot_product(vector<GLfloat> A, vector<GLfloat> B) {
    GLfloat result = 0;
    
    if (A.size() != B.size()) {
        cout << "Dot product cannot be computed on vectors of different sizes" << '\n';
    }
    
    // Following the formula for vector dot product:
    // Dot product = Ax*Bx + Ay*By + Az*Bz
    for (int i = 0; i < A.size(); i++) {
        result += A[i] * B[i];
    }
    
    return result;
}

// Initializes the base color of a plane to a single color
vector<GLfloat> init_base_color(GLfloat r, GLfloat g, GLfloat b) {
    vector<GLfloat> base_color = {
        r,   g,   b,
        r,   g,   b,
        r,   g,   b,
        r,   g,   b
    };
    return base_color;
}

// Initializes the base color of a plane by specifying the color of each point
vector<GLfloat> init_base_color(GLfloat r0, GLfloat g0, GLfloat b0, GLfloat r1, GLfloat g1, GLfloat b1,
                                GLfloat r2, GLfloat g2, GLfloat b2, GLfloat r3, GLfloat g3, GLfloat b3) {
    // This enables OpenGL to use interpolation for (Gouraud) shading the plane
    vector<GLfloat> base_color = {
        r0,   g0,   b0,
        r1,   g1,   b1,
        r2,   g2,   b2,
        r3,   g3,   b3
    };
    return base_color;
}

// I changed ambient, diffuse, and specular light to be represented as GLfloats instead of vectors since I wasn't concerned with varying them for R, G, and B
ObjectModel apply_shading(ObjectModel object_model, vector<GLfloat> light_source, vector<GLfloat> camera,
                          GLfloat amb, GLfloat diff, GLfloat spec, GLfloat gloss) {
    vector<GLfloat> colors;
    
    vector<GLfloat> base_colors = object_model.get_base_colors();
    vector<GLfloat> points = object_model.get_points();
    vector<GLfloat> normals = object_model.get_normals();
    
    for(int i = 0; i < points.size(); i += 3){
        vector<GLfloat> point = {points[i], points[i+1], points[i+2]};
        vector<GLfloat> base_color = {base_colors[i], base_colors[i+1], base_colors[i+2]};
        vector<GLfloat> n = {normals[i], normals[i+1], normals[i+2]};
        
        vector<GLfloat> l = vector_subtract(light_source, point);
        // vector h is halfway between the viewing angle and the light source angle
        vector<GLfloat> h = unit_vector(vector_multiply_scalar(0.5, vector_add(camera, light_source)));
        GLfloat n_dot_l = dot_product(n, l);
        GLfloat n_dot_h = dot_product(n, h);
        
        GLfloat r = base_color[0] * (amb + (diff * (n_dot_l))) + pow(spec * (n_dot_h), gloss);

        GLfloat g = base_color[1] * (amb + (diff * (n_dot_l))) + pow(spec * (n_dot_h), gloss);

        GLfloat b = base_color[2] * (amb + (diff * (n_dot_l))) + pow(spec * (n_dot_h), gloss);
        
        colors.push_back(r);
        colors.push_back(g);
        colors.push_back(b);
    }
    
    object_model.set_colors(colors);
    
    return object_model;
}


/**************************************************
 *            Camera and World Modeling           *
 *                                                *
 *  create a scene by applying transformations to *
 *  the objects built from planes and position    *
 *  the camera to view the scene by setting       *
 *  the projection/viewing matrices               *
 *                                                *
 *************************************************/

void setup() {
    // Enable the vertex array functionality
    glEnableClientState(GL_VERTEX_ARRAY);
    // Enable the color array functionality (so we can specify a color for each vertex)
    glEnableClientState(GL_COLOR_ARRAY);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Set up some default base color
    glColor3f(0.5, 0.5, 0.5);
    // Set up white background
    glClearColor(1.0, 1.0, 1.0, 0.0);
}

void init_camera() {
    // Camera parameters
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define a 50 degree field of view, 1:1 aspect ratio, near and far planes at 1 and 20
    gluPerspective(50.0, 1.0, 1.0, 20.0);
    // Position camera at (6.5, 6.5, 6.5), attention at (0, 0, 0), up at (0, 1, 0)
    gluLookAt(6.5, 6.5, 6.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    
}

ObjectModel make_table() {
    ObjectModel tableObj;
    vector<GLfloat> table = build_cube();
    
    // table is just one rectangular prism
    table = to_cartesian_coord(mat_mult(translation_matrix(+0.0, +0.2, +0.0),
                     mat_mult(scaling_matrix(+1.0, +0.4, +1.0), table)));
 
    tableObj.set_points(table);
    
    vector<GLfloat> normals = generate_normals(tableObj.get_points());
    tableObj.set_normals(normals);
    
    vector<GLfloat> colors;
    
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.73, 0.43, 0.25);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    
    tableObj.set_base_colors(colors);
    
    return apply_shading(tableObj, LIGHT_POS, CAMERA_POS, AMBIENT, DIFFUSE, SPECULAR, GLOSSINESS);
}

ObjectModel make_couch1() {
    ObjectModel couch1Obj;
    vector<GLfloat> couch1;
    
    vector<GLfloat> arm_rest1 = build_cube();
    arm_rest1 = to_cartesian_coord(mat_mult(translation_matrix(+1.2, +0.6, +2.0),
                         mat_mult(scaling_matrix(+0.2, +1.2, +1.2), arm_rest1)));
    for (int i = 0; i < arm_rest1.size(); i++) {
        couch1.push_back(arm_rest1[i]);
    }
    
    vector<GLfloat> arm_rest2 = build_cube();
    arm_rest2 = to_cartesian_coord(mat_mult(translation_matrix(-1.2, +0.6, +2.0), mat_mult(scaling_matrix(+0.2, +1.2, +1.2), arm_rest2)));
    for (int j = 0; j < arm_rest2.size(); j++) {
        couch1.push_back(arm_rest2[j]);
    }
    
    vector<GLfloat> back1 = build_cube();
    back1 = to_cartesian_coord(mat_mult(translation_matrix(+0.0, +0.6, +2.55), mat_mult(scaling_matrix(+2.4, +1.2, +0.1), back1)));
    for (int k = 0; k < back1.size(); k++) {
        couch1.push_back(back1[k]);
    }
    
    vector<GLfloat> seat1 = build_cube();
    seat1 = to_cartesian_coord(mat_mult(translation_matrix(+0.0, +0.15, +2.0), mat_mult(scaling_matrix(+2.4, +0.3, +1.1), seat1)));
    for (int l = 0; l < seat1.size(); l++) {
        couch1.push_back(seat1[l]);
    }
    
    couch1Obj.set_points(couch1);
    
    vector<GLfloat> normals = generate_normals(couch1Obj.get_points());
    couch1Obj.set_normals(normals);
    
    vector<GLfloat> colors;
    
    // arm rest 1
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.333, 0.537, 0.878);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    // arm rest 2
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.333, 0.537, 0.878);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    // back 1
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.576, 0.718, 0.949);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    // seat 1
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.745, 0.808, 0.918);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    
    couch1Obj.set_base_colors(colors);
    
    return apply_shading(couch1Obj, LIGHT_POS, CAMERA_POS, AMBIENT, DIFFUSE, SPECULAR, GLOSSINESS);
}

ObjectModel make_couch2() {
    ObjectModel couch2Obj;
    vector<GLfloat> couch2;
    
    vector<GLfloat> arm_rest3 = build_cube();
    arm_rest3 = to_cartesian_coord(mat_mult(translation_matrix(+1.2, +0.6, -2.0), mat_mult(scaling_matrix(+0.2, +1.2, +1.2), arm_rest3)));
    for (int i = 0; i < arm_rest3.size(); i++) {
        couch2.push_back(arm_rest3[i]);
    }
    
    vector<GLfloat> arm_rest4 = build_cube();
    arm_rest4 = to_cartesian_coord(mat_mult(translation_matrix(-1.2, +0.6, -2.0), mat_mult(scaling_matrix(+0.2, +1.2, +1.2), arm_rest4)));
    for (int j = 0; j < arm_rest4.size(); j++) {
        couch2.push_back(arm_rest4[j]);
    }
    
    vector<GLfloat> back2 = build_cube();
    back2 = to_cartesian_coord(mat_mult(translation_matrix(+0.0, +0.6, -2.55), mat_mult(scaling_matrix(+2.4, +1.2, +0.1), back2)));
    for (int k = 0; k < back2.size(); k++) {
        couch2.push_back(back2[k]);
    }
    
    vector<GLfloat> seat2 = build_cube();
    seat2 = to_cartesian_coord(mat_mult(translation_matrix(+0.0, +0.15, -2.0), mat_mult(scaling_matrix(+2.4, +0.3, +1.1), seat2)));
    for (int l = 0; l < seat2.size(); l++) {
        couch2.push_back(seat2[l]);
    }
    
    couch2Obj.set_points(couch2);
    
    vector<GLfloat> normals = generate_normals(couch2Obj.get_points());
    couch2Obj.set_normals(normals);
    
    vector<GLfloat> colors;
    
    // arm rest 3
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.220, 0.957, 0.231);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    // arm rest 4
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.220, 0.957, 0.231);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    // back 2
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.506, 0.937, 0.514);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    // seat 2
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.667, 0.937, 0.671);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    
    couch2Obj.set_base_colors(colors);
    
    return apply_shading(couch2Obj, LIGHT_POS, CAMERA_POS, AMBIENT, DIFFUSE, SPECULAR, GLOSSINESS);
}

ObjectModel make_cabinet() {
    ObjectModel cabinetObj;
    vector<GLfloat> cabinet;
    
    vector<GLfloat> right_panel = build_cube();
    right_panel = to_cartesian_coord(mat_mult(translation_matrix(+1.0, +0.5, +3.25), mat_mult(scaling_matrix(+0.01, +1.0, +1.0), right_panel)));
    for (int i = 0; i < right_panel.size(); i++) {
        cabinet.push_back(right_panel[i]);
    }
    
    vector<GLfloat> left_panel = build_cube();
    left_panel = to_cartesian_coord(mat_mult(translation_matrix(-1.0, +0.5, +3.25), mat_mult(scaling_matrix(+0.01, +1.0, +1.0), left_panel)));
    for (int j = 0; j < left_panel.size(); j++) {
        cabinet.push_back(left_panel[j]);
    }
    
    vector<GLfloat> top_panel = build_cube();
    top_panel = to_cartesian_coord(mat_mult(translation_matrix(+0.0, +1.005, +3.25), mat_mult(scaling_matrix(+2.0, +0.01, +1.0), top_panel)));
    for (int k = 0; k < top_panel.size(); k++) {
        cabinet.push_back(top_panel[k]);
    }
    
    vector<GLfloat> bottom_panel = build_cube();
    bottom_panel = to_cartesian_coord(mat_mult(translation_matrix(+0.0, +0.005, +3.25), mat_mult(scaling_matrix(+2.0, +0.01, +1.0), bottom_panel)));
    for (int l = 0; l < bottom_panel.size(); l++) {
        cabinet.push_back(bottom_panel[l]);
    }
    
    cabinetObj.set_points(cabinet);
    
    vector<GLfloat> normals = generate_normals(cabinetObj.get_points());
    cabinetObj.set_normals(normals);
    
    vector<GLfloat> colors;
    
    // right panel
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.208, 0.208, 0.208);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    // left panel
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.208, 0.208, 0.208);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    // top panel
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.208, 0.208, 0.208);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    // bottom panel
    for(int i = 0; i < PLANES_IN_A_CUBE; i++){
        vector<GLfloat> base_color = init_base_color(0.208, 0.208, 0.208);
        for (int j = 0; j < base_color.size(); j++) {
            colors.push_back(base_color[j]);
        }
    }
    
    cabinetObj.set_base_colors(colors);
    
    return apply_shading(cabinetObj, LIGHT_POS, CAMERA_POS, AMBIENT, DIFFUSE, SPECULAR, GLOSSINESS);
}

// Construct the scene using objects built from cubes/prisms
vector<GLfloat> init_scene() {
    vector<GLfloat> scene;
    
    ObjectModel table = make_table();
    vector<GLfloat> table_points = table.get_points();
    scene.insert(scene.end(), table_points.begin(), table_points.end());
    
    ObjectModel couch1 = make_couch1();
    vector<GLfloat> couch1_points = couch1.get_points();
    scene.insert(scene.end(), couch1_points.begin(), couch1_points.end());
    
    ObjectModel couch2 = make_couch2();
    vector<GLfloat> couch2_points = couch2.get_points();
    scene.insert(scene.end(), couch2_points.begin(), couch2_points.end());
    
    ObjectModel cabinet = make_cabinet();
    vector<GLfloat> cabinet_points = cabinet.get_points();
    scene.insert(scene.end(), cabinet_points.begin(), cabinet_points.end());
    
    return scene;
}

// Construct the color mapping of the scene
vector<GLfloat> init_color() {
    vector<GLfloat> colors;
    
    ObjectModel table = make_table();
    vector<GLfloat> table_colors = table.get_colors();
    colors.insert(colors.end(), table_colors.begin(), table_colors.end());
    
    ObjectModel couch1 = make_couch1();
    vector<GLfloat> couch1_colors = couch1.get_colors();
    colors.insert(colors.end(), couch1_colors.begin(), couch1_colors.end());
    
    ObjectModel couch2 = make_couch2();
    vector<GLfloat> couch2_colors = couch2.get_colors();
    colors.insert(colors.end(), couch2_colors.begin(), couch2_colors.end());
    
    ObjectModel cabinet = make_cabinet();
    vector<GLfloat> cabinet_colors = cabinet.get_colors();
    colors.insert(colors.end(), cabinet_colors.begin(), cabinet_colors.end());
    
    return colors;
}

void display_func() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    SCENE.set_points(to_homogeneous_coord(SCENE.get_points()));
    // Every 600 frames, the scene changes rotation direction
    if (TURNS % 1000 <= 500) {
        SCENE.set_points(to_cartesian_coord(mat_mult(rotation_matrix_y(THETA), SCENE.get_points())));
    } else {
        SCENE.set_points(to_cartesian_coord(mat_mult(rotation_matrix_y(-THETA), SCENE.get_points())));
    }
    TURNS += 1;
    
    // Shading is applied during the creation of each object
    
    GLfloat* scene_vertices = vector2array(SCENE.get_points());
    GLfloat* color_vertices = vector2array(SCENE.get_colors());
    // Pass the scene vertex pointer
    glVertexPointer(3,                // 3 components (x, y, z)
                    GL_FLOAT,         // Vertex type is GL_FLOAT
                    0,                // Start position in referenced memory
                    scene_vertices);  // Pointer to memory location to read from
    
    // Pass the color vertex pointer
    glColorPointer(3,                   // 3 components (r, g, b)
                   GL_FLOAT,            // Vertex type is GL_FLOAT
                   0,                   // Start position in referenced memory
                   color_vertices);     // Pointer to memory location to read from
    
    // Draw quad point planes: each 4 vertices with 3 dimensions
    glDrawArrays(GL_QUADS, 0, (int)SCENE.get_points().size() / 3);
    
    glFlush();			//Finish rendering
    glutSwapBuffers();
    
    // Clean up
    delete scene_vertices;
    delete color_vertices;
}

void idle_func() {
    display_func();
}

int main (int argc, char **argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    // Create a window with rendering context and everything else we need
    glutCreateWindow("Assignment 4");
    
    setup();
    init_camera();
    
    SCENE.set_points(init_scene());
    SCENE.set_colors(init_color());
    
    // Set up our display function
    glutDisplayFunc(display_func);
    glutIdleFunc(idle_func);
    // Render our world
    glutMainLoop();
    
    return 0;
}
