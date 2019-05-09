#!/usr/bin/env python3
"""
    Copyright 2018 Steve Göring

    This file is part of SITI.
    SITI is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    SITI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with SITI. If not, see <http://www.gnu.org/licenses/>.
"""

import argparse
import sys
from abc import ABC, abstractmethod
from multiprocessing import Pool
import multiprocessing
from scipy import ndimage
import numpy as np
import pandas as pd
import skvideo.io


class Feature(ABC):
    def __init__(self):
        self._values = []

    @abstractmethod
    def calc(self, frame):
        pass

    @property
    def values(self):
        return self._values


class SiFeatures(Feature):
    def calc(self, frame):
        sobx = ndimage.sobel(frame, axis=0)
        soby = ndimage.sobel(frame, axis=1)
        value = np.hypot(sobx, soby).std()
        self._values.append(value)
        return value


class TiFeatures(Feature):
    def __init__(self):
        super().__init__()
        self._previous_frame = None

    def calc(self, frame):
        value = 0
        if self._previous_frame is not None:
            value = (frame - self._previous_frame).std()
        self._previous_frame = frame
        self._values.append(value)
        return value


def analyze_video(video):
    features = {
        "si": SiFeatures(),
        "ti": TiFeatures()
    }
    i = 0
    for frame in skvideo.io.vreader(video, as_grey=True):
        if len(frame.shape) > 2:
            width = frame.shape[-2]
            height = frame.shape[-3]
            frame = frame.reshape((height, width))

        frame = frame.astype('float32')

        print("frame {} of video {}".format(i, video))
        for feature in features:
            v = features[feature].calc(frame)
            print("{} -> {}".format(feature, v))
        i += 1

    result = {}
    for feature in features:
        result[feature] = features[feature].values
    df = pd.DataFrame(result)
    df["frame"] = range(len(df))
    df["video"] = video
    return df


def main(_):
    # argument parsing
    parser = argparse.ArgumentParser(description='si ti calculation',
                                     epilog="stg7 2018",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("video", type=str, nargs="+", help="video to analyze")
    parser.add_argument("--output_file", type=str, default="siti.csv", help="output file for si ti report")
    parser.add_argument('--cpu_count', type=int, default=multiprocessing.cpu_count(), help='thread/cpu count')

    a = vars(parser.parse_args())
    print("calculate si/ti for {} videos".format(len(a["video"])))

    pool = Pool(processes=a["cpu_count"])
    results = pool.map(analyze_video, a["video"])
    all_df = pd.DataFrame()
    for x in results:
        all_df = all_df.append(x, ignore_index=True)
    all_df.to_csv(a["output_file"], index=False)
    print("done")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
