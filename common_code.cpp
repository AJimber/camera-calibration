#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include "common_code.hpp"

std::vector<cv::Point3f>
fsiv_generate_3d_calibration_points(const cv::Size &board_size,
                                    float square_size)
{
    std::vector<cv::Point3f> ret_v;
    for(int r = 1; r <= board_size.height; ++r)
    {
        for(int c = 1; c <= board_size.width; ++c)
        {
            ret_v.push_back(cv::Point3f(c * square_size, r * square_size, 0.0));
        }
    }
    CV_Assert(ret_v.size() == static_cast<size_t>(board_size.width * board_size.height));
    return ret_v;
}

bool fsiv_find_chessboard_corners(const cv::Mat &img, const cv::Size &board_size,
                                  std::vector<cv::Point2f> &corner_points,
                                  int win_r,
                                  int zz_r,
                                  const cv::TermCriteria &tcr)
{
    CV_Assert(img.type() == CV_8UC3);
    bool was_found = false;
    was_found = cv::findChessboardCorners(img, board_size, corner_points);
    
    if(was_found)
    {
        cv::Mat img_gray;
        cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY); 
        cv::cornerSubPix(img_gray, corner_points, cv::Size(win_r, win_r), cv::Size(zz_r, zz_r), tcr);
    }

    CV_Assert(!was_found || static_cast<int>(corner_points.size()) == (board_size.width * board_size.height));
    return was_found;
}

float fsiv_calibrate_camera(const std::vector<std::vector<cv::Point2f>> &_2d_points,
                            const std::vector<std::vector<cv::Point3f>> &_3d_points,
                            const cv::Size &camera_size,
                            cv::Mat &camera_matrix,
                            cv::Mat &dist_coeffs,
                            std::vector<cv::Mat> *rvecs,
                            std::vector<cv::Mat> *tvecs)
{
    CV_Assert(_3d_points.size() >= 2 && _3d_points.size() == _2d_points.size());
    float error = 0.0;

    std::vector<cv::Mat> rvecs_;
    std::vector<cv::Mat> tvecs_;

    error = cv::calibrateCamera(_3d_points, _2d_points, camera_size, camera_matrix, 
        dist_coeffs, rvecs_, tvecs_);
    
    if(rvecs)
    {
        *rvecs = rvecs_;
    }
    if(tvecs)
    {
        *tvecs = tvecs_;
    }

    CV_Assert(camera_matrix.rows == camera_matrix.cols &&
              camera_matrix.rows == 3 &&
              camera_matrix.type() == CV_64FC1);
    CV_Assert((dist_coeffs.rows * dist_coeffs.cols) == 5 &&
              dist_coeffs.type() == CV_64FC1);
    CV_Assert(rvecs == nullptr || rvecs->size() == _2d_points.size());
    CV_Assert(tvecs == nullptr || tvecs->size() == _2d_points.size());
    return error;
}

void fsiv_save_calibration_parameters(cv::FileStorage &fs,
                                      const cv::Size &camera_size,
                                      float error,
                                      const cv::Mat &camera_matrix,
                                      const cv::Mat &dist_coeffs,
                                      const cv::Mat &rvec,
                                      const cv::Mat &tvec)
{
    CV_Assert(fs.isOpened());
    CV_Assert(camera_matrix.type() == CV_64FC1 && camera_matrix.rows == 3 && camera_matrix.cols == 3);
    CV_Assert(dist_coeffs.type() == CV_64FC1 && dist_coeffs.rows == 1 && dist_coeffs.cols == 5);
    CV_Assert(rvec.type() == CV_64FC1 && rvec.rows == 3 && rvec.cols == 1);
    CV_Assert(tvec.type() == CV_64FC1 && tvec.rows == 3 && tvec.cols == 1);

    fs << "image-width" << camera_size.width;
    fs << "image-height" << camera_size.height;
    fs << "error" << error;
    fs << "camera-matrix" << camera_matrix;
    fs << "distortion-coefficients" << dist_coeffs;
    fs << "rvec" << rvec;
    fs << "tvec" << tvec;

    CV_Assert(fs.isOpened());
    return;
}

void fsiv_load_calibration_parameters(cv::FileStorage &fs,
                                      cv::Size &camera_size,
                                      float &error,
                                      cv::Mat &camera_matrix,
                                      cv::Mat &dist_coeffs,
                                      cv::Mat &rvec,
                                      cv::Mat &tvec)
{
    CV_Assert(fs.isOpened());

    fs["image-width"] >> camera_size.width;
    fs["image-height"] >> camera_size.height;
    fs["error"] >> error;
    fs["camera-matrix"] >> camera_matrix;
    fs["distortion-coefficients"] >> dist_coeffs;
    fs["rvec"] >> rvec;
    fs["tvec"] >> tvec;

    CV_Assert(fs.isOpened());
    CV_Assert(camera_matrix.type() == CV_64FC1 && camera_matrix.rows == 3 && camera_matrix.cols == 3);
    CV_Assert(dist_coeffs.type() == CV_64FC1 && dist_coeffs.rows == 1 && dist_coeffs.cols == 5);
    CV_Assert(rvec.type() == CV_64FC1 && rvec.rows == 3 && rvec.cols == 1);
    CV_Assert(tvec.type() == CV_64FC1 && tvec.rows == 3 && tvec.cols == 1);
    return;
}

void fsiv_undistort_image(const cv::Mat &input, cv::Mat &output,
                          const cv::Mat &camera_matrix,
                          const cv::Mat &dist_coeffs)
{
    cv::undistort(input, output, camera_matrix, dist_coeffs);
}

void fsiv_undistort_video_stream(cv::VideoCapture &input_stream,
                                 cv::VideoWriter &output_stream,
                                 const cv::Mat &camera_matrix,
                                 const cv::Mat &dist_coeffs,
                                 const int interp,
                                 const char *input_wname,
                                 const char *output_wname,
                                 double fps)
{
    CV_Assert(input_stream.isOpened());
    CV_Assert(output_stream.isOpened());

    cv::Mat input_image, output_image;
    input_stream >> input_image;

    if(input_image.empty())
        return;

    cv::Mat map1, map2;
    cv::initUndistortRectifyMap(camera_matrix, dist_coeffs, cv::Mat(), camera_matrix, input_image.size(), CV_32F, map1, map2);
    
    // Controla la tecla de escape
    int key = 0;
    while(!input_image.empty() && key != 27) // Si pulsa la tecla 27 (escape) termina
    {
        cv::remap(input_image, output_image, map1, map2, interp);
        
        if(input_wname)
        {
            cv::imshow(input_wname, input_image);
        }
        if(output_wname)
        {
            cv::imshow(output_wname, output_image);
        }
        
        key = cv::waitKey(1000 / fps) & 0xff;
    }

    CV_Assert(input_stream.isOpened());
    CV_Assert(output_stream.isOpened());
}
