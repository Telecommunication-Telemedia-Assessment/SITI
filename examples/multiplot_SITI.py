"""
@author: stg7
"""
import pandas as pd
import glob

videos_list = []
df = pd.DataFrame()
for x in list(glob.glob("*.csv")):
    tmp = pd.read_csv(x, delimiter=",")
    tmp["SRC"] = x.replace(".csv", "")
    df = df.append(tmp, ignore_index=True)

import seaborn as sns

ax = sns.lmplot(x="SI", y="TI", hue="SRC", data=df, fit_reg =False)
ax.savefig("multiplot_SITI.jpg", dpi=200)
