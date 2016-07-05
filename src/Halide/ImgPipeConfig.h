///////////////////////////////////////////////////////////////
// Image Pipeline Halide Configuration File
///////////////////////////////////////////////////////////////
//
// This file defines the parameters associated with the camera
// model and the input images. These parameters can be found
// within the camera model files themselves. The format of 
// the camera model files is described at the link below. 
//
// Model format readme:
// http://www.comp.nus.edu.sg/~brown/radiometric_calibration/datasets/Model_param/readme.pdf
//
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
// Input Image Parameters
///////////////////////////////////////////////////////////////

// Demosaiced raw input (to forward pipeline)
extern char demosaiced_image[] = 
"../../imgs/NikonD7000FL/DSC_0916.NEF.raw_1C.tiff.demosaiced.tif";

// Jpeg input (to backward pipeline)
extern char jpg_image[] =
"../../imgs/NikonD7000FL/DSC_0916.JPG";

///////////////////////////////////////////////////////////////
// Camera Model Parameters
///////////////////////////////////////////////////////////////

// Path to the camera model to be used
extern char cam_model_path[] =
"../../camera_models/NikonD7000/";

// White balance index (select white balance from transform file)
// The first white balance in the file has a wb_index of 1
// For more information on model format see the readme
extern int wb_index = 
6;

// Number of control points
#define NUM_CTRL_PTS 3702

///////////////////////////////////////////////////////////////
// Patches to test 
///////////////////////////////////////////////////////////////

// Patch start locations
// [xstart, ystart]
//
// NOTE: These start locations must align with the demosiac 
// pattern start if using the version of this pipeline with
// demosaicing

extern int patchstarts[12][2] = {
  {551,  2751},
  {1001, 2751},
  {1501, 2751},
  {2001, 2751},
  {551,  2251},
  {1001, 2251},
  {1501, 2251},
  {2001, 2251},
  {551,  1751},
  {1001, 1751},
  {1501, 1751},
  {2001, 1751}
};

// Height and width of patches
extern int patchsize = 
10;

// Number of patches to test
extern int patchnum  = 
1;
