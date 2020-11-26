#include <chrono>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <random>

#include "src/proto/training_example.pb.h"

using namespace cv;
using namespace std::chrono;

#define REALSENSE_CAM_ID 0
// #define REALSENSE_CAM_ID 6

std::string SerializeImage(Mat m) {
  std::vector<uchar> bytes;
  bool encode_ok = imencode(".jpg", m, bytes);
  if (!encode_ok) {
    std::cout << "Encode image failed!" << std::endl;
  }
  std::string res(bytes.begin(), bytes.end());
  return res;
}

int main(int argc, char** argv) {
  VideoCapture cap;
  if (!cap.open(REALSENSE_CAM_ID, CAP_V4L)) {
    std::cout << "Open cam " << REALSENSE_CAM_ID << " failed." << std::endl;
    return 0;
  }

  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(1, 6);

  visual_controller::TrainingExamples examples;

  // Take several initial frames.
  for (size_t i = 0; i < 10; i++) {
    Mat frame;
    cap >> frame;
  }

  for (size_t i = 0; i < 10; i++) {
    auto example = examples.add_examples();

    example->mutable_command()->set_type(
        static_cast<visual_controller::Command::CommandType>(
            distribution(generator)));
    std::cout << visual_controller::Command::CommandType_Name(
                     example->command().type())
              << std::endl;

    const auto start_t = high_resolution_clock::now();
    auto cur_t = high_resolution_clock::now();

    int num_frames = 0;
    // Each command takes 500ms / 15 frames.
    while (duration_cast<microseconds>(cur_t - start_t).count() < 5e5) {
      Mat frame;
      cap >> frame;
      std::cout << "Frame " << num_frames << " ";
      if (frame.empty()) break;  // end of video stream
      imshow("this is you, smile! :)", frame);
      if (waitKey(1) == 27) break;  // stop capturing by pressing ESC
      const auto imstr = SerializeImage(frame);
      auto example_frame = example->add_frames();
      example_frame->set_image(imstr);
      example_frame->set_timestamp(
          duration_cast<microseconds>(cur_t.time_since_epoch()).count());
      auto t4 = high_resolution_clock::now();
      std::cout << "iteration took "
                << duration_cast<microseconds>(t4 - cur_t).count()
                << " microseconds" << std::endl;
      num_frames++;
      cur_t = high_resolution_clock::now();
    }
    std::cout << "Example " << i << " contains " << num_frames << " frames."
              << std::endl;
  }

  int64 timestamp = duration_cast<microseconds>(
                        high_resolution_clock::now().time_since_epoch())
                        .count();

  // std::cout << "examples str: "<< examples.DebugString();

  std::string examples_string = examples.SerializeAsString();
  std::ostringstream outfile;
  outfile << timestamp << ".binarypb";
  std::ofstream out(outfile.str());
  out << examples_string;
  out.close();

  return 0;
}