#include <iostream>
#include <vector>
#include <string>

#include "opencv2/opencv.hpp"
#include "Eigen/Core"

enum PatternType
{
    PATTERN0,
    PATTERN1,
    PATTERN2S,
    PATTERN2D,
    PATTERN3,
    PATTERN4
};

enum Pattern2dType
{
    PATTERN2DP,
    PATTERN2DM,
    PATTERN2DE,

};

Pattern2dType findPattern2dType(Eigen::Array<float, 1, 4> values, float alpha)
{
    float c0 = values(3) + values(1) - values(0) - values(2);
    float c1 = values(0) - values(3);
    float c2 = values(2) - values(3);
    float c3 = values(3);
    float D = c1 * c2 - c0 * c3 + c0 * alpha;

    if (D > 0)
    {
        return PATTERN2DP;
    }
    else if (D < 0)
    {
        return PATTERN2DM;
    }
    else
    {
        return PATTERN2DE;
    }
}

class Pattern
{
public:
    Pattern()
    {
        pattern = PATTERN0;
        pattern2d = PATTERN2DE;
        patternNumber = 0;
    }
    Pattern(Eigen::Array<bool, 1, 4> c, Eigen::Array<float, 1, 4> v, float a)
    {
        values = v;
        corners = c;
        alpha = a;
        int count = corners.count();
        switch (count)
        {
        case 0:
            pattern = PATTERN0;
            pattern2d = PATTERN2DE;
            patternNumber = 0;
            break;
        case 1:
            pattern = PATTERN1;
            pattern2d = PATTERN2DE;
            for (int i = 0; i < 4; i++)
            {
                if (corners(i))
                {
                    patternNumber = i;
                    break;
                }
            }
            points.push_back(cv::Point2d(25, 0)); // top
            points.push_back(cv::Point2d(0, 25)); // left
            break;
        case 3:
            pattern = PATTERN3;
            pattern2d = PATTERN2DE;
            for (int i = 0; i < 4; i++)
            {
                if (!corners(i))
                {
                    patternNumber = i;
                    break;
                }
            }
            points.push_back(cv::Point2d(25, 0)); // bottom
            points.push_back(cv::Point2d(0, 25)); // left
            break;
        case 4:
            pattern = PATTERN4;
            pattern2d = PATTERN2DE;
            patternNumber = 0;
            break;
        case 2:
            for (int i = 0; i < 4; i++)
            {
                int next = (i + 1) % 4;
                int prec = (i + 3) % 4;
                if (corners(i) && corners(next))
                {
                    pattern = PATTERN2S;
                    pattern2d = PATTERN2DP;
                    patternNumber = i;
                    points.push_back(cv::Point2d(0, 25));  // left
                    points.push_back(cv::Point2d(50, 25)); // right
                    break;
                }
                else if (corners(i) && !corners(next) && !corners(prec))
                {
                    pattern = PATTERN2D;
                    pattern2d = findPattern2dType(values, alpha);
                    patternNumber = i;
                    if (pattern2d = PATTERN2DP)
                    {
                        points.push_back(cv::Point2d(25, 0));  // top
                        points.push_back(cv::Point2d(50, 25)); // right
                        points.push_back(cv::Point2d(25, 50)); // bottom
                        points.push_back(cv::Point2d(0, 25));  // left
                    }
                    else if (pattern2d = PATTERN2DM)
                    {
                        points.push_back(cv::Point2d(25, 0));  // top
                        points.push_back(cv::Point2d(0, 25));  // left
                        points.push_back(cv::Point2d(25, 50)); // bottom
                        points.push_back(cv::Point2d(50, 25)); // right
                    }
                    else
                    {
                        points.push_back(cv::Point2d(0, 25));  // left
                        points.push_back(cv::Point2d(50, 25)); // right
                        points.push_back(cv::Point2d(25, 0));  // top
                        points.push_back(cv::Point2d(25, 50)); // bottom
                    }
                    break;
                }
            }
            break;
        };
    }

