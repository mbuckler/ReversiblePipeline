
#####################################################################
# Matlab implementation of the forward and backward pipeline
#####################################################################

#####################################################################
# How to use 
#####################################################################

Directions below are for using this pipeline with new images,
If you are only interested in running with the provided images 
and model, then all you need to do is run ImgPipe_Matlab.m. 

  1) Install Matlab
    Matlab docs: http://www.mathworks.com/help/install/ug/install-mathworks-software.html

  2) Run the raw preprocessor in src/scripts. Note that matlab
     wants the scaled but not demosaiced version. This will have
     an extension of .raw_1C.tiff, not .demos_3C.png.

  3) Edit any points within the matlab code that you would like
     to change. This might include:
     * Where your processed images are
     * Where your camera model is stored
     * The patches to test in your image. Ensure that your xstart and
       ystart align with the start of the bayer mosaic pattern.
     * The raw image mosaic pattern. Not all cameras have the same pixel 
       alignment. Ensure that the forward and backward pipelines have
       the right pattern.

  4) Run ImgPipe_Matlab.m

  5) Observe your output in src/Matlab/pipe_results/

#####################################################################
# Pipeline Results Folder Contents 
#####################################################################

After running the pipeline you will see a new folder created in this
directory called pipe_results. In this directory you will have folders
for forward results and images as well as backward results and images.
The result text files contain values and error metrics for each of 
the patches which have been tested. The image folders contain the 
image outputs of each of the respective pipelines.
