This is the Matlab implementation of the pipeline. If you are 
only interested in running code with the provided images and model,
then all you need to do is run ImgPipe_Matlab.m. If you would
like to run with other images, you will need some details about
the images, and then will need to perform a Raw to Tiff conversion.


* Code changes

1) Point the Matlab code to the folder where your images are

2) Point the Matlab code to where the model is stored

3) The default test sites and sizes may not work for your image.
Change them to wherever you would like to test.

4) Raw image mosaic pattern: Not all cameras have the same pixel 
alignment. Ensure that the forward and backward pipelines have
the right pattern.

5) Raw image scale: Not all cameras have the same output bitwidth.
Change the scale values to scale up to 16 bits.


* Raw to Tiff conversion

To convert from your raw file format to tiff files which can be 
read by Matlab, follow these steps. Note that you will need 
OpenCV and LibRaw installed. For more information see RAW2TIFF.cpp

1) Compile the converter
> cd ReversiblePipeline/src/Matlab/
> make

2) Convert your file
> ./RAW2TIFF <path-to-image>/<image-file-name>

This will create a new tiff file in the input image's directory
by reformatting (but not scaling or processing) the input raw image.
