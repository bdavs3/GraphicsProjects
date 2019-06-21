# Assignment 1: Cartoonify

### Loyola Marymount University CMSI 371 Computer Graphics

##### Professor Alex Wong

**Due Friday, February 15, 2019 11:59 PM PST**
The assignment will focus on the concept of Chaikin, Bézier curve algorithm. Given a headshot photo of an individual (e.g. the headshot of Ed Sheeran), generate the cartoon version of the photo by sketching it using Chaikin or Bézier curves.
Skeleton code has been provided to guide you along the way. The places that you will be required to implement has been marked with a ​TODO​.
I have provided you with a simple Vertex class that allows you to specify the x and y values of a point. You will utilize this class for modeling the control points of your sketch.
\
_Note_: the C++ vector class is the equivalent to a list in most other languages. You may use the `push_back(Object o)`​ function of the vector class to hold your set of points.

You will complete the following functions for the assignment:

1. generate_points​ : a function that generates takes in a set of control points for your Chaikin or Bézier curve algorithm and returns the new set of control points\
   **parameters**: vector<Vertex>\
   **returns**: vector<Vertex>
2. draw_curve​ : calls ​generate_points​ to generate the control points using the Chaikin or Bézier curve algorithm and forms a curve by connecting the points with lines\
   **parameters**: vector<Vertex>, int\
   **returns**: none\
   The parameter ​`n_iter​` refers to the number of iterations to run the Chaikin or Bézier algorithm. Recall that each time the algorithm is ran, you will obtain a set of new points.

#### Submission

You will submit the following to Brightspace:

- assignment1.cpp
- Your sketch in JPEG, JPG, or PNG: results.{jpeg, jpg, png}
- The photo your sketch was based on in JPEG, JPG or PNG: photo.{jpeg, jpg, png}

#### Grading

- I will be compiling the assignment using the following command:
  ```sh
  gcc -o assignment1 assignment1.cpp -std=c++14 -lGL -lGLU -lglut
  ```
- Your code must compile for me to assign points!
- Your assignment will be graded on:
  80% the correctness of your implementation of Bézier’s algorithm
  20% effort placed recreating the subject via your sketch (e.g. a simple happy face does not do Ed Sheeran justice)

#### Late Policy

- For each day the assignment is late, 50% of its worth will be deducted, e.g. 100% on time, 50% 1 day late, 25% 2 days late, etc.
