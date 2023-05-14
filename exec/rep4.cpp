#include <iostream>
#include <vector>
#include <string>

#include "opencv2/opencv.hpp"
#include "Eigen/Core"

// Same notation as in the course, the final interger is the number of "+" in the neighbourhood
enum PatternType
{
    PATTERN0,
    PATTERN1,
    PATTERN2S,
    PATTERN2D,
    PATTERN3,
    PATTERN4
};

// Results of the asymptotic decider diven the sign of the determinant
enum Pattern2dType
{
    PATTERN2DP, // Positive
    PATTERN2DM, // Negative
    PATTERN2DE, // Equal to zero

};

// Find the pattern type of a 2D configuration given the values of the neighbourhood by computing the determinant D
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

// Find the pattern of a 2D scalar field and provides some methods to draw it
class Pattern
{
public:
    // Constuctor for Eigen array initilaisation
    Pattern()
    {
        pattern = PATTERN0;
        pattern2d = PATTERN2DE;
        patternNumber = 0;
    }

    // Constructor for a given configuration
    // c: corners from the mask matrix
    // v: values from the scalar field
    // a: alpha
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
            pattern2d = PATTERN2DE; // default, not used later
            patternNumber = 0;      // one configuration is possible
            break;

        case 1:
            pattern = PATTERN1;
            pattern2d = PATTERN2DE; // default, not used later

            // Find the index of the corner, since the corners are clockwise ordered, the index is the pattern number
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
            pattern2d = PATTERN2DE; // default, not used later

            // Find the index of the corner, since the corners are clockwise ordered, the index is the pattern number (same as PATTERN1)
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
            pattern2d = PATTERN2DE; // default, not used later
            patternNumber = 0;      // one configuration is possible
            break;

        case 2:
            for (int i = 0; i < 4; i++)
            {
                int next = (i + 1) % 4; // next corner (clockwise)
                int prec = (i + 3) % 4; // previous corner (clockwise)

                if (corners(i) && corners(next))
                {
                    pattern = PATTERN2S;
                    pattern2d = PATTERN2DP;                // default, not used later
                    patternNumber = i;                     // four configuration possible, the index is the pattern number (the first + followed by another +)
                    points.push_back(cv::Point2d(0, 25));  // left
                    points.push_back(cv::Point2d(50, 25)); // right
                    break;
                }
                else if (corners(i) && !corners(next) && !corners(prec))
                {
                    pattern = PATTERN2D;
                    pattern2d = findPattern2dType(values, alpha);
                    patternNumber = i; // two configuration possible in theory, but four here because of the symmetry, it does not affect the result

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

public:
    PatternType pattern;
    Pattern2dType pattern2d;
    int patternNumber; // pattern rotation (possible configuration for each pattern)

    // Neighbourhood
    Eigen::Array<bool, 1, 4> corners; // corners from the mask matrix
    Eigen::Array<float, 1, 4> values; // values from the scalar field
    float alpha;

    std::vector<cv::Point2d> points; // points to draw the pattern, the points are the coordinates as if the pattern was not rotated (in its default configuration)

    // Draw the pattern, just create an square image and draw the lines, and rotate given the configuration
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

        // Draw the corners so that it is easier to see the pattern
        cv::circle(img, cv::Point2d(0, 0), 2, cv::Scalar(255), -1);
        cv::circle(img, cv::Point2d(50, 50), 2, cv::Scalar(255), -1);
        cv::circle(img, cv::Point2d(50, 0), 2, cv::Scalar(255), -1);
        cv::circle(img, cv::Point2d(0, 50), 2, cv::Scalar(255), -1);

        return img;
    }

    // Interpolate the pattern, given the alpha value and the values of the scalar field.
    // The points are the coordinates as if the pattern was not rotated (in its default configuration)
    void linearInterpolation()
    {
        int prec = (patternNumber + 3) % 4; // previous corner (clockwise)
        int next = (patternNumber + 1) % 4; // next corner (clockwise)
        int opp = (patternNumber + 2) % 4;  // opposite corner

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

    // Method to print the pattern for debugging purposes
    std::string toString()
    {
        std::string str = "";
        switch (pattern)
        {
        case PATTERN0:
            str += "Pattern Number  = 0";
            break;
        case PATTERN1:
            str += "Pattern Number  = 1";
            break;
        case PATTERN2S:
            str += "Pattern Number  = 2S";
            break;
        case PATTERN2D:
            str += "Pattern Number  = 2D";
            break;
        case PATTERN3:
            str += "Pattern Number  = 3";
            break;
        case PATTERN4:
            str += "Pattern Number  = 4";
            break;
        }

        str += " ";

        if (pattern == PATTERN2D)
        {

            switch (pattern2d)
            {
            case PATTERN2DP:
                str += "D>0";
                break;
            case PATTERN2DM:
                str += "D<0";
                break;
            case PATTERN2DE:
                str += "D=0";
                break;
            }
        }
        str += " Configuration = ";

        str += std::to_string(patternNumber);

        str += "\n";
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

    // Create the mask of boolean values
    mask = grid > alpha;

    // Create the patterns for each square between points of the scalar field
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

    // Print the patterns
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

    // Fuse the images of each pattern into one
    cv::Mat entireImage = cv::Mat::zeros(150, 150, CV_8U);
    for (int i = 0; i < patterns.rows(); i++)
    {
        for (int j = 0; j < patterns.cols(); j++)
        {
            cv::Mat img = patterns(i, j).draw();
            img.copyTo(entireImage(cv::Rect(50 * j, 50 * i, 50, 50)));
        }
    }

    // Interpolation on each pattern
    for (int i = 0; i < patterns.rows(); i++)
    {
        for (int j = 0; j < patterns.cols(); j++)
        {
            patterns(i, j).linearInterpolation();
        }
    }

    // Fuse the images of each pattern into one
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
