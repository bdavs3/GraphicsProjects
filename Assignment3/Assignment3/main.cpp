/***
 Assignment-3: Geometric Modeling of a Scene
 
 Name: Davis, Ben
 
 Collaborators: N/A
 
 Project Summary: In this project, a unit cube (volume 1), is constructed at the point (0, 0, 0) by using 6 rotated and translated planes.  Several instances of this cube are then scaled and translated according to a reference photo such that it forms a 3D living room scene.  This scene is rotated about the y-axis back and forth, creating the illusion of seeing it from many camera angles (though the camera stays in place). I used 2-D vectors to represent matrices in this program for readability.
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

// THETA is the angle to rotate the scene
float THETA = 0.3;
// when TURNS reaches increments of 500 ( incremented in display_func() ), we start spinning the scene the opposite direction
int TURNS = 0;
// Vector placeholders for the scene and color array
vector<vector<GLfloat>> SCENE;
vector<vector<GLfloat>> COLOR;

/**************************************************
 *  Rectangular Prisms via Hierarchical Modeling  *
 *                                                *
 *  using planes as building blocks, build a unit *
 *  cube with transformations that will serve as  *
 *  a primitive for modeling objects in the scene *
 *                                                *
 *************************************************/

// Initializes a square plane of unit lengths using 4 points
// Written in the form: [ x0, x1, x2, x3,
//                        y0, y1, y2, y3,
//                        z0, z1, z2, z3 ]
vector<vector<GLfloat>> init_plane() {
    vector<vector<GLfloat>> vertices = {
        {+0.5, -0.5, -0.5, +0.5}, // x-components
        {+0.5, +0.5, -0.5, -0.5}, // y-components
        {+0.0, +0.0, +0.0, +0.0}, // z-components
    };
    return vertices;
}

// Converts degrees to radians for rotation
float deg2rad(float d) {
    return (d*M_PI) / 180.0;
}

// Converts a vector to an array
// UNUSED IN MY PROGRAM BECAUSE OF THE CHOICE TO USE 2D-VECTORS (SEE BELOW METHOD)
GLfloat* vector2array(vector<GLfloat> vec) {
    GLfloat* arr = new GLfloat[vec.size()];
    for (int i = 0; i < vec.size(); i++) {
        arr[i] = vec[i];
    }
    return arr;
}

// Converts a 2D-vector to an array.
GLfloat* two_d_vector_to_array(vector<vector<GLfloat>> vec) {
    vector<GLfloat> one_d_vector(vec.size() * vec[0].size(), +0.0);
    
    for (int i = 0; i < vec[0].size(); i++) {
        for (int j = 0; j < vec.size(); j++) {
            one_d_vector[(3*j)+i] = vec[j][i];
        }
    }
    
    GLfloat* arr = new GLfloat[one_d_vector.size()];
    
    for (int i = 0; i < one_d_vector.size(); i++) {
        arr[i] = one_d_vector[i];
    }
    
    return arr;
}

// Pivots a matrix that is written in this form (which is used with my mat_mult, rotation, translation, scaling, to_homogeneous, and to_cartesian methods):
// [ x0, x1, x2, ...,
//   y0, y1, y2, ...,
//   z0, z1, z2, ..., ]
// such that it is now written in this form (:
// [ x0, y0, z0, ...
//   x1, y1, z1, ...
//   x2, y2, z2, ... ]
vector<vector<GLfloat>> pivot_coordinates(vector<vector<GLfloat>> original) {
    vector<vector<GLfloat>> result(original[0].size(), vector<GLfloat>(original.size(), 0.0));
    
    for (int i = 0; i < original.size(); i++) {
        for (int j = 0; j < original[0].size(); j++) {
            result[j][i] = original[i][j];
        }
    }
    
    return result;
}

// Converts Cartesian coordinates to homogeneous coordinates
vector<vector<GLfloat>> to_homogeneous_coord(vector<vector<GLfloat>> cartesian_coords) {
    vector<vector<GLfloat>> result = cartesian_coords;
    
    vector<GLfloat> homogeneous_component(cartesian_coords[0].size(), +1.0);
    
    result.push_back(homogeneous_component);
    
    return result;
}

// Converts Cartesian coordinates to homogeneous coordinates
vector<vector<GLfloat>> to_cartesian_coord(vector<vector<GLfloat>> homogeneous_coords) {
    vector<vector<GLfloat>> result = homogeneous_coords;
    
    result.pop_back();
    
    return result;
}

