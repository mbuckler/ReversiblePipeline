/////////////////////////////////////////////////////////////////////////
//
//  Raw Image Preprocessor
//
//  This script is built to preprocess raw files for both the Halide and 
//  Matlab reversible imaging pipelines. Raw files are used as input
//  to the forward imaging pipelines, and as reference output for the 
//  the backward imaging pipelines. Since neither Halide nor Matlab can
//  read raw file formats (such as Nikon .NEF), LibRaw is used to 
//  receive raw data, and OpenCV is used to scale, preprocess as needed,
//  and write readable files.
//
//  Input:
//    Single raw image file (tested with 14 bit Nikon .NEF)
//  
//  Output:
//    <infilepath>/<infilename>.raw_1C.tiff:
//      A scaled version of the raw input, stored in a 1 channel tiff
//      Used by the Matlab pipeline
//    <infilepath>/<infilename>.demos_3C.png: 
//      Scaled and demosiaced raw input, stored in a 3 channel png
//      Used by the Halide pipeline
//
//  Author: Mark Buckler
//
/////////////////////////////////////////////////////////////////////////
//
//  How to use:
//    1) Install LibRaw and OpenCV
//      LibRaw docs: http://www.libraw.org/docs/API-CXX-eng.html
//      OpenCV docs: http://docs.opencv.org/3.1.0/
//    2) Change to this directory (ReversiblePipeline/src/scripts)
//    3) Compile with the provided Makefile
//      > make
//    4) Determine the bitdepth of your raw input file
//    5) Run the script
//      > ./RawPreproc path/to/image <raw file bitdepth>
//      example:
//      > ./RawPreproc ../../imgs/NikonD7000FL/DSC_0916.NEF 14
//
/////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>

using namespace std;
using namespace cv;

int main(int argc, char** argv )
{
  
  // Inform user of usage method
  if ( argc != 3 )
  {
      printf("usage: \n./RawPreproc path/to/image <raw file bitdepth>\n");
      return -1;
  }

  ////////////////////////////////////////////////////////////////////
  //  LibRaw raw data extraction  ////////////////////////////////////

  // Establish use of LibRaw
  LibRaw RawProcessor;
  #define imgdata RawProcessor.imgdata

  // Function failure flag
  int ret;

  // Path to input
  const char * in_path = argv[1];

  // Bitdepth of raw input
  int num_raw_bits = atoi(argv[2]);

  // Read in raw image with LibRaw
  if ((ret = RawProcessor.open_file(in_path)) != LIBRAW_SUCCESS)
  {
      fprintf(stderr, in_path, libraw_strerror(ret));
      return -1; 
  }

  // Unpack the raw image, storing it in imgdata.rawdata.raw_image
  if ((ret = RawProcessor.unpack()) != LIBRAW_SUCCESS)
  {
      return -1; 
  }
 
  ////////////////////////////////////////////////////////////////////
  //  Raw data for Matlab ////////////////////////////////////////////

  // Extract raw data stored in 16 bit, 1 channel image
  Mat raw_1C = Mat(
    imgdata.sizes.raw_height,
    imgdata.sizes.raw_width,
    CV_16UC1,
    imgdata.rawdata.raw_image
    );

  // Scale the data to fit the 16 bit representation
  int scale = 1 << (16-num_raw_bits);
  raw_1C = raw_1C * scale;

  // Write the raw 1 channel representation to file
  imwrite( (std::string(in_path)+".raw_1C.tiff").c_str(), raw_1C);

  ////////////////////////////////////////////////////////////////////
  //  Demosaiced data for Halide /////////////////////////////////////

  // Demosiac (debayer) the image
  Mat bgr_3C;
  cvtColor(raw_1C, bgr_3C, CV_BayerRG2RGB);
  
  // Write the demosiaced 12 bit 3 channel representation to file
  imwrite( (std::string(in_path)+".demos_3C.png").c_str(), bgr_3C);


  return 0;
}
