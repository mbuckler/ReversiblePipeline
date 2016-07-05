#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

struct testarray { float array_[3][3]; };

using namespace std;

// Get color space transform
vector<vector<float>> get_Ts(char* cam_model_path);

// Get white balance transform
float** get_Tw(char* cam_model_path, int wb_index);

// Get combined transforms for checking
float** get_TsTw(char* cam_model_path, int wb_index);

// Get control points
float** get_ctrl_pnts(char* cam_model_path, bool pipeline);

// Get weights
float** get_weights(char* cam_model_path, bool pipeline);

// Get coeficients 
float** get_coeficients(char* cam_model_path, bool pipeline);

// Get reverse tone mapping
float** get_rev_tone(char* cam_model_path);
