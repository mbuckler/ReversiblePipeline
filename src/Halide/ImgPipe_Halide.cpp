//////////////////////////
// Update this later
///////////////////////////////


#include "Halide.h"
#include "ImgPipeConfig.h"
#include "LoadCamModel.h"
#include <stdio.h>

#include "halide_image_io.h"

int main(int argc, char **argv) {

  struct testarray Ts;

  Ts = get_Ts(cam_model_path);

  for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
      printf("%f\n",Ts.array_[i][j]);
    }
  }

  /*

  using namespace Halide;

  // Support code for loading pngs.
  using namespace Halide::Tools;


  // First we'll declare some Vars to use below.
  Var x("x"), y("y"), c("c");

  // Inform user of usage method
  if ( argc != 2 ) 
  {
      printf("usage: \n> ImgPipe <Input_Image_Path>\n");
      return -1; 
  }

  // Now we'll express a multi-stage pipeline that blurs an image
  // first horizontally, and then vertically.
  {
    // Take a color 8-bit input
    Image<uint8_t> input = load_image(argv[argc-1]);

    // Upgrade it to 16-bit, so we can do math without it overflowing.
    Func input_16("input_16");
    input_16(x, y, c) = cast<uint16_t>(input(x, y, c));

    // Blur it horizontally:
    Func blur_x("blur_x");
    blur_x(x, y, c) = (input_16(x-1, y, c) +
                       2 * input_16(x, y, c) +
                       input_16(x+1, y, c)) / 4;

    // Blur it vertically:
    Func blur_y("blur_y");
    blur_y(x, y, c) = (blur_x(x, y-1, c) +
                       2 * blur_x(x, y, c) +
                       blur_x(x, y+1, c)) / 4;

    // Convert back to 8-bit.
    Func output("output");
    output(x, y, c) = cast<uint8_t>(blur_y(x, y, c));

    // Each Func in this pipeline calls a previous one using
    // familiar function call syntax (we've overloaded operator()
    // on Func objects). A Func may call any other Func that has
    // been given a definition. This restriction prevents
    // pipelines with loops in them. Halide pipelines are always
    // feed-forward graphs of Funcs.

    // Now let's realize it...

    // Image<uint8_t> result = output.realize(input.width(), input.height(), 3);

    // Except that the line above is not going to work. Uncomment
    // it to see what happens.

    // Realizing this pipeline over the same domain as the input
    // image requires reading pixels out of bounds in the input,
    // because the blur_x stage reaches outwards horizontally, and
    // the blur_y stage reaches outwards vertically. Halide
    // detects this by injecting a piece of code at the top of the
    // pipeline that computes the region over which the input will
    // be read. When it starts to run the pipeline it first runs
    // this code, determines that the input will be read out of
    // bounds, and refuses to continue. No actual bounds checks
    // occur in the inner loop; that would be slow.
    //
    // So what do we do? There are a few options. If we realize
    // over a domain shifted inwards by one pixel, we won't be
    // asking the Halide routine to read out of bounds. We saw how
    // to do this in the previous lesson:
    Image<uint8_t> result(input.width()-2, input.height()-2, 3);
    result.set_min(1, 1);
    output.realize(result);

    // Save the result. It should look like a slightly blurry
    // parrot, and it should be two pixels narrower and two pixels
    // shorter than the input image.
    save_image(result, "blurry_parrot_1.png");

    // This is usually the fastest way to deal with boundaries:
    // don't write code that reads out of bounds :) The more
    // general solution is our next example.
  }
  */

  printf("Success!\n");
  return 0;
}
