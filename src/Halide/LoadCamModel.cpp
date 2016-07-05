#include "LoadCamModel.h"

using namespace std;

// Get color space transform
vector<vector<float>> get_Ts(char* cam_model_path) {

  //float Ts[3][3] = {{}};
  vector<vector<float>> Ts;

  std::ifstream file(cam_model_path+std::string("raw2jpg_transform.txt"));

  std::string   line;

  int line_idx = 0;


  // Read one line at a time into the variable line:
  while(std::getline(file, line))
  {
      std::vector<float> lineData,temp;
      std::stringstream  lineStream(line);
      float value;
      // Read one value at a time from the line
      while(lineStream >> value)
      {
          lineData.push_back(value);
      }
      if (line_idx>=1 && line_idx<=3) {
        for (int i=0; i<3; i++) {
          temp.push_back(lineData[i]);
        }
        Ts.push_back(temp);      
      }

      line_idx = line_idx + 1;
  }

  return Ts;
}

// Get white balance transform
float** get_Tw(char* cam_model_path, int wb_index) {

}

// Get combined transforms for checking
float** get_TsTw(char* cam_model_path, int wb_index) {

}

// Get control points
float** get_ctrl_pnts(char* cam_model_path, bool pipeline) {

}

// Get weights
float** get_weights(char* cam_model_path, bool pipeline) {

}

// Get coeficients 
float** get_coeficients(char* cam_model_path, bool pipeline) {

}

// Get reverse tone mapping
float** get_rev_tone(char* cam_model_path) {

}


