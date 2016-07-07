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
      ctrl_pts_h(x,y) = ctrl_pts[y][x];
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
  

  //Image<float> ctrl_pts_h( Buffer(Float(32), N, M, 0, 0, &ctrl_pts) );


  // Load input image 
  Image<uint8_t> input = load_image(demosaiced_image);

  // Declare image handle variables
  Var x, y, c;

  Func input_16("input_16");
  input_16(x,y,c)  = cast<uint16_t>(input(x,y,c));
  

  // Color map and white balance transform
  Func transform("transform");
  transform(x,y,c) = max( select(
    // Perform matrix multiplication, set min of 0
    c == 0, input_16(x,y,0)*TsTw_tran[0][0] + input_16(x,y,1)*TsTw_tran[1][0] + input_16(x,y,2)*TsTw_tran[2][0],
    c == 1, input_16(x,y,0)*TsTw_tran[0][1] + input_16(x,y,1)*TsTw_tran[1][1] + input_16(x,y,2)*TsTw_tran[2][1],
            input_16(x,y,0)*TsTw_tran[0][2] + input_16(x,y,1)*TsTw_tran[1][2] + input_16(x,y,2)*TsTw_tran[2][2])
                          , 0);



  // Weighted radial basis function for gamut mapping
  Func rbf_ctrl_pts("rbf_ctrl_pts");
  // Initialization:     red_val, green_val, blue_val
  rbf_ctrl_pts(x,y,c) = {    0.0,       0.0,      0.0};
  // Index to iterate with
  RDom idx(0,num_ctrl_pts-1);
  // Loop code
  // Subtract the vectors 
  Expr red_sub   = transform(x,y,0) - ctrl_pts_h(0,idx);
  Expr green_sub = transform(x,y,1) - ctrl_pts_h(1,idx);
  Expr blue_sub  = transform(x,y,2) - ctrl_pts_h(2,idx);
  // Take the L2 norm to get the distance
  Expr dist      = sqrt( red_sub*red_sub + green_sub*green_sub + blue_sub*blue_sub );
  // Extract the old values
  Expr old_red   = rbf_ctrl_pts(x,y,c)[0];
  Expr old_green = rbf_ctrl_pts(x,y,c)[1];
  Expr old_blue  = rbf_ctrl_pts(x,y,c)[2];
  Expr new_red   = old_red +   (weights_h(0,idx) * dist);
  Expr new_green = old_green + (weights_h(1,idx) * dist);
  Expr new_blue  = old_blue +  (weights_h(2,idx) * dist);
  // Update persistant loop variables
  rbf_ctrl_pts(x,y,c) = { new_red, new_green, new_blue};




  Func output_8("output_8");
  output_8(x,y,c) = cast<uint8_t>(transform(x,y,c));

  Func output_f("output_f");
  output_f(x,y,c) = cast<float>(transform(x,y,c));

  // Realize the function(s)
  Image<uint8_t> output =
      output_8.realize(input.width(), input.height(), input.channels());
  Image<float> transformed = 
      output_f.realize(input.width(), input.height(), input.channels());

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

  // Save the output for inspection
  save_image(output, "output.png");

  return 0;
}

