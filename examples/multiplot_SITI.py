"""
@author: stg7
"""
import pandas as pd
import glob
from argparse import ArgumentParser

parser = ArgumentParser(description="This tool plots SI/TI values.")
parser.add_argument("-order_srcs", dest="order_srcs", default=False, action='store_true',
                    help="Add this parameter, if you want to enable alphabetical ordering in the legend"
                         " and display it above the plot.")
args = parser.parse_args()

videos_list = []
df = pd.DataFrame()
for x in list(glob.glob("*.csv")):
    tmp = pd.read_csv(x, delimiter=",")
    tmp["SRC"] = x.replace(".csv", "")
    df = df.append(tmp, ignore_index=True)

if args.order_srcs:
    hue_order = []
    for i in range(1, len(glob.glob("*.csv")) + 1):
        hue_order.append(str(i))

import seaborn as sns
import matplotlib.pyplot as plt

sns.set_style("white")
if args.order_srcs:
    ax = sns.lmplot(x="SI", y="TI", hue="SRC", data=df, fit_reg=False, legend=False, hue_order=hue_order)
    plt.legend(bbox_to_anchor=(0, 1.02, 1, 0.2), loc="lower left", mode="expand", borderaxespad=0, ncol=8)
else:
    ax = sns.lmplot(x="SI", y="TI", hue="SRC", data=df, fit_reg=False)
# ax.set(alpha=0.5, xlim=(0, 110))
ax.savefig("multiplot_SITI.jpg", dpi=200)
# ax.savefig("multiplot_SITI.pdf")
