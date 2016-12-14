import matplotlib.pyplot as plt
import numpy as np

def read_siti(filename):
	SI = []
	TI = []
	f = open(filename, 'r')
	for l in f:
		l = l.rstrip('\r\n')
		c = l.split(',')
		try:
			SI.append(float(c[0]))
			TI.append(float(c[1]))
		except:
			print l[0]
	return(SI,TI)
	
SI, TI = read_siti('results\\SITI_yv12.txt')
print 'yv12: ',  max(SI), np.mean(SI), max(TI), np.mean(TI)
plt.figure(0)
plt.plot(SI,'ro')
plt.plot(TI,'bs')
plt.ylabel('SI, TI')
plt.legend(('SI', 'TI'))
plt.savefig('SI_TI.png')
plt.show()
