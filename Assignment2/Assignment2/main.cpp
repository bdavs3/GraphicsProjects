/***
 Assignment-2: Rotating a Cube in 3-Dimensional Space
 
 Name: Davis, Ben
 
 Collaborators: N/A
 Project Summary: This program creates a cube in 3D space and rotates it about the X, Y, and Z axes.  In order to do this, the points making up the planes of the cube are first converted to homogenous.  They are then multiplied by (already homogenous) rotation matrices for each axis.  Finally, the rotated points are converted back to Cartesian coordinates so that they may be drawn in space.  I decided to almost exclusively use 2-dimensional vectors in all functions (to represent matrices) so that they could be accessed in the following way: matrix[row][column].
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

#include <math.h>
#include <vector>
#include <iostream>
using namespace std;


GLfloat theta = 0.0;

// Converts degrees to radians for rotation
GLfloat deg2rad(GLfloat d) {
    return (d*M_PI) / 180.0;
}

// Converts a 2D vector to an array.  I made some large modifications to this function because of the fact that I decided to work exclusively with 2D vectors in this program.
GLfloat* two_d_vector_to_array(vector<vector<GLfloat>> vec) {
    vector<GLfloat> one_d_vector(vec.size() * vec[0].size());
    
    for (int i = 0; i < vec[0].size(); i++) {
        for (int j = 0; j < vec.size(); j++) {
            one_d_vector[(3*i)+j] = vec[j][i];
        }
    }
    
    GLfloat* arr = new GLfloat[one_d_vector.size()];
    for (int i = 0; i < one_d_vector.size(); i++) {
        arr[i] = one_d_vector[i];
    }
    
    return arr;
}

// Converts Cartesian coordinates to homogeneous coordinates
vector<vector<GLfloat>> to_homogenous_coord(vector<vector<GLfloat>> cartesian_coords) {
    vector<vector<GLfloat>> result(cartesian_coords.size(), vector<GLfloat>(cartesian_coords[0].size(), 0.0));
    
    for (int i = 0; i < cartesian_coords.size(); i++){
        for (int j = 0; j < cartesian_coords[0].size(); j++) {
            result[i][j] = cartesian_coords[i][j];
        }
    }
    
    vector<GLfloat> appended_vector(cartesian_coords[0].size(), 0.0);
    appended_vector[appended_vector.size() - 1] = 1.0;
    
    result.resize(result.size() + 1, appended_vector);
    
    return result;
}

// Converts homogenous coordinates to Cartesian coordinates
vector<vector<GLfloat>> to_cartesian_coord(vector<vector<GLfloat>> homogenous_coords) {
    vector<vector<GLfloat>> result(homogenous_coords.size(), vector<GLfloat>(homogenous_coords[0].size(), 0.0));
    
    for (int i = 0; i < homogenous_coords.size(); i++){
        for (int j = 0; j < homogenous_coords[0].size(); j++) {
            result[i][j] = homogenous_coords[i][j];
        }
    }
    
    result.resize(result.size() - 1);
    
    return result;
}

// Definition of a rotation matrix about the x-axis theta degrees
vector<vector<GLfloat>> rotation_matrix_x (float theta) {
    vector<vector<GLfloat>> rotate_mat_x;
    
    theta = deg2rad(theta);

    rotate_mat_x = {
        {1.0, 0.0, 0.0, 0.0},
        {0.0, cos(theta), -sin(theta), 0.0},
        {0.0, sin(theta), cos(theta), 0.0},
        {0.0, 0.0, 0.0, 1.0},
    };
    
    return rotate_mat_x;
}


// Definition of a rotation matrix along the y-axis by theta degrees
vector<vector<GLfloat>> rotation_matrix_y (float theta) {
    vector<vector<GLfloat>> rotate_mat_y;
    
    theta = deg2rad(theta) / 2.5; // rotates a bit slower than X rotation
    
    rotate_mat_y = {
        {cos(theta), 0.0, -sin(theta), 0.0},
        {0.0, 1.0, 0.0, 0.0},
        {sin(theta), 0.0, cos(theta), 0.0},
        {0.0, 0.0, 0.0, 1.0},
    };
    
    return rotate_mat_y;
}


// Definition of a rotation matrix along the z-axis by theta degrees
vector<vector<GLfloat>> rotation_matrix_z (float theta) {
    vector<vector<GLfloat>> rotate_mat_z;
    
    theta = deg2rad(theta) / 5.0; // rotates a bit slower than X and Y rotations
    
    rotate_mat_z = {
        {cos(theta), -sin(theta), 0.0, 0.0},
        {sin(theta), cos(theta), 0.0, 0.0},
        {0.0, 0.0, 1.0, 0.0},
        {0.0, 0.0, 0.0, 1.0},
    };
    
    return rotate_mat_z;
}

// Perform matrix multiplication for A B
vector<vector<GLfloat>> mat_mult(vector<vector<GLfloat>> A, vector<vector<GLfloat>> B) {
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
    
    return result;
}

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
    // Define a 50 degree field of view, 1:1 aspect ratio, near and far planes at 3 and 7
    gluPerspective(50.0, 1.0, 2.0, 10.0);
    // Position camera at (2, 3, 5), attention at (0, 0, 0), up at (0, 1, 0)
    gluLookAt(2.0, 6.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void display_func() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // World model parameters
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    vector<vector<GLfloat>> points = {
//        Changed to the following form:
//        [ x1, x2, x3, ...
//          y1, y2, y3, ...
//          z1, z2, z3, ... ]
        {+1.0, -1.0, -1.0, +1.0, +1.0, -1.0, -1.0, +1.0, +1.0, +1.0, +1.0, +1.0, -1.0, -1.0, -1.0, -1.0, +1.0, -1.0, -1.0, +1.0, +1.0, -1.0, -1.0, +1.0},
        {+1.0, +1.0, -1.0, -1.0, +1.0, +1.0, -1.0, -1.0, +1.0, +1.0, -1.0, -1.0, +1.0, +1.0, -1.0, -1.0, +1.0, +1.0, +1.0, +1.0, -1.0, -1.0, -1.0, -1.0},
        {+1.0, +1.0, +1.0, +1.0, -1.0, -1.0, -1.0, -1.0, -1.0, +1.0, +1.0, -1.0, -1.0, +1.0, +1.0, -1.0, +1.0, +1.0, -1.0, -1.0, +1.0, +1.0, -1.0, -1.0},
    };

    GLfloat colors[] = {
        // Front plane
        1.0,    0.0,    0.0,
        1.0,    0.0,    0.0,
        1.0,    0.0,    0.0,
        1.0,    0.0,    0.0,
        // Back plane
        0.0,    1.0,    0.0,
        0.0,    1.0,    0.0,
        0.0,    1.0,    0.0,
        0.0,    1.0,    0.0,
        // Right
        0.0,    0.0,    1.0,
        0.0,    0.0,    1.0,
        0.0,    0.0,    1.0,
        0.0,    0.0,    1.0,
        // Left
        1.0,    1.0,    0.0,
        1.0,    1.0,    0.0,
        1.0,    1.0,    0.0,
        1.0,    1.0,    0.0,
        // Top
        1.0,    0.0,    1.0,
        1.0,    0.0,    1.0,
        1.0,    0.0,    1.0,
        1.0,    0.0,    1.0,
        // Bottom
        0.0,    1.0,    1.0,
        0.0,    1.0,    1.0,
        0.0,    1.0,    1.0,
        0.0,    1.0,    1.0,
    };
    
    vector<vector<GLfloat>> rotated_points = to_homogenous_coord(points);
    
    rotated_points = mat_mult(rotation_matrix_x(theta), rotated_points);
    rotated_points = mat_mult(rotation_matrix_y(theta), rotated_points);
    rotated_points = mat_mult(rotation_matrix_z(theta), rotated_points);
    
    rotated_points = to_cartesian_coord(rotated_points);
    
    GLfloat* vertices = two_d_vector_to_array(rotated_points);
    
    glVertexPointer(3,          // 3 components (x, y, z)
                    GL_FLOAT,   // Vertex type is GL_FLOAT
                    0,          // Start position in referenced memory
                    vertices);  // Pointer to memory location to read from
    
    //pass the color pointer
    glColorPointer(3,           // 3 components (r, g, b)
                   GL_FLOAT,    // Vertex type is GL_FLOAT
                   0,           // Start position in referenced memory
                   colors);     // Pointer to memory location to read from
    
    // Draw quad point planes: each 4 vertices
    glDrawArrays(GL_QUADS, 0, 4*6);
    
    glFlush();			//Finish rendering
    glutSwapBuffers();
}

void idle_func() {
    theta = theta+0.075;
    display_func();
}

int main (int argc, char **argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    // Create a window with rendering context and everything else we need
    glutCreateWindow("Assignment 2");
    
    setup();
    init_camera();
    
    // Set up our display function
    glutDisplayFunc(display_func);
    glutIdleFunc(idle_func);
    // Render our world
    glutMainLoop();
    return 0;
}
