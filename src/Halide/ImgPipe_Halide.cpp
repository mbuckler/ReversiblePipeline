//////////////////////////
// Update this later
///////////////////////////////

#include "Halide.h"
#include "ImgPipeConfig.h"
#include "LoadCamModel.h"
#include "MatrixOps.h"
#include <stdio.h>
#include <math.h>
#include "halide_image_io.h"

// Function prototypes
int run_pipeline(bool direction, bool full, int patchsize, int xstart, int ystart);


int main(int argc, char **argv) {
  using namespace std;

  bool full = true;

  int patchsize = 1;
  int xstart    = 551;
  int ystart    = 2751; 

  // Run forward pipeline
  run_pipeline(true, full, patchsize, xstart, ystart);

  // Run backward pipeline
  //run_pipeline(false, full, patchsize, xstart, ystart);

  printf("Success!\n");
  return 0;
}


// Reversible pipeline function
int run_pipeline(bool direction, bool full, int patchsize, int xstart, int ystart) {

  using namespace std;  

  // Declare model parameters
  vector<vector<float>> Ts, Tw, TsTw;
  vector<vector<float>> ctrl_pts, weights, coefs;
  vector<vector<float>> rev_tone;

  // Load model parameters from file
  Ts        = get_Ts       (cam_model_path);
  Tw        = get_Tw       (cam_model_path, wb_index);
  TsTw      = get_TsTw     (cam_model_path, wb_index);
  ctrl_pts  = get_ctrl_pts (cam_model_path, num_ctrl_pts, direction);
  weights   = get_weights  (cam_model_path, num_ctrl_pts, direction);
  coefs     = get_coefs    (cam_model_path, num_ctrl_pts, direction);
  rev_tone  = get_rev_tone (cam_model_path);

/*
  // Verify that Ts*Tw = TsTw
  vector<vector<float>> TsTw_test = dot_matmat(Ts,Tw);
  disp_mat(TsTw);
  disp_mat(TsTw_test);
  if (TsTw_test != TsTw)
    throw runtime_error("Error: TsTw != Ts*Tw. Possible model import error or improper model format");
*/
 
  // Take the transpose of the color map and white balance transform for later use
  vector<vector<float>> TsTw_tran = transpose_mat (TsTw);


  using namespace Halide;
  using namespace Halide::Tools;

  // Convert control points to a Halide image
  int width  = ctrl_pts[0].size();
  int length = ctrl_pts.size();
  Image<float> ctrl_pts_h(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      ctrl_pts_h(x,y) = ctrl_pts[y][x];
    }
  }
  // Convert weights to a Halide image
  width  = weights[0].size();
  length = weights.size();
  Image<float> weights_h(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      weights_h(x,y) = weights[y][x];
    }
  }
  // Convert the reverse tone mapping function to a Halide image
  width  = 3;
  length = 256;
  Image<float> rev_tone_h(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      rev_tone_h(x,y) = rev_tone[y][x];
    }
  }
 

  // Load input image 
  Image<uint8_t> input = load_image(demosaiced_image);
  // Declare image handle variables
  Var x, y, c;


  // Cast input to float and scale according to its 8 bit input format
  Func input_scaled("input_scaled");
  input_scaled(x,y,c)  = cast<float>(input(x,y,c))/256;
  

  // Color map and white balance transform
  Func transform("transform");
  transform(x,y,c) = max( select(
    // Perform matrix multiplication, set min of 0
    c == 0, input_scaled(x,y,0)*TsTw_tran[0][0]
          + input_scaled(x,y,1)*TsTw_tran[1][0]
          + input_scaled(x,y,2)*TsTw_tran[2][0],
    c == 1, input_scaled(x,y,0)*TsTw_tran[0][1]
          + input_scaled(x,y,1)*TsTw_tran[1][1]
          + input_scaled(x,y,2)*TsTw_tran[2][1],
            input_scaled(x,y,0)*TsTw_tran[0][2]
          + input_scaled(x,y,1)*TsTw_tran[1][2]
          + input_scaled(x,y,2)*TsTw_tran[2][2])
                          , 0);

  //transform.trace_stores();

  Image<float> transformed =
    transform.realize(input.width(), input.height(), input.channels());


  // Weighted radial basis function for gamut mapping
  Func rbf_ctrl_pts("rbf_ctrl_pts");
  // Initialization with all zero
  rbf_ctrl_pts(x,y,c) = cast<float>(0);
  // Index to iterate with
  RDom idx(0,num_ctrl_pts);
  // Loop code
  // Subtract the vectors 
  Expr red_sub   = transformed(x,y,0) - ctrl_pts_h(0,idx);
  Expr green_sub = transformed(x,y,1) - ctrl_pts_h(1,idx);
  Expr blue_sub  = transformed(x,y,2) - ctrl_pts_h(2,idx);
  // Take the L2 norm to get the distance
  Expr dist      = sqrt( red_sub*red_sub + green_sub*green_sub + blue_sub*blue_sub );
  // Update persistant loop variables
  rbf_ctrl_pts(x,y,c) = select( c == 0, rbf_ctrl_pts(x,y,c) + (weights_h(0,idx) * dist),
                                c == 1, rbf_ctrl_pts(x,y,c) + (weights_h(1,idx) * dist),
                                        rbf_ctrl_pts(x,y,c) + (weights_h(2,idx) * dist));
  //rbf_ctrl_pts.trace_stores();

  Image<float> after_ctrl_pts =
    rbf_ctrl_pts.realize(input.width(), input.height(), input.channels());
 

  // Add on the biases for the RBF
  Func rbf_biases("rbf_biases");
  rbf_biases(x,y,c) = max( select( 
    c == 0, after_ctrl_pts(x,y,0) + coefs[0][0] + coefs[1][0]*transform(x,y,0) +
      coefs[2][0]*transform(x,y,1) + coefs[3][0]*transform(x,y,2),
    c == 1, after_ctrl_pts(x,y,1) + coefs[0][1] + coefs[1][1]*transform(x,y,0) +
      coefs[2][1]*transform(x,y,1) + coefs[3][1]*transform(x,y,2),
            after_ctrl_pts(x,y,2) + coefs[0][2] + coefs[1][2]*transform(x,y,0) +
      coefs[2][2]*transform(x,y,1) + coefs[3][2]*transform(x,y,2))
                          , 0);
  rbf_biases.trace_stores();


  // Forward tone mapping
  Func tonemap("tonemap");
  RDom idx2(0,256);
  // Theres a lot in this one line! Functionality wise it finds the entry in 
  // the reverse tone mapping function which is closest to the value found by
  // gamut mapping. This entry is then scaled by 256 to ensure that the output
  // is fractional, just like the rest of the pipeline.
  tonemap(x,y,c) = argmin( abs( rev_tone_h(c,idx2) - rbf_biases(x,y,c) ) )[0]  / 256.0f;
  //tonemap.trace_stores();


  // Rescale the output and cast the output to 8 bit for image writing
  Func output_8("output_8");
  output_8(x,y,c) = cast<uint8_t>(tonemap(x,y,c)*256);
  //output_8.trace_stores();


  // Realize the functions
  if (full == true) {
    // Put full image through pipeline
    Image<uint8_t> output =
      output_8.realize(input.width(), input.height(), input.channels());
    // Save the output for inspection
    save_image(output, "output.png");
  } else {
    // Only compute a patch
    Image<uint8_t> output(patchsize,patchsize,3);
    output.set_min(xstart,ystart);
    output_8.realize(output);
    // Save the output for inspection
    save_image(output, "output.png");
  }

  return 0;
}

