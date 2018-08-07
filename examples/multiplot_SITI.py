import matplotlib.pyplot as plt
import pandas as pd

videos_list = []
for i in range(1, 10):
  videos_list.append(i)

SI_TI = pd.DataFrame()

for v in videos_list:
  df = pd.read_csv("%s.txt" % (v))
  SI_TI[v-1] = [df["SI"].max(), df["TI"].max()]

SI_TI = SI_TI.T
SI_TI.columns = ["SI", "TI"]

ax = SI_TI.plot(style='o')
ax.set_xticklabels(videos_list)
plt.xlabel("Content")
plt.ylabel("SI/TI")
plt.savefig("SITI_analysis.png", dpi=300, bbox_inches='tight')
