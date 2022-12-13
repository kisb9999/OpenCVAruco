#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core_detect.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

bool readCameraParameters(string filename, Mat& cameraMatrix, Mat& distortion_coefficients) {
    FileStorage filestorage(filename, FileStorage::READ);
    if (!filestorage.isOpened())
        return false;

    filestorage["camera_matrix"] >> cameraMatrix;
    filestorage["distortion_coefficients"] >> distortion_coefficients;

    return true;
}

double Calibrate(Mat cameraMtx, Mat distortionCoffeicient) {
    Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    Ptr<aruco::GridBoard> board = aruco::GridBoard::create(5, 7, 0.026, 0.003, dictionary);
    Size imgSize = Size(0.026, 0.026);

    vector<vector<Point2f>> corners;
    vector<int> ids, markerCtr;
    //Mat cameraMtx, distortionCoffeicient;
    vector<Mat> rotationVector, translationVector;
    int calibrationFlags = 0;

    return aruco::calibrateCameraAruco(corners, ids, markerCtr, board, imgSize, cameraMtx, distortionCoffeicient, rotationVector, translationVector, calibrationFlags);
}


int main(){
    //Marker creation for all directions + destination
    /*
    Mat markerImage;
    //Mat markerImage = imread("D:\\aruco_example_board.png");
    Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    //up
    cv::aruco::drawMarker(dictionary, 1, 200, markerImage, 1);
    imwrite("marker1.png", markerImage);
    //down
    cv::aruco::drawMarker(dictionary, 2, 250, markerImage, 1);
    imwrite("marker2.png", markerImage);
    //left
    cv::aruco::drawMarker(dictionary, 3, 200, markerImage, 1);
    imwrite("marker3.png", markerImage);
    //right
    cv::aruco::drawMarker(dictionary, 4, 200, markerImage, 1);
    imwrite("marker4.png", markerImage);
    //destination
    cv::aruco::drawMarker(dictionary, 0, 200, markerImage, 1);
    imwrite("marker0.png", markerImage);
    //imshow("Out1", markerImage);*/


    //Starting video capture
    cv::VideoCapture videocapture;
    videocapture.open(0, cv::CAP_DSHOW);
    

    //Camera calibration from the calib.yml file
    Mat cameraMatrix;
    Mat distortionCoefficient;
    string file = "D:\\BME\\OpenCVProjects\\OpenCVDebugVersion\\calib.yaml";
    bool successful = readCameraParameters(file, cameraMatrix, distortionCoefficient);

    if (!successful)
        throw new exception("Calibration unsuccessful");

    Mat img;
    Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    //Ptr<aruco::DetectorParameters> param = aruco::DetectorParameters().create();

    namedWindow("Out", 0);

    if (!videocapture.isOpened())
        throw new exception("Videocapture is not working");

    //Basically an infinite loop that runs until we do not stop the program by pressing ESC
    while (videocapture.grab()) {
        Mat outImg;
        videocapture >> img;
        img.copyTo(outImg);

        vector<vector<Point2f>> corners;
        vector<int> ids;
        
        //Fill corners and ids with data
        aruco::detectMarkers(img, dictionary, corners, ids);

        //If we have detected at least one marker
        if (ids.size() > 0) {
            aruco::drawDetectedMarkers(outImg, corners, ids);
            
            vector<Vec3d> rotationVectors, translationVectors;

            aruco::estimatePoseSingleMarkers(corners, 0.026, cameraMatrix, distortionCoefficient, rotationVectors, translationVectors);
            
            //Printing commands to console based on the id of the last marker that was detected
            switch (ids[(int)ids.size()-1]) {
            case 0:
                cout << "You have successfully arrived to your destination!\n";
                break;
            case 1:
                cout << "Go upwards!\n";
                break;
            case 2:
                cout << "Go downwards!\n";
                break;
            case 3:
                cout << "Go left!\n";
                break;
            case 4:
                cout << "Go right!\n";
                break;
            default:
                break;
            }


            //aruco::drawAxis(outImg, cameraMatrix, distortionCoefficient, rvecs, tvecs, 0.026);
            for (int i = 0; i < (int)ids.size(); i++) {
                drawFrameAxes(outImg, cameraMatrix, distortionCoefficient, rotationVectors[i], translationVectors[i], 0.026);
                
                String str = "Dist: ";
                double distance = sqrt(pow(translationVectors[i][0], 2) + pow(translationVectors[i][1], 2) + pow(translationVectors[i][2], 2));
                
                //Printing values of the tranlastion vectors
                //cout << "x value squared: " << translationVectors[i][0] << "\n";
                //cout << "y value squared: " << translationVectors[i][1] << "\n";
                //cout << "z value squared: " << translationVectors[i][2] << "\n";
                //cout << "distance: " << distance << "\n";
                //cout << "\n";

                str += to_string(distance*100);
                putText(outImg, str, corners[i][0], FONT_HERSHEY_COMPLEX, 0.5, (100, 200, 100), 2, LINE_AA);

                if (distance*100 > 10)
                    cout << "Too far away from marker, get closer!\n";
                if (distance*100 < 5 )
                    cout << "Too close to the marker, get back!\n";
            }
        }

        cv::imshow("Out", outImg);
        int key = waitKey(4);
        if (key == 27)
            break;

    }

    videocapture.release();
    cv::destroyAllWindows();
    return 0;
}

