// SITI
// main.cpp
//
// Copyright (c) 2016 Pierre Lebreton
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <fstream>
#include <vector>
#include <functional> // c++11 feature

#include "cxxopts.hpp"  // cli argument parsing


#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

typedef std::tuple<int, double, double> TSTD;


//#define USE_OCL 1

#ifdef USE_OCL
	typedef cv::UMat cMat;
#else
	typedef cv::Mat cMat;
#endif


bool readYUV420(cv::Mat &mat, FILE *vf, unsigned char *buffer) {
	// -----------------------------------------------------------
	// Pix Format: YUV420p

	if(vf == NULL) return false;
	if(fread(mat.data, 1, mat.cols*mat.rows, vf) != (mat.cols*mat.rows)) {
		fclose(vf);
		vf = NULL;
		return false;
	}

	if(fread(buffer, 1, mat.cols*mat.rows/2, vf) != (mat.cols*mat.rows/2)) {
		return false;
	}


	return true;
}


bool readYUV422(cv::Mat &mat, FILE *vf, unsigned char *buffer) {

	// -----------------------------------------------------------
	// Pix Format: YUV422p

	if(vf == NULL) return false;
	if(fread(mat.data, 1, mat.cols*mat.rows, vf) != (mat.cols*mat.rows)) {
		fclose(vf);
		vf = NULL;
		return false;
	}

	for(int i = 0 ; i < 2 ; ++i) {
		if(fread(buffer, 1, mat.cols*mat.rows/2, vf) != (mat.cols*mat.rows/2)) {
			return false;
		}
	}


	return true;
}

bool readYUYV422(cv::Mat &mat, FILE *vf, unsigned char *buffer) {

	// -----------------------------------------------------------
	// Pix Format: YUYV422

	if(vf == NULL) return false;
	unsigned char buf[4];
	for(int i = 0 ; i < mat.cols*mat.rows / 2 ; ++i) {
		if(fread(buf, 1, 4, vf) != 4) {
			fclose(vf);
			vf = NULL;
			return false;
		}
		mat.data[2*i] = buf[0];
		mat.data[2*i+1] = buf[2];
	}


	return true;
}


bool readUYVY422(cv::Mat &mat, FILE *vf, unsigned char *buffer) {

	// -----------------------------------------------------------
	// Pix Format: YUYV422

	if(vf == NULL) return false;
	unsigned char buf[4];
	for(int i = 0 ; i < mat.cols*mat.rows / 2 ; ++i) {
		if(fread(buf, 1, 4, vf) != 4) {
			fclose(vf);
			vf = NULL;
			return false;
		}
		mat.data[2*i] = buf[1];
		mat.data[2*i+1] = buf[3];
	}


	return true;
}

bool readYUV444(cv::Mat &mat, FILE *vf, unsigned char *buffer) {

	// -----------------------------------------------------------
	// Pix Format: YUV444p

	if(vf == NULL) return false;
	if(fread(mat.data, 1, mat.cols*mat.rows, vf) != (mat.cols*mat.rows)) {
		fclose(vf);
		vf = NULL;
		return false;
	}

	for(int i = 0 ; i < 4 ; ++i) {
		if(fread(buffer, 1, mat.cols*mat.rows/2, vf) != (mat.cols*mat.rows/2)) {
			return false;
		}
	}


	return true;
}

#define POS(i,j)					(i)*width+(j)

bool grabFrame(cv::Mat &mat, cv::VideoCapture &capture) {

	cv::Mat colorFrame;
	capture >> colorFrame;

	if(!colorFrame.empty())
		cv::cvtColor(colorFrame, mat, cv::COLOR_BGR2GRAY);


	return !colorFrame.empty();
}