// Definition of a translation matrix
vector<vector<GLfloat>> translation_matrix (float dx, float dy, float dz) {
    vector<vector<GLfloat>> translate_mat;
    
    translate_mat = {
        {+1.0, +0.0, +0.0, dx},
        {+0.0, +1.0, +0.0, dy},
        {+0.0, +0.0, +1.0, dz},
        {+0.0, +0.0, +0.0, +1.0},
    };
    
    return translate_mat;
}

// Definition of a scaling matrix
vector<vector<GLfloat>> scaling_matrix (float sx, float sy, float sz) {
    vector<vector<GLfloat>> scale_mat;
    
    scale_mat = {
        {sx, +0.0, +0.0, +0.0},
        {+0.0, sy, +0.0, +0.0},
        {+0.0, +0.0, sz, +0.0},
        {+0.0, +0.0, +0.0, +1.0},
    };
    
    return scale_mat;
}

// Definition of a rotation matrix about the x-axis theta degrees
vector<vector<GLfloat>> rotation_matrix_x (float theta) {
    vector<vector<GLfloat>> rotate_mat_x;
    
    theta = deg2rad(theta);
    
    rotate_mat_x = {
        {+1.0, +0.0, +0.0, +0.0},
        {+0.0, cos(theta), -sin(theta), +0.0},
        {+0.0, sin(theta), cos(theta), +0.0},
        {+0.0, +0.0, +0.0, +1.0},
    };
    
    return rotate_mat_x;
}


// Definition of a rotation matrix along the y-axis by theta degrees
vector<vector<GLfloat>> rotation_matrix_y (float theta) {
    vector<vector<GLfloat>> rotate_mat_y;
    
    theta = deg2rad(theta);
    
    rotate_mat_y = {
        {cos(theta), +0.0, -sin(theta), +0.0},
        {+0.0, +1.0, +0.0, +0.0},
        {sin(theta), +0.0, cos(theta), +0.0},
        {+0.0, +0.0, +0.0, +1.0},
    };
    
    return rotate_mat_y;
}


// Definition of a rotation matrix along the z-axis by theta degrees
vector<vector<GLfloat>> rotation_matrix_z (float theta) {
    vector<vector<GLfloat>> rotate_mat_z;
    
    theta = deg2rad(theta);
    
    rotate_mat_z = {
        {cos(theta), -sin(theta), +0.0, +0.0},
        {sin(theta), cos(theta), +0.0, +0.0},
        {+0.0, +0.0, +1.0, +0.0},
        {+0.0, +0.0, +0.0, +1.0},
    };
    
    return rotate_mat_z;
}

// Perform matrix multiplication between matrix A and matrix B
vector<vector<GLfloat>> mat_mult(vector<vector<GLfloat>> A, vector<vector<GLfloat>> B) {
//    INPUT MATRIX "A"
//    cout << "Matrix A:" << '\n';
//    for (vector<GLfloat> v : A) {
//        for (GLfloat f : v) {
//            cout << f << ", ";
//        }
//        cout << '\n';
//    }
//    cout << '\n';

//    INPUT MATRIX "B"
//    cout << "Matrix B:" << '\n';
//    for (vector<GLfloat> v : B) {
//        for (GLfloat f : v) {
//            cout << f << ", ";
//        }
//        cout << '\n';
//    }
//    cout << '\n';
    
    vector<vector<GLfloat>> result(A.size(), vector<GLfloat>(B[0].size(), 0.0));
    
    if (A[0].size() != B.size()) {
        cout << "Invalid matrix multiplication... Matrix sizes " << A.size() << "x" << A[0].size() << " and " << B.size() << "x" << B[0].size() << " are incompatible" << '\n';
    }
    
    for (int i = 0; i < B[0].size(); i++) {
        for (int j = 0; j < A.size(); j++) {
            for (int k = 0; k < A[0].size(); k++) {
                result[j][i] += A[j][k] * B[k][i];
            }
        }
    }
    
//    RESULT OF MATRIX MULTIPLICATION
//    cout << "Result:" << '\n';
//    for (vector<GLfloat> v : result) {
//        for (GLfloat f : v) {
//            cout << f << ", ";
//        }
//        cout << '\n';
//    }
//    cout << '\n';
    
    return result;
}

