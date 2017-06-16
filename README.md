# ReversiblePipeline

This is an implementation of a pre-learned camera image
processing model. A description of the model can be found in
"A New In-Camera Imaging Model for Color Computer Vision 
and its Application" by Seon Joo Kim, Hai Ting Lin, Zheng Lu,
Sabine Susstrunk, Stephen Lin, and Michael S. Brown. Code for
learning a new model can be found at the original project 
page. This particular implementation was written by 
Mark Buckler.

If you are looking for the full Configurable & Reversible Imaging Pipeline (CRIP) as seen in the paper [Reconfiguring the Imaging Pipeline for Computer Vision](https://arxiv.org/abs/1705.04352) by Mark Buckler, Suren Jayasuriya, and Adrian Sampson, see [this repository](https://github.com/cucapra/approx-vision) instead.

Original Project Page:
* http://www.comp.nus.edu.sg/~brown/radiometric_calibration/

Model Format Readme:
* http://www.comp.nus.edu.sg/~brown/radiometric_calibration/datasets/Model_param/readme.pdf


## Pipeline Implementation Descriptions

* Strict Forward Pipeline

A forward only pipeline which displays intermediate as well as final
outputs is provided in the src/scripts directory. Use this if you 
want a fast forward pipeline but dont need to emulate a specific
camera model or reverse the processing. This implementation uses
LibRaw and OpenCV.

* Reversible Matlab Pipeline

This implementation can be found in src/Matlab. Use this pipeline
if you would like to use one of the available forward and backward
camera pipeline models, aren't concerned about speed, and would 
like to take advantage of Matlab's many pre-built image processing 
tools. Note that this implementation is prohibitively slow 
if you would like to process large images. Its default is to do 
patch based processing to enable a reasonable run time.

* Reversible Halide Pipeline

This can be found in src/Halide. Use this pipeline if you want to 
process full high resolution images with both a backward and 
forward image processing pipeline implementation. The model is 
fully implemented with Halide Funcs, and some basic scheduling
for loop unrolling and pre-computation is provided. Scheduling for
paralellism on the CPU or GPU can be easily added by the user.
Do note that while this implementation is much faster than Matlab,
it will take close to 10 minutes to process a 5000x3000 pixel image.

## General Use Instructions

Instructions for using all of the different pieces of the pipeline
can be found in readmes like this one within the appropriate 
directories. All code has been tested with Ubuntu 14.04, but with
small edits to makefiles all code should work with OS X as well.

## Dependencies

* LibRaw (for preprocessing)
* OpenCV (for preprocessing)
* Matlab (for Matlab pipeline)
* Halide (for Halide pipeline)
