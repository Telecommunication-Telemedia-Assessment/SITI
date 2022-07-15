# SITI

A command-line-based tool for Windows to calculate spatial information (SI) and temporal information (TI) according to ITU-T P.910.

**:warning: This tool uses a legacy computation for SI/TI and is no longer being maintained. Please see https://github.com/VQEG/siti-tools for a reference implementation for SI/TI according to the latest version of ITU-T P.910.**

## Usage

Use `bin/SITI.exe` to run the program under Windows. For Unix, please build it yourself.

The following command line options are available:

    --help                    produce help message
    -i [ --input-file ] arg   input: images (BMP, TIFF, PNG, JPEG...), video
                              (.avi, .mkv, .mp4, .yuv... )
    -w [ --width ] arg        Width of the video (required for yuv).
    -h [ --height ] arg       Height of the video (required for yuv).
    -f [ --color-format ] arg (int) Color representation of YUV format (1:
                              YUV420p (default), 2: YUV422, 3: YUYV422 (YUY2), 4:
                              YUYV422 (UYVY), 5: YUV444
    -s [ --summary ]          produce summary statistics (maximum, minimum)
                              instead of per-frame information
    -c [ --check-input ]      Show in a window how frames are read

The output will be a comma-separated list of frame number, SI and TI values.

## Acknowledgement

If you use this program for your research, please provide a link to this repository when publishing your results.

## Building from source

### Windows

Requirements:

- Windows
- OpenCV
- Visual Studio

Open `src/SITI.sln` for building the project in Visual Studio.

### Linux / Unix / macOS

Requirements:

- [Scons](http://www.scons.org/pages/download.html)
    - Ubuntu / Debian: `apt-get install scons`
    - macOS: `brew install scons`
- OpenCV
    - Ubuntu / Debian: `apt-get install libopencv-dev`
    - macOS: `brew install opencv`
- `gcc` / compiler with C++11 support
    - Ubuntu / Debian: `apt-get install build-essential`
    - macOS: through XCode

To build the program, go to `src/SITI/` and run `scons`. The result is a `siti` executable in the same folder.

## Background

ITU-T Recommendation P.910 ("Subjective video quality assessment methods for multimedia applications") specifies two metrics that can be used to classify content.

### Spatial Information

> The spatial perceptual information (SI) is based on the Sobel filter. Each video frame (luminance plane) at time n (Fn) is first filtered with the Sobel filter [Sobel(Fn)]. The standard deviation over the pixels (stdspace) in each Sobel-filtered frame is then computed. This operation is repeated for each frame in the video sequence and results in a time series of spatial information of the scene. The maximum value in the time series (maxtime) is chosen to represent the spatial information content of the scene.

### Temporal information

> The temporal perceptual information (TI) is based upon the motion difference feature, Mn(i, j), which is the difference between the pixel values (of the luminance plane) at the same location in space but at successive times or frames. Mn(i, j) as a function of time (n) is defined as:

> ![](http://i.imgur.com/MRsJtdT.png)

> here Fn(i, j) is the pixel at the ith row and jth column of nth frame in time.
The measure of temporal information (TI) is computed as the maximum over time (maxtime) of the standard deviation over space (stdspace) of Mn(i, j) over all i and j.

> ![](http://i.imgur.com/zRXcVJO.png)

> More motion in adjacent frames will result in higher values of TI

## Authors

- Pierre Lebreton
- Werner Robitza
- Steve Göring

## License

GNU General Public License v3

Copyright (c) 2016-2018 Pierre Lebreton, Werner Robitza, Steve Göring

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/.
