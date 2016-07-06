#include "MatrixOps.h"

using namespace std;

// Matrix Matrix Dot Product
vector<vector<float>> dot_matmat (vector<vector<float>> a, vector<vector<float>> b) {

}

// Vector Matrix Dot Product
vector<float> dot_vecmat (vector<float> a, vector<vector<float>> b) {

}

// Vector Vector Dot Product
float dot_vecvec (vector<float> a, vector<float> b) {

  float result = 0;
  int   size_a = a.size();
  int   size_b = b.size();

  // Verify that both vectors are of the same size
  if (size_a != size_b)
    throw invalid_argument("Attempt to dot product vectors of different size");
  
  // Multiply the vectors
  for (int i=0; i<size_a; i++) {
    result += a[i] * b[i];
  }

  return result;
}

// Three by three matrix inversion
vector<vector<float>> inv_3x3mat (vector<vector<float>> inmat) {

}
