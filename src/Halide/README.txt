
#####################################################################
# Halide implementation of the forward and backward pipeline
#####################################################################

#####################################################################
# How to use
#####################################################################

  1) Install Halide in ~/packages/Halide/. Alternatively, install in 
     a directory of your choice and update the makefile.
    Halide docs: http://halide-lang.org/index.html#gettingstarted

  2) If you would like to run the forward version of the pipeline
     on a new image, go to src/scripts and run the RawPreprocessor
     on the raw image to demosaic it and write it to a png. 

  3) Update ImgPipeConfig.h to account for any changes you would like
    > vim ImgPipeConfig.h

  4) In this directory, run make
    > make

  5) Then execute the code with the ImgPipe command
    > ./ImgPipe

  6) Observe your output file(s) in this directory

#####################################################################
# Directory contents
#####################################################################

* ImgPipeConfig.h
Config file for the image pipeline. Contains information on the
location of the camera model and the image to test.

* ImgPipe_Halide.cpp
The primary file, contains all Halide code for the pipeline

* LoadCamModel.cpp
Loads all information from the camera model

* MatrixOps.cpp
A few linear algebra functions. Implementing them here prevents
the need to install YAL (Yet Another Library)

* clock.h
Helper file for determining computation speed

#####################################################################
# Additional notes
#####################################################################

A few changes may be needed to use the Makefile on non Linux
distributions