    PatternType pattern;
    Pattern2dType pattern2d;
    int patternNumber; // 0-3

    Eigen::Array<bool, 1, 4> corners;
    Eigen::Array<float, 1, 4> values;
    float alpha;

    std::vector<cv::Point2d> points;

    cv::Mat draw()
    {
        cv::Mat img = cv::Mat::zeros(50, 50, CV_8U);
        for (int i = 0; i < points.size(); i += 2)
        {
            cv::line(img, points[i], points[i + 1], cv::Scalar(255), 1);
        }
        switch (patternNumber)
        {
        case 0:
            break;
        case 1:
            cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
            break;
        case 2:
            cv::rotate(img, img, cv::ROTATE_180);
            break;
        case 3:
            cv::rotate(img, img, cv::ROTATE_90_COUNTERCLOCKWISE);
            break;
        }

        cv::circle(img, cv::Point2d(0, 0), 2, cv::Scalar(255), -1);
        cv::circle(img, cv::Point2d(50, 50), 2, cv::Scalar(255), -1);
        cv::circle(img, cv::Point2d(50, 0), 2, cv::Scalar(255), -1);
        cv::circle(img, cv::Point2d(0, 50), 2, cv::Scalar(255), -1);

        return img;
    }

    void linearInterpolation()
    {
        int prec = (patternNumber + 3) % 4;
        int next = (patternNumber + 1) % 4;
        int opp = (patternNumber + 2) % 4;
        switch (pattern)
        {
        case PATTERN1:
            points[0] = cv::Point2d(50 * (alpha - values(patternNumber)) / (values(next) - values(patternNumber)), 0); // top
            points[1] = cv::Point2d(0, 50 * (alpha - values(patternNumber)) / (values(prec) - values(patternNumber))); // left
            break;
        case PATTERN2S:
            if (patternNumber == 0 || patternNumber == 2)
            {
                points[0] = cv::Point2d(0, 50 * (alpha - values(patternNumber)) / (values(prec) - values(patternNumber))); // left
                points[1] = cv::Point2d(50, 50 * (alpha - values(next)) / (values(opp) - values(next)));                   // right
            }
            else
            {
                points[0] = cv::Point2d(50 * (alpha - values(patternNumber)) / (values(next) - values(patternNumber)), 0); // top
                points[1] = cv::Point2d(50 * (alpha - values(prec)) / (values(opp) - values(prec)), 50);                   // bottom
            }
            break;
        case PATTERN2D:
            if (pattern2d == PATTERN2DP)
            {
                points[0] = cv::Point2d(50 * (alpha - values(patternNumber)) / (values(next) - values(patternNumber)), 0); // top
                points[1] = cv::Point2d(50, 50 * (alpha - values(next)) / (values(opp) - values(next)));                   // right
                points[2] = cv::Point2d(50 * (alpha - values(prec)) / (values(opp) - values(prec)), 50);                   // bottom
                points[3] = cv::Point2d(0, 50 * (alpha - values(patternNumber)) / (values(prec) - values(patternNumber))); // left
            }
            else if (pattern2d == PATTERN2DM)
            {

                points[0] = cv::Point2d(50 * (alpha - values(patternNumber)) / (values(next) - values(patternNumber)), 0); // top
                points[1] = cv::Point2d(0, 50 * (alpha - values(patternNumber)) / (values(prec) - values(patternNumber))); // left
                points[2] = cv::Point2d(50 * (alpha - values(prec)) / (values(opp) - values(prec)), 50);                   // bottom
                points[3] = cv::Point2d(50, 50 * (alpha - values(next)) / (values(opp) - values(next)));                   // right
            }
            else
            {
                points[0] = cv::Point2d(0, 50 * (alpha - values(patternNumber)) / (values(prec) - values(patternNumber))); // left
                points[1] = cv::Point2d(50, 50 * (alpha - values(next)) / (values(opp) - values(next)));                   // right
                points[2] = cv::Point2d(50 * (alpha - values(patternNumber)) / (values(next) - values(patternNumber)), 0); // top
                points[3] = cv::Point2d(50 * (alpha - values(prec)) / (values(opp) - values(prec)), 50);                   // bottom
            }
            break;
        case PATTERN3:
            points[0] = cv::Point2d(50 * (alpha - values(patternNumber)) / (values(next) - values(patternNumber)), 0); // top
            points[1] = cv::Point2d(0, 50 * (alpha - values(patternNumber)) / (values(prec) - values(patternNumber))); // left
            break;
        };
    }

