
#####################################################################
# Halide implementation of the forward and backward pipeline
#####################################################################

#####################################################################
# How to build
#####################################################################

Install Halide, update the Makefile with the location of your 
Halide installation

Update ImgPipeConfig.h to account for any changes you would like
> vim ImgPipeConfig.h

In this directory, run make
> make

Then execute the code with the ImgPipe command
> ./ImgPipe

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

#####################################################################
# Additional notes
#####################################################################

A few changes may be needed to use the Makefile on non Linux
distributions

Halide can only load and store png images
