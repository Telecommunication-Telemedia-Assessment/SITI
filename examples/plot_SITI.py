import matplotlib.pyplot as plt
import numpy as np

def read_siti(filename):
  SI = []
  TI = []
  with open(filename, 'r') as f:
    for l in f:
      l = l.rstrip('\r\n')
      c = l.split(',')
      try:
        SI.append(float(c[1]))
        TI.append(float(c[2]))
      except:
        print l
    return(SI,TI)

SI, TI = read_siti('example.txt')

print("max SI:  " + str(max(SI)))
print("mean SI: " + str(np.mean(SI)))
print("max TI:  " + str(max(TI)))
print("mean TI: " + str(np.mean(TI)))

plt.figure(0)
plt.plot(SI,'ro')
plt.plot(TI,'bs')
plt.ylabel('SI, TI')
plt.legend(('SI', 'TI'))
plt.savefig('example.png')
plt.show()
