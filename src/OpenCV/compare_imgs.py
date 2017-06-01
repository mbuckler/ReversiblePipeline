# import the necessary packages

#from skimage.measure import structural_similarity as ssim
import matplotlib.pyplot as plt
import numpy as np
import cv2

def get_mse(imageA, imageB):
	# the 'Mean Squared Error' between the two images is the
	# sum of the squared difference between the two images;
	# NOTE: the two images must have the same dimension
	err = np.sum((imageA.astype("float") - imageB.astype("float")) ** 2)
	err /= float(imageA.shape[0] * imageA.shape[1] * imageA.shape[2])
	
	# return the MSE, the lower the error, the more "similar"
	# the two images are
	return err

def get_psnr(imageA,imageB):
	mse = get_mse(imageA,imageB)
	psnr = (20 * np.log10(256))-(10 * np.log10(mse))
	return psnr

def get_perc_error(imageA,imageB):
	err = np.sum(abs((imageA.astype("float") - imageB.astype("float"))) )
	err /= float(imageA.shape[0] * imageA.shape[1] * imageA.shape[2])
	perc_err = err/ 256
        perc_err *= 100
	# return the MSE, the lower the error, the more "similar"
	# the two images are
	return perc_err


real_isp_output = cv2.imread("../../imgs/NikonD7000FL/DSC_0916.png")
forward_pipeline_out = cv2.imread("../../imgs/NikonD7000FL/forward_pipe_output.png")
forward_pipeline_out = forward_pipeline_out[0:3264,0:4928] #Crop out camera test pixels
real_demosiaced_raw = cv2.imread("../../imgs/NikonD7000FL/DSC_0916.NEF.demos_3C.png")
real_demosiaced_raw = real_demosiaced_raw[0:3264,0:4928] #Crop out camera test pixels
backward_pipeline_out = cv2.imread("../../imgs/NikonD7000FL/backward_pipe_output.png")

print "===Forward pipeline==="
print "Per pix % error: "+str(get_perc_error(real_isp_output,forward_pipeline_out))
print "MSE: "+str(get_mse(real_isp_output,forward_pipeline_out))
print "PSNR: "+str(get_psnr(real_isp_output,forward_pipeline_out))
print "===Backward pipeline==="
print "Per pix % error: "+str(get_perc_error(real_demosiaced_raw,backward_pipeline_out))
print "MSE: "+str(get_mse(real_demosiaced_raw,backward_pipeline_out))
print "PSNR: "+str(get_psnr(real_demosiaced_raw,backward_pipeline_out))
