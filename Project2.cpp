#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;


//////////// images ////////////

Mat imgOriginal, imgGray, imgCanny, imgThre, imgBlur, imgDil, imgErode, imgWarp, imgCrop;
vector<Point> initialPoints, docPoints;

float w = 420, h = 596;

Mat preProcessing(Mat img)
{
	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);
	Canny(imgBlur, imgCanny, 25, 75);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, kernel);
	/*erode(imgDil, imgErode, kernel);*/
	return imgDil;
}

vector<Point> getContours(Mat imgDil) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(imgDil, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	/*drawContours(img, contours, -1, Scalar(255, 0, 255), 2);*/

	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());

	vector<Point>biggest;

	int maxarea=0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);
		cout << area << endl;

		string objectType;
		//vector<vector<Point>> conPoly;
		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);

			if(area>maxarea && conPoly[i].size()==4)
			{	
				/*drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 5);*/
				biggest = { conPoly[i][0],conPoly[i][1],conPoly[i][2],conPoly[i][3] };
				maxarea = area;
			}
			
			/*rectangle(imgOriginal, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);*/


		}
	}
	return biggest;
}

void drawPoints(vector<Point> points,Scalar color) 
{
	for (int i = 0;i<points.size(); i++)
	{
		circle(imgOriginal,points[i],10,color,FILLED);
		putText(imgOriginal, to_string(i), points[i], FONT_HERSHEY_PLAIN, 4, color,4 );
	}
}

vector<Point> reorder(vector<Point> points)
{
	vector<Point> newPoints;
	vector<int>	sumPoints, subPoints;

	for (int i = 0; i < 4; i++)
	{
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);

	}

	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);//0
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);//3
	return newPoints;
}

Mat getWarp(Mat img, vector<Point> points, float w, float h)
{
	
	Point2f src[4] = { points[0],points[1],points[2],points[3] };
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));

	return imgWarp;

}

void main() {
	string path = "Resources/paper.jpg";
	imgOriginal = imread(path);
	/*resize(imgOriginal, imgOriginal, Size(), 0.5, 0.5);*/

	//Preprocessing

	imgThre=preProcessing(imgOriginal);

	//Get Contours - Biggest

	initialPoints=getContours(imgThre);
	

	docPoints=reorder(initialPoints);
	/*drawPoints(docPoints, Scalar(0, 255, 0));*/
	//Warp

	imgWarp = getWarp(imgOriginal, docPoints, w, h);
	//Crop

	int cropVal = 5;
	Rect roi(cropVal, cropVal, w - (2 * cropVal), h - (2 * cropVal));
	imgCrop = imgWarp(roi);

	imshow("Image", imgOriginal);
	imshow("imgDil", imgDil);
	imshow("imgWarp", imgWarp);
	imshow("img", imgCrop);

	waitKey(0);

}