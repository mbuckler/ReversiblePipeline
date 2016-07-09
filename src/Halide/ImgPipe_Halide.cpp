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
int   run_pipeline(bool direction);
float degreen(float in);

int main(int argc, char **argv) {
  using namespace std;

  // Run forward pipeline
  run_pipeline(0);

  // Run backward pipeline
  //run_pipeline(1);

  printf("Success!\n");
  return 0;
}

int run_pipeline(bool direction) {

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

  // Convert control point and weights to Halide Images
  int width  = ctrl_pts[0].size();
  int length = ctrl_pts.size();
  Image<float> ctrl_pts_h(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      // Scale the control points to fit 8 bits
      ctrl_pts_h(x,y) = 256 * ctrl_pts[y][x];
    }
  }
  width  = weights[0].size();
  length = weights.size();
  Image<float> weights_h(width,length);
  for (int y=0; y<length; y++) {
    for (int x=0; x<width; x++) {
      weights_h(x,y) = weights[y][x];
    }
  }

  // Scale the bias coeficients to fit 8 bits
  //for (int i=0;i<4;i++) {
    for (int j=0;j<3;j++) {
      coefs[0][j] = 256 * coefs[0][j];
    }
  //}



  int patchsize = 1;
  int xstart    = 551;
  int ystart    = 2751;
 
  // Load input image 
  Image<uint8_t> input = load_image(demosaiced_image);
//  input.set_min(xstart, ystart);

 // Define the patch to process
 //  Image<uint8_t> patch(patchsize, patchsize, 3);
 // patch.set_min(xstart, ystart, 0);
  // Copy the subsection into the patch
  
  



  // Declare image handle variables
  Var x, y, c;

  Func input_16("input_16");
  input_16(x,y,c)  = cast<float>(input(x,y,c));
  

  // Color map and white balance transform
  Func transform("transform");
  transform(x,y,c) = max( select(
    // Perform matrix multiplication, set min of 0
    c == 0, input_16(x,y,0)*TsTw_tran[0][0]
          + input_16(x,y,1)*TsTw_tran[1][0]
          + input_16(x,y,2)*TsTw_tran[2][0],
    c == 1, input_16(x,y,0)*TsTw_tran[0][1]
          + input_16(x,y,1)*TsTw_tran[1][1]
          + input_16(x,y,2)*TsTw_tran[2][1],
            input_16(x,y,0)*TsTw_tran[0][2]
          + input_16(x,y,1)*TsTw_tran[1][2]
          + input_16(x,y,2)*TsTw_tran[2][2])
                          , 0);
  transform.trace_stores();

  // Weighted radial basis function for gamut mapping
  Func rbf_ctrl_pts("rbf_ctrl_pts");
  // Initialization with all zero
  rbf_ctrl_pts(x,y,c) = cast<float>(0);
  // Index to iterate with
  RDom idx(0,num_ctrl_pts);
  // Loop code
  // Subtract the vectors 
  Expr red_sub   = transform(x,y,0) - ctrl_pts_h(0,idx);
  Expr green_sub = transform(x,y,1) - ctrl_pts_h(1,idx);
  Expr blue_sub  = transform(x,y,2) - ctrl_pts_h(2,idx);
  // Take the L2 norm to get the distance
  Expr dist      = sqrt( red_sub*red_sub + green_sub*green_sub + blue_sub*blue_sub );
  // Update persistant loop variables
  rbf_ctrl_pts(x,y,c) = select( c == 0, rbf_ctrl_pts(x,y,c) + (weights_h(0,idx) * dist),
                                c == 1, rbf_ctrl_pts(x,y,c) + (weights_h(1,idx) * dist),
                                        rbf_ctrl_pts(x,y,c) + (weights_h(2,idx) * dist));
  //rbf_ctrl_pts.trace_stores();


  // Works!
  //Image<float> ctrl_pt_out = 
  //  rbf_ctrl_pts.realize(input.width(), input.height(), input.channels());


  // Add on the biases for the RBF
  Func rbf_biases("rbf_biases");
  rbf_biases(x,y,c) = max( select( 
    c == 0, rbf_ctrl_pts(x,y,0) + coefs[0][0] + coefs[1][0]*transform(x,y,0) +
      coefs[2][0]*transform(x,y,1) + coefs[3][0]*transform(x,y,2),
    c == 1, rbf_ctrl_pts(x,y,1) + coefs[0][1] + coefs[1][1]*transform(x,y,0) +
      coefs[2][1]*transform(x,y,1) + coefs[3][1]*transform(x,y,2),
            rbf_ctrl_pts(x,y,2) + coefs[0][2] + coefs[1][2]*transform(x,y,0) +
      coefs[2][2]*transform(x,y,1) + coefs[3][2]*transform(x,y,2))
                          , 0);
  rbf_biases.trace_stores();

  // Cast the output to 8 bit
  Func output_8("output_8");
  output_8(x,y,c) = cast<uint8_t>(rbf_biases(x,y,c));
  output_8.trace_stores();





  // Realize the functions
  Image<uint8_t> output(patchsize,patchsize,3);
  output.set_min(xstart,ystart);
  output_8.realize(output);

  // Save the output for inspection
  save_image(output, "output.png");



/*
  // Further processing
  //float rbf_vals[3] = {0};
  //float red_sub, green_sub, blue_sub;
  //float dist;
  Func zeros;
  zeros(x) = 0;
  Image<float> rbf_vals  = zeros.realize(3);
  Image<float> red_sub   = zeros.realize(1);
  Image<float> green_sub = zeros.realize(1);
  Image<float> blue_sub  = zeros.realize(1);
  Image<float> dist      = zeros.realize(1);


  // Gammut mapping and tone mapping
  for (int i=0;i<input.width();i++) {
    for (int j=0;j<input.height();j++) {
      /////////////////////////////////////////////////////
      // RBF for gamut mapping
      // RBF weighted control points
      for (int pnt_idx=0;pnt_idx<num_ctrl_pts;pnt_idx++) {
        // Compute distance
        red_sub(0)   = transformed(x,y,0) - ctrl_pts[pnt_idx][0];
        green_sub(0) = transformed(x,y,1) - ctrl_pts[pnt_idx][1];
        blue_sub(0)  = transformed(x,y,2) - ctrl_pts[pnt_idx][2];
        dist(0)      = sqrtf( powf(red_sub(0),2)+powf(green_sub(0),2)+powf(blue_sub(0),2) );
        // Update the rbf values
        for (int rgb=0;rgb<3;rgb++) {
          rbf_vals(rgb) = rbf_vals(rgb) + (weights[pnt_idx][rgb] * dist(0));
        }        
      }
      // RBF biases
      for (int rgb=0;rgb<3;rgb++) {
        rbf_vals(rgb) = rbf_vals(rgb) + (coefs[0][rgb]);
        rbf_vals(rgb) = rbf_vals(rgb) + (coefs[1][rgb] * transformed(x,y,0));
        rbf_vals(rgb) = rbf_vals(rgb) + (coefs[2][rgb] * transformed(x,y,1));
        rbf_vals(rgb) = rbf_vals(rgb) + (coefs[3][rgb] * transformed(x,y,2));
      }
      // Write values to gammutmapped image 
      for (int rgb=0;rgb<3;rgb++) {
        transformed(x,y,rgb) = rbf_vals(rgb);
      }


    }
  }
*/

  return 0;
}

