#!/usr/bin/env python3
import argparse
import sys
import os
import json
from multiprocessing import Pool
import multiprocessing

import numpy as np
import pandas as pd
import skvideo.io
import scipy


class Feature:
    def __init__(self):
        self._values = []

    def calc(self, frame):
        raise NotImplementedError("not implemented")

    def get_values(self):
        return self._values


class SiFeatures(Feature):
    def __init__(self):
        self._values = []

    def calc(self, frame):
        from scipy import ndimage
        value = ndimage.sobel(frame).std()
        self._values.append(value)
        return value


class TiFeatures(Feature):
    def __init__(self):
        self._values = []
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
    for frame in skvideo.io.vreader(video):
        print("frame {} of video {}".format(i, video))
        for feature in features:
            v = features[feature].calc(frame)
            print("{} -> {}".format(feature, v))
        i += 1

    result = {}
    for feature in features:
        result[feature] = features[feature].get_values()
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