// Builds a unit cube centered at the origin
vector<vector<GLfloat>> build_cube() {
    vector<vector<GLfloat>> result;
    
    vector<vector<GLfloat>> initial_plane = init_plane();
    initial_plane = to_homogeneous_coord(initial_plane);
    
    vector<vector<GLfloat>> front_plane = mat_mult(translation_matrix(+0.0, +0.0, +0.5), initial_plane);
    front_plane = to_cartesian_coord(front_plane);
    front_plane = pivot_coordinates(front_plane);
    result.insert(result.end(), front_plane.begin(), front_plane.end());
    
    vector<vector<GLfloat>> left_plane = mat_mult(rotation_matrix_y(-90.0), initial_plane);
    left_plane = mat_mult(translation_matrix(-0.5, +0.0, +0.0), left_plane);
    left_plane = to_cartesian_coord(left_plane);
    left_plane = pivot_coordinates(left_plane);
    result.insert(result.end(), left_plane.begin(), left_plane.end());

    vector<vector<GLfloat>> right_plane = mat_mult(rotation_matrix_y(+90.0), initial_plane);
    right_plane = mat_mult(translation_matrix(+0.5, +0.0, +0.0), right_plane);
    right_plane = to_cartesian_coord(right_plane);
    right_plane = pivot_coordinates(right_plane);
    result.insert(result.end(), right_plane.begin(), right_plane.end());

    vector<vector<GLfloat>> back_plane = mat_mult(rotation_matrix_y(+180.0), initial_plane);
    back_plane = mat_mult(translation_matrix(+0.0, +0.0, -0.5), back_plane);
    back_plane = to_cartesian_coord(back_plane);
    back_plane = pivot_coordinates(back_plane);
    result.insert(result.end(), back_plane.begin(), back_plane.end());

    vector<vector<GLfloat>> top_plane = mat_mult(rotation_matrix_x(-90.0), initial_plane);
    top_plane = mat_mult(translation_matrix(+0.0, +0.5, +0.0), top_plane);
    top_plane = to_cartesian_coord(top_plane);
    top_plane = pivot_coordinates(top_plane);
    result.insert(result.end(), top_plane.begin(), top_plane.end());

    vector<vector<GLfloat>> bottom_plane = mat_mult(rotation_matrix_x(+90.0), initial_plane);
    bottom_plane = mat_mult(translation_matrix(+0.0, -0.5, +0.0), bottom_plane);
    bottom_plane = to_cartesian_coord(bottom_plane);
    bottom_plane = pivot_coordinates(bottom_plane);
    result.insert(result.end(), bottom_plane.begin(), bottom_plane.end());
    
//    DISPLAY THE 6 PLANES OF OUR UNIT CUBE
//    int count = 0;
//    cout << "Result" << '\n';
//    for (vector<GLfloat>& v : result) {
//        for (GLfloat& f : v) {
//            cout << f << ", ";
//        }
//        cout << '\n';
//        if (count == 3) {
//            cout << '\n';
//            count = 0;
//        } else {
//            count++;
//        }
//    }
//    cout << '\n';

    return result;
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

// Construct the scene using objects built from cubes/prisms
vector<vector<GLfloat>> init_scene() {
    vector<vector<GLfloat>> scene;
    
    // Table in between the couches:
    vector<vector<GLfloat>> table = build_cube();
    table = pivot_coordinates(table);
    table = to_homogeneous_coord(table);
    table = mat_mult(scaling_matrix(+1.0, +0.4, +1.0), table);
    table = mat_mult(translation_matrix(+0.0, +0.2, +0.0), table);
    table = to_cartesian_coord(table);
    table = pivot_coordinates(table);
    scene.insert(scene.end(), table.begin(), table.end());
    
    // Couch along positive z-axis:
    // Arm-rest 1
    vector<vector<GLfloat>> arm_rest1 = build_cube();
    arm_rest1 = pivot_coordinates(arm_rest1);
    arm_rest1 = to_homogeneous_coord(arm_rest1);
    arm_rest1 = mat_mult(scaling_matrix(+0.2, +1.2, +1.2), arm_rest1);
    arm_rest1 = mat_mult(translation_matrix(+1.2, +0.6, +2.0), arm_rest1);
    arm_rest1 = to_cartesian_coord(arm_rest1);
    arm_rest1 = pivot_coordinates(arm_rest1);
    scene.insert(scene.end(), arm_rest1.begin(), arm_rest1.end());
    // Arm-rest 2
    vector<vector<GLfloat>> arm_rest2 = build_cube();
    arm_rest2 = pivot_coordinates(arm_rest2);
    arm_rest2 = to_homogeneous_coord(arm_rest2);
    arm_rest2 = mat_mult(scaling_matrix(+0.2, +1.2, +1.2), arm_rest2);
    arm_rest2 = mat_mult(translation_matrix(-1.2, +0.6, +2.0), arm_rest2);
    arm_rest2 = to_cartesian_coord(arm_rest2);
    arm_rest2 = pivot_coordinates(arm_rest2);
    scene.insert(scene.end(), arm_rest2.begin(), arm_rest2.end());
    // Couch back 1
    vector<vector<GLfloat>> back1 = build_cube();
    back1 = pivot_coordinates(back1);
    back1 = to_homogeneous_coord(back1);
    back1 = mat_mult(scaling_matrix(+2.4, +1.2, +0.1), back1);
    back1 = mat_mult(translation_matrix(+0.0, +0.6, +2.55), back1);
    back1 = to_cartesian_coord(back1);
    back1 = pivot_coordinates(back1);
    scene.insert(scene.end(), back1.begin(), back1.end());
    // Couch seat 1
    vector<vector<GLfloat>> seat1 = build_cube();
    seat1 = pivot_coordinates(seat1);
    seat1 = to_homogeneous_coord(seat1);
    seat1 = mat_mult(scaling_matrix(+2.4, +0.3, +1.1), seat1);
    seat1 = mat_mult(translation_matrix(+0.0, +0.15, +2.0), seat1);
    seat1 = to_cartesian_coord(seat1);
    seat1 = pivot_coordinates(seat1);
    scene.insert(scene.end(), seat1.begin(), seat1.end());
    
    // Couch along negative z-axis:
    // Arm-rest 3
    vector<vector<GLfloat>> arm_rest3 = build_cube();
    arm_rest3 = pivot_coordinates(arm_rest3);
    arm_rest3 = to_homogeneous_coord(arm_rest3);
    arm_rest3 = mat_mult(scaling_matrix(+0.2, +1.2, +1.2), arm_rest3);
    arm_rest3 = mat_mult(translation_matrix(+1.2, +0.6, -2.0), arm_rest3);
    arm_rest3 = to_cartesian_coord(arm_rest3);
    arm_rest3 = pivot_coordinates(arm_rest3);
    scene.insert(scene.end(), arm_rest3.begin(), arm_rest3.end());
    // Arm-rest 4
    vector<vector<GLfloat>> arm_rest4 = build_cube();
    arm_rest4 = pivot_coordinates(arm_rest4);
    arm_rest4 = to_homogeneous_coord(arm_rest4);
    arm_rest4 = mat_mult(scaling_matrix(+0.2, +1.2, +1.2), arm_rest4);
    arm_rest4 = mat_mult(translation_matrix(-1.2, +0.6, -2.0), arm_rest4);
    arm_rest4 = to_cartesian_coord(arm_rest4);
    arm_rest4 = pivot_coordinates(arm_rest4);
    scene.insert(scene.end(), arm_rest4.begin(), arm_rest4.end());
    // Couch back 2
    vector<vector<GLfloat>> back2 = build_cube();
    back2 = pivot_coordinates(back2);
    back2 = to_homogeneous_coord(back2);
    back2 = mat_mult(scaling_matrix(+2.4, +1.2, +0.1), back2);
    back2 = mat_mult(translation_matrix(+0.0, +0.6, -2.55), back2);
    back2 = to_cartesian_coord(back2);
    back2 = pivot_coordinates(back2);
    scene.insert(scene.end(), back2.begin(), back2.end());
    // Couch seat 2
    vector<vector<GLfloat>> seat2 = build_cube();
    seat2 = pivot_coordinates(seat2);
    seat2 = to_homogeneous_coord(seat2);
    seat2 = mat_mult(scaling_matrix(+2.4, +0.3, +1.1), seat2);
    seat2 = mat_mult(translation_matrix(+0.0, +0.15, -2.0), seat2);
    seat2 = to_cartesian_coord(seat2);
    seat2 = pivot_coordinates(seat2);
    scene.insert(scene.end(), seat2.begin(), seat2.end());
    
    // Cabinet behind couch:
    // Right panel
    vector<vector<GLfloat>> right_panel = build_cube();
    right_panel = pivot_coordinates(right_panel);
    right_panel = to_homogeneous_coord(right_panel);
    right_panel = mat_mult(scaling_matrix(+0.01, +1.0, +1.0), right_panel);
    right_panel = mat_mult(translation_matrix(+1.0, +0.5, +3.25), right_panel);
    right_panel = to_cartesian_coord(right_panel);
    right_panel = pivot_coordinates(right_panel);
    scene.insert(scene.end(), right_panel.begin(), right_panel.end());
    // Left panel
    vector<vector<GLfloat>> left_panel = build_cube();
    left_panel = pivot_coordinates(left_panel);
    left_panel = to_homogeneous_coord(left_panel);
    left_panel = mat_mult(scaling_matrix(+0.01, +1.0, +1.0), left_panel);
    left_panel = mat_mult(translation_matrix(-1.0, +0.5, +3.25), left_panel);
    left_panel = to_cartesian_coord(left_panel);
    left_panel = pivot_coordinates(left_panel);
    scene.insert(scene.end(), left_panel.begin(), left_panel.end());
    // Top panel
    vector<vector<GLfloat>> top_panel = build_cube();
    top_panel = pivot_coordinates(top_panel);
    top_panel = to_homogeneous_coord(top_panel);
    top_panel = mat_mult(scaling_matrix(+2.0, +0.01, +1.0), top_panel);
    top_panel = mat_mult(translation_matrix(+0.0, +1.005, +3.25), top_panel);
    top_panel = to_cartesian_coord(top_panel);
    top_panel = pivot_coordinates(top_panel);
    scene.insert(scene.end(), top_panel.begin(), top_panel.end());
    // Bottom panel
    vector<vector<GLfloat>> bottom_panel = build_cube();
    bottom_panel = pivot_coordinates(bottom_panel);
    bottom_panel = to_homogeneous_coord(bottom_panel);
    bottom_panel = mat_mult(scaling_matrix(+2.0, +0.01, +1.0), bottom_panel);
    bottom_panel = mat_mult(translation_matrix(+0.0, +0.005, +3.25), bottom_panel);
    bottom_panel = to_cartesian_coord(bottom_panel);
    bottom_panel = pivot_coordinates(bottom_panel);
    scene.insert(scene.end(), bottom_panel.begin(), bottom_panel.end());
    
    return scene;
}

// Construct the color mapping of the scene
vector<vector<GLfloat>> init_color(vector<vector<GLfloat>> scene) {
    vector<vector<GLfloat>> colors(scene.size(), vector<GLfloat>(scene[0].size(), +0.0));
    
    for (int i = 0; i < scene.size(); i++) {
        for (int j = 0; j < scene[0].size(); j++) {
          colors[i][j] = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
        }
    }
    
    return colors;
}

void display_func() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    SCENE = pivot_coordinates(SCENE);
    SCENE = to_homogeneous_coord(SCENE);
    // Every 600 frames, the scene changes rotation direction
    if (TURNS % 1000 <= 500) {
        SCENE = mat_mult(rotation_matrix_y(THETA), SCENE);
    } else {
        SCENE = mat_mult(rotation_matrix_y(-THETA), SCENE);
    }
    TURNS += 1;
    SCENE = to_cartesian_coord(SCENE);
    SCENE = pivot_coordinates(SCENE);
    
    GLfloat* scene_vertices = two_d_vector_to_array(SCENE);
    GLfloat* color_vertices = two_d_vector_to_array(COLOR);
    // Pass the scene vertex pointer
    glVertexPointer(3,                  // 3 components (x, y, z)
                    GL_FLOAT,           // Vertex type is GL_FLOAT
                    0,                  // Start position in referenced memory
                    scene_vertices);    // Pointer to memory location to read from
    
    // Pass the color vertex pointer
    glColorPointer(3,                   // 3 components (r, g, b)
                   GL_FLOAT,            // Vertex type is GL_FLOAT
                   0,                   // Start position in referenced memory
                   color_vertices);     // Pointer to memory location to read from
    
    // Draw quad point planes: each 4 vertices
    glDrawArrays(GL_QUADS, 0, (int) SCENE.size());
    
    glFlush();            //Finish rendering
    glutSwapBuffers();
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
    glutCreateWindow("Assignment 3");
    
    setup();
    init_camera();
    // Setting global variables SCENE and COLOR with actual values
    SCENE = init_scene();
    COLOR = init_color(SCENE);
    
    // Set up our display function
    glutDisplayFunc(display_func);
    glutIdleFunc(idle_func);
    // Render our world
    glutMainLoop();
    
    // Remember to call "delete" on your dynmically allocated arrays
    // such that you don't suffer from memory leaks. e.g.
    // delete arr;
    
    return 0;
}