double computeSI(const cMat& frame1, const cMat &maskValidSobel) {
	cMat gx, gy, grad;
	cv::Sobel(frame1, gx, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
	cv::Sobel(frame1, gy, CV_32F, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT);
	cv::add(gx.mul(gx), gy.mul(gy), grad);
	cv::sqrt(grad, grad);

	cv::Scalar meanSp, stddevSp;
	cv::meanStdDev(grad, meanSp, stddevSp, maskValidSobel);

	return stddevSp.val[0];
}



int main(int argc, char **argv) {

	// --------------------------------------------------------------------
	// parsing parameters...

	cxxopts::Options options(argv[0], "SI / TI Calculation Tool\nAssessment of IP-Based Applications, Telekom Innovation Laboratories, TU Berlin\nErnst-Reuter-Platz 7, 10587 Berlin, Germany\n");
    options.add_options()
        ("i,input-file", "input: images (BMP, TIFF, PNG, JPEG...), video (.avi, .mkv, .mp4, .yuv... )", cxxopts::value<std::string>())
        ("w,width", "Width of the video (required for yuv).", cxxopts::value<int>())
        ("h,height", "Height of the video (required for yuv).", cxxopts::value<int>())
        ("f,color-format", "(int) Color representation of YUV format (1: YUV420p (default), 2: YUV422, 3: YUYV422 (YUY2), 4: YUYV422 (UYVY), 5: YUV444.", cxxopts::value<int>())
        ("s,summary", "produce summary statistics (maximum, minimum) instead of per-frame information")
        ("c,check-input", "Show in a window how frames are read");

    try {
        options.parse(argc, argv);
    } catch (...) {
        std::cerr << options.help() << std::endl;
        return -1;
    }

	std::function<bool (cv::Mat &mat)> readYUV;
	FILE *f = NULL;
	cv::VideoCapture capture;

	int width       = 0;
	int height      = 0;
	bool summary    = false;
	bool check      = false;
	int colorFormat = 1;
	int frameCount = 0;

	if(options.count("height")) {
		height = options["height"].as<int>();
	}

	if(options.count("width")) {
		width = options["width"].as<int>();
	}

	if(options.count("summary")) {
		summary = true;
	}

	if(options.count("check-input")) {
		check = true;
	}

	if(options.count("color-format")) {
		colorFormat = options["color-format"].as<int>();
		if(colorFormat < 1 || colorFormat > 5) {
			std::cerr << "Color format should be between 1-5, see --help\n";
			return -1;
		}
	}


	std::vector<unsigned char> buffer(width * height / 2);


	if(options.count("input-file")) {
		const std::string &input = options["input-file"].as<std::string>();

		if(input.at(input.length()-3) == 'y' && input.at(input.length()-2) == 'u' && input.at(input.length()-1) == 'v') {
			if(height == 0 || width == 0) {
				std::cerr << "a YUV file requires both height and width to be set. See --help \n";
				return -1;
			}

			f = fopen(input.c_str(), "rb");
			if(f == NULL) {
				std::cerr<< "cannot open: " << input << std::endl;
				return -1;
			}

			switch(colorFormat) {
				case 1:
					readYUV = std::bind(readYUV420, std::placeholders::_1, f, &buffer[0]);
					break;

				case 2:
					readYUV = std::bind(readYUV422, std::placeholders::_1, f, &buffer[0]);
					break;

				case 3:
					readYUV = std::bind(readYUYV422, std::placeholders::_1, f, &buffer[0]);
					break;

				case 4:
					readYUV = std::bind(readUYVY422, std::placeholders::_1, f, &buffer[0]);
					break;

				case 5:
					readYUV = std::bind(readYUV444, std::placeholders::_1, f, &buffer[0]);
					break;

				default:
					std::cerr << "Unknown color format\n";
					return -1;

			}


		} else {
			capture.open(input);
			if(!capture.isOpened()) {
				std::cerr<< "cannot open: " << input << std::endl;
				return -1;
			}

			readYUV = std::bind(grabFrame, std::placeholders::_1, std::ref(capture));

			width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
			height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);

		}
	}

	if(readYUV == NULL) {
		std::cerr << "Please specify an input file. See --help... \n";
		return -1;
	}

	if(height == 0 || width == 0) {
		std::cerr << "Something happend, both width/height cannot be found... \n";
		return -1;
	}


	// --------------------------------------------------------------------
	// estimate SI/TI per frame...

	cv::Mat frame1(height, width, CV_8UC1);
	cv::Mat frame2(height, width, CV_8UC1);

	cMat	uframe1, uframe2;
	cMat maskValidSobel(frame1.size(), CV_8UC1);
	maskValidSobel(cv::Rect(1,1,width-1,height-1)) = 1;


	double maxSI = std::numeric_limits<double>::min();
	double maxTI = std::numeric_limits<double>::min();
	double minSI = std::numeric_limits<double>::max();
	double minTI = std::numeric_limits<double>::max();

	// write header
	std::cerr << "frameCount,SI,TI" << std::endl;

	readYUV(frame1);
	frame1.convertTo(uframe1, CV_32FC1);

	while(readYUV(frame2)) {
		frameCount++;
		frame2.copyTo(uframe2);
		uframe2.convertTo(uframe2, CV_32FC1);

		if(check) {
			cv::Mat viz;
			uframe2.copyTo(viz);
			cv::imshow("frame", viz / 255.);
			cv::waitKey(10);
		}


		double si = computeSI(uframe1, maskValidSobel);

		maxSI = std::max(maxSI, si);
		minSI = std::min(minSI, si);

		cMat dt;
		cv::subtract(uframe1, uframe2, dt);

		cv::Scalar meanT, stddevT;
		cv::meanStdDev(dt, meanT, stddevT, maskValidSobel);

		maxTI = std::max(maxTI, stddevT.val[0]);
		minTI = std::min(minTI, stddevT.val[0]);

		if (!summary) {
			std::cerr << frameCount << "," << si << "," << stddevT.val[0] << std::endl;
		}

		std::swap(uframe1, uframe2);
	}

	if(!uframe1.empty()) {
		double si = computeSI(uframe1, maskValidSobel);

		frameCount++;

		maxSI = std::max(maxSI, si);
		minSI = std::min(minSI, si);

		if (!summary) {
			std::cerr << frameCount << "," << si << ","  << std::endl;
		}
	}



	if (summary) {
		std::cerr << "maxSI: " << maxSI << std::endl;
		std::cerr << "maxTI: " << maxTI << std::endl;
		std::cerr << "minSI: " << minSI << std::endl;
		std::cerr << "minTI: " << minTI << std::endl;
	}

	return 0;
}

