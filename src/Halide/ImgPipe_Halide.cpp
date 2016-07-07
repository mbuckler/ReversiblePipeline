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
 
/*
  // Weighted radial basis function for gamut mapping
  Func rbf_ctrl_pts("rbf_ctrl_pts");
  // Initialization:     red_val, green_val, blue_val
  rbf_ctrl_pts(x,y,c) = {      0,         0,        0};
  // Index to iterate with
  RDom idx(0,num_ctrl_pts-1);
  // Loop code
  // Subtract the vectors 
  Expr red_sub   = transform(x,y,0) - ctrl_pts[r][0];
  Expr green_sub = transform(x,y,1) - ctrl_pts[r][1];
  Expr blue_sub  = transform(x,y,2) - ctrl_pts[r][2];
  // Take the L2 norm to get the distance
  Expr dist      = sqrtf( powf(red_sub,2)+powf(green_sub,2)+powf(blue_sub,2) );
  // Extract the old values
  Expr old_red   = rbf_ctrl_pts(x,y,c)

  // Update persistant loop variables
  rbf_ctrl_pts(x,y,c) = { , , };
*/

  Func output_8("output_8");
  output_8(x,y,c) = cast<uint8_t>(transform(x,y,c));

  // Realize the function
  Image<uint8_t> output =
      output_8.realize(input.width(), input.height(), input.channels());


  for (int i=0;i<input.width();i++) {
    for (int j=0;j<input.height();j++) {
      for (int rgb=0;rgb<3;rgb++) {
        output(i,j,rgb) = output(i,j,rgb) * 0.5f; 
      }
    }
  }


  // Save the output for inspection. It should look like a bright parrot.
  save_image(output, "output.png");

  return 0;
}

float degreen(float in) {
  
  return in * 0.5;

}

