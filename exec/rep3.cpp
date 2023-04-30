#include <iostream>
#include <vector>
#include <string>

#include "opencv2/opencv.hpp"

int main(int, char **)
{
    cv::Mat truth = cv::imread("images/3/A.pgm", cv::IMREAD_GRAYSCALE);
    cv::Mat approx = cv::imread("images/3/B.pgm", cv::IMREAD_GRAYSCALE);

    cv::Mat floatTruth, floatApprox;
    truth.convertTo(floatTruth, CV_32F);
    approx.convertTo(floatApprox, CV_32F);

    cv::Mat diff;

    cv::subtract(floatApprox, floatTruth, diff);

    double maxVal;
    double minVal;
    cv::minMaxLoc(diff, &minVal, &maxVal);

    diff = 255 * (diff - minVal) / (maxVal - minVal);
    cv::Mat intDiff;
    diff.convertTo(intDiff, CV_8U);

    cv::Mat turbo_map;
    cv::applyColorMap(intDiff, turbo_map, cv::ColormapTypes::COLORMAP_JET);

    cv::namedWindow("diff", cv::WINDOW_AUTOSIZE);
    cv::imshow("diff", diff);

    cv::namedWindow("intDiff", cv::WINDOW_AUTOSIZE);
    cv::imshow("intDiff", intDiff);

    cv::namedWindow("turbo_map", cv::WINDOW_AUTOSIZE);
    cv::imshow("turbo_map", turbo_map);
    cv::waitKey(10000);

    cv::imwrite("images/3/results/diff.png", diff);
    cv::imwrite("images/3/results/intDiff.png", intDiff);
    cv::imwrite("images/3/results/turbo_map.png", turbo_map);

    cv::destroyAllWindows();
}
