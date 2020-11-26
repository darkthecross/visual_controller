#include <chrono>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "src/proto/training_example.pb.h"

using namespace cv;
using namespace std::chrono;

#define REALSENSE_CAM_ID 6

Mat DeserializeImage(const std::string& s) {
  std::vector<uchar> bytes(s.begin(), s.end());
  return imdecode(bytes, IMREAD_UNCHANGED);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Wrong num of args. Should be: ./inspector xxx.binarypb"
              << std::endl;
    return -1;
  }

  std::ifstream fin;
  fin.open(argv[1]);
  if (!fin.is_open()) {
    std::cerr << "Open file failed for " << argv[1] << std::endl;
  }

  std::cout << "Inspecting " << argv[1] << "..." << std::endl;

  std::string pb_str((std::istreambuf_iterator<char>(fin)),
                     std::istreambuf_iterator<char>());

  visual_controller::TrainingExamples examples;
  if (!examples.ParseFromString(pb_str)) {
    std::cerr << "Parse proto failed for " << argv[1] << std::endl;
    return -2;
  }

  std::cout << "Num examples: " << examples.examples_size() << std::endl;

  size_t ex_id = 0;
  for (const auto& ex : examples.examples()) {
    std::cout << "Example " << ex_id << ": ";
    std::cout << "Command type: "
              << visual_controller::Command::CommandType_Name(
                     ex.command().type()) << " ";
    std::cout << "Frame count: " << ex.frames_size() << std::endl;

    size_t frame_id = 0;
    auto frame_start_ts = high_resolution_clock::now();
    int64 disp_duration = 3e4;

    Mat frame = DeserializeImage(ex.frames(frame_id).image());

    imshow("frame", frame);
    if (waitKey(1) == 27) return 0;  // stop capturing by pressing ESC

    while (true) {
      auto t = high_resolution_clock::now();
      const int64 micros_passed =
          duration_cast<microseconds>(t - frame_start_ts).count();
      if (micros_passed > disp_duration) {
        frame_id = (frame_id + 1) % ex.frames_size();
        frame = DeserializeImage(ex.frames(frame_id).image());
        imshow("frame", frame);
        if (waitKey(1) == 27) break;  // stop capturing by pressing ESC
        frame_start_ts = t;
      }
    }
    ex_id++;
  }

  return 0;
}