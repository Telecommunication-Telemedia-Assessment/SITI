# alternate implementation in python without opencv dependency

**important** it's not checked if you will get the same results as with the c++ version

## requirements
* python3, with the following packages
    * numpy, pandas, skvideo, scipy

## usage
see `./siti.py --help`:
```
usage: siti.py [-h] [--output_file OUTPUT_FILE] [--cpu_count CPU_COUNT]
               video [video ...]

si ti calculation

positional arguments:
  video                 video to analyze

optional arguments:
  -h, --help            show this help message and exit
  --output_file OUTPUT_FILE
                        output file for si ti report (default: siti.csv)
  --cpu_count CPU_COUNT
                        thread/cpu count (default: 8)

stg7 2018
```
