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

#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/program_options.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

typedef boost::tuple<int, double, double> TSTD;

void add(TSTD &tuple, double v) {
	++tuple.get<0>();
	tuple.get<1>() += v;
	tuple.get<2>() += v*v;
}

double stdev(const TSTD &tuple) {
	return sqrt((tuple.get<2>() / tuple.get<0>()) - (tuple.get<1>() / tuple.get<0>())*(tuple.get<1>() / tuple.get<0>()));
}

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
		cv::cvtColor(colorFrame, mat, CV_BGR2GRAY);

	return !colorFrame.empty();
}

int main(int argc, char **argv) {

	// --------------------------------------------------------------------
	// parsing parameters...

	namespace po = boost::program_options;

	po::positional_options_description p;
	p.add("input-file", -1);

	po::options_description desc("Allowed options");
	desc.add_options()
			("help", "produce help message")
			("input-file,i", po::value<std::string>(), "input: images (BMP, TIFF, PNG, JPEG...), video (.avi, .mkv, .mp4, .yuv... )")
			("width,w", po::value< int >(), "Width of the video (required for yuv).")
			("height,h", po::value< int >(), "Height of the video (required for yuv).")
			("color-format,f", po::value< int >(), "(int) Color representation of YUV format (1: YUV420p (default), 2: YUV422, 3: YUYV422 (YUY2), 4: YUYV422 (UYVY), 5: YUV444")
			("summary,s", "produce summary statistics (maximum, minimum) instead of per-frame information")
			("check-input,c", "Show in a window how frames are read")
	;

	po::variables_map cli_arguments;
	try {
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), cli_arguments);
		po::notify(cli_arguments);
	} catch(boost::exception &) {
		std::cerr << "Error incorect program options. See --help... \n";
		return 0;
	}


	if (cli_arguments.count("help")) {
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		std::cout << "SI / TI Calculation Tool" << std::endl << std::endl;
		std::cout << "Assessment of IP-Based Applications, Telekom Innovation Laboratories, TU Berlin" << std::endl;
		std::cout << "Ernst-Reuter-Platz 7, 10587 Berlin, Germany" << std::endl;
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
		std::cout << std::endl << std::endl;
		std::cout << desc << std::endl;
		return -1;
	}

	boost::function<bool (cv::Mat &mat)> readYUV;
	FILE *f = NULL;
	cv::VideoCapture capture;

	int width       = 0;
	int height      = 0;
	bool summary    = false;
	bool check      = false;
	int colorFormat = 1;
	int frameCount = 0;

	if(cli_arguments.count("height")) {
		height = cli_arguments["height"].as<int>();
	}

	if(cli_arguments.count("width")) {
		width = cli_arguments["width"].as<int>();
	}

	if(cli_arguments.count("summary")) {
		summary = true;
	}

	if(cli_arguments.count("check-input")) {
		check = true;
	}

	if(cli_arguments.count("color-format")) {
		colorFormat = cli_arguments["color-format"].as<int>();
		if(colorFormat < 1 || colorFormat > 5) {
			std::cerr << "Color format should be between 1-5, see --help\n";
			return -1;
		}
	}


	std::vector<unsigned char> buffer(width*height/2);



	if(cli_arguments.count("input-file")) {
		const std::string &input = cli_arguments["input-file"].as<std::string>();

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
					readYUV = boost::bind(readYUV420, _1, f, &buffer[0]);
					break;

				case 2:
					readYUV = boost::bind(readYUV422, _1, f, &buffer[0]);
					break;

				case 3:
					readYUV = boost::bind(readYUYV422, _1, f, &buffer[0]);
					break;

				case 4:
					readYUV = boost::bind(readUYVY422, _1, f, &buffer[0]);
					break;

				case 5:
					readYUV = boost::bind(readYUV444, _1, f, &buffer[0]);
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

			readYUV = boost::bind(grabFrame, _1, boost::ref(capture));

			width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
			height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

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


	cv::Mat frame1(height, width, CV_8U);
	cv::Mat frame2(height, width, CV_8U);


	double maxSI = std::numeric_limits<double>::min();
	double maxTI = std::numeric_limits<double>::min();
	double minSI = std::numeric_limits<double>::max();
	double minTI = std::numeric_limits<double>::max();

	// write header
	std::cout << "frameCount,SI,TI" << std::endl;

	readYUV(frame1);
	while(readYUV(frame2)) {
		frameCount++;

		if(check) {
			cv::imshow("frame", frame2);
			cv::waitKey(10);
		}

		TSTD si;
		TSTD ti;

		for(int i = 1 ; i < height-1 ; ++i) {
			for(int j = 1 ; j < width-1 ; ++j) {
				double gx = -  static_cast<double>(frame1.data[POS(i-1,j-1)])
						   -2*static_cast<double>(frame1.data[POS(i-1,j)])
						   -  static_cast<double>(frame1.data[POS(i,j+1)])
						   +  static_cast<double>(frame1.data[POS(i+1,j-1)])
						   +2*static_cast<double>(frame1.data[POS(i+1,j)])
						   +  static_cast<double>(frame1.data[POS(i+1,j+1)]);

				double gy = -  static_cast<double>(frame1.data[POS(i-1,j-1)])
						   -2*static_cast<double>(frame1.data[POS(i,j-1)])
						   -  static_cast<double>(frame1.data[POS(i+1,j-1)])
						   +  static_cast<double>(frame1.data[POS(i-1,j+1)])
						   +2*static_cast<double>(frame1.data[POS(i,j+1)])
						   +  static_cast<double>(frame1.data[POS(i+1,j+1)]);

				double g = std::sqrt(gx*gx+gy*gy);

				add(si, g);
				add(ti, static_cast<double>(frame1.data[POS(i,j)])-static_cast<double>(frame2.data[POS(i,j)]));
			}
		}

		if (!summary) {
			std::cout << frameCount << "," << stdev(si) << "," << stdev(ti) << std::endl;
		}

		maxSI = std::max(maxSI, stdev(si));
		maxTI = std::max(maxTI, stdev(ti));
		minSI = std::min(minSI, stdev(si));
		minTI = std::min(minTI, stdev(ti));

		std::swap(frame1, frame2);
	}

	if (summary) {
		std::cout << "maxSI: " << maxSI << std::endl;
		std::cout << "maxTI: " << maxTI << std::endl;
		std::cout << "minSI: " << minSI << std::endl;
		std::cout << "minTI: " << minTI << std::endl;
	}

	return 0;
}