    std::string toString()
    {
        std::string str = "";
        switch (pattern)
        {
        case PATTERN0:
            str += "PATTERN0";
            break;
        case PATTERN1:
            str += "PATTERN1";
            break;
        case PATTERN2S:
            str += "PATTERN2S";
            break;
        case PATTERN2D:
            str += "PATTERN2D";
            break;
        case PATTERN3:
            str += "PATTERN3";
            break;
        case PATTERN4:
            str += "PATTERN4";
            break;
        }

        str += " ";

        switch (pattern2d)
        {
        case PATTERN2DP:
            str += "PATTERN2DP";
            break;
        case PATTERN2DM:
            str += "PATTERN2DM";
            break;
        case PATTERN2DE:
            str += "PATTERN2DE";
            break;
        }

        str += " ";

        str += std::to_string(patternNumber);

        str += "\n";

        for (auto &point : points)
        {
            str += "(" + std::to_string(point.x) + ", " + std::to_string(point.y) + ") ";
        }

        return str;
    }
};

int main(int, char **)
{
    cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);

    Eigen::Array44f grid;
    grid << 12, 14, 25, 20,
        20, 16, 20, 18,
        16, 22, 15, 17,
        24, 20, 13, 15;

    float alpha = 21;

    Eigen::Array<bool, 4, 4> mask;
    Eigen::Array<Pattern, 3, 3> patterns;

    mask = grid > alpha;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            Eigen::Array<bool, 1, 4> corners;
            Eigen::Array<float, 1, 4> values;

            corners << mask(i, j), mask(i, j + 1), mask(i + 1, j + 1), mask(i + 1, j);
            values << grid(i, j), grid(i, j + 1), grid(i + 1, j + 1), grid(i + 1, j);

            patterns(i, j) = Pattern(corners, values, alpha);
        }
    }

    std::cout << grid << std::endl;

    std::cout << mask << std::endl;

    for (int i = 0; i < 3; i++)
    {
        std::cout << std::endl;
        for (int j = 0; j < 3; j++)
        {
            std::cout << patterns(i, j).toString() << std::endl;
            cv::Mat img = patterns(i, j).draw();
            cv::imshow("Display window", img);
            cv::waitKey(1);
        }
    }

    cv::Mat entireImage = cv::Mat::zeros(150, 150, CV_8U);
    for (int i = 0; i < patterns.rows(); i++)
    {
        for (int j = 0; j < patterns.cols(); j++)
        {
            cv::Mat img = patterns(i, j).draw();
            img.copyTo(entireImage(cv::Rect(50 * j, 50 * i, 50, 50)));
        }
    }

    for (int i = 0; i < patterns.rows(); i++)
    {
        for (int j = 0; j < patterns.cols(); j++)
        {
            patterns(i, j).linearInterpolation();
        }
    }

    cv::Mat entireImageInterp = cv::Mat::zeros(150, 150, CV_8U);
    for (int i = 0; i < patterns.rows(); i++)
    {
        for (int j = 0; j < patterns.cols(); j++)
        {
            cv::Mat img = patterns(i, j).draw();
            img.copyTo(entireImageInterp(cv::Rect(50 * j, 50 * i, 50, 50)));
        }
    }

    cv::imshow("Display window", entireImage);
    cv::waitKey(0);

    cv::imshow("Display window", entireImageInterp);
    cv::waitKey(0);

    cv::imwrite("./images/4/results/WithoutInterpolation.png", entireImage);
    cv::imwrite("./images/4/results/WithInterpolation.png", entireImageInterp);

    cv::destroyAllWindows();
}
