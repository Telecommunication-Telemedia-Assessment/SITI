import matplotlib.pyplot as plt
import pandas as pd

videos_list = []
for i in range(1, 8):
  videos_list.append(i)

SI_TI_mean = pd.DataFrame()

for v in videos_list:
  df = pd.read_csv("%s.txt" % (v))
  SI_TI_mean[v-1] = [df["SI"].mean(), df["TI"].mean()]

SI_TI_mean = SI_TI_mean.T
SI_TI_mean.columns = ["Mean SI", "Mean TI"]

ax = SI_TI_mean.plot(style='o')
ax.set_xticklabels(videos_list)
plt.xlabel("Content")
plt.ylabel("Mean SI/TI")
plt.savefig("SITI_analysis.jpg", dpi=300, bbox_inches='tight')
