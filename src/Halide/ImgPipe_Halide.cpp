//////////////////////////
// Update this later
///////////////////////////////

#include "Halide.h"
#include "ImgPipeConfig.h"
#include "LoadCamModel.h"
#include "MatrixOps.h"
#include <stdio.h>
#include "halide_image_io.h"

// Function prototypes
int run_pipeline(bool direction);


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
 



  using namespace Halide;

  using namespace Halide::Tools;


  Image<uint8_t> input = load_image(demosaiced_image);

  // Next we define our Func object that represents our stage
  Halide::Func brighter;
  Halide::Var x, y, c;

  // For each pixel of the input image.
  Halide::Expr value = input(x, y, c);

  // Selectively brighten
  value = select(c == 0, input(x,y,c) * 2.0f,
                 c == 1, input(x,y,1)*1.0f+input(x,y,2)*1.0f ,
                       input(x,y,c)*1.0f);

  // Clamp it to be less than 255
  value = Halide::min(value, 255.0f);

  // Cast it back to an 8-bit unsigned integer.
  value = Halide::cast<uint8_t>(value);

  // Define the function.
  brighter(x, y, c) = value;

  // The equivalent one-liner to all of the above is:
  // brighter(x, y, c) = Halide::cast<uint8_t>(min(input(x, y, c) * 1.5f, 255));

  // Realize the function
  Halide::Image<uint8_t> output =
      brighter.realize(input.width(), input.height(), input.channels());

  // Save the output for inspection. It should look like a bright parrot.
  save_image(output, "brighter.png");

  return 0;
}
