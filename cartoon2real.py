#!/usr/bin/python3

import numpy as np
import pylab as pl
#
data = np.loadtxt('log.dat');
#
fig = pl.figure(1)
#
a = 0
b = 6000 #len(data[:,0])
# compound
pl.subplot(711);
pl.plot(data[a:b,0],data[a:b,6]);
pl.plot(data[a:b,0],data[a:b,2]);
pl.ylabel('vis1/green lm');
#
pl.subplot(712);
pl.plot(data[a:b,0],data[a:b,26]);
pl.ylabel('vis2/green r');
#
pl.subplot(713);
pl.plot(data[a:b,0],data[a:b,9]);
pl.ylabel('mPFC: green lm');
#
pl.subplot(714);
pl.plot(data[a:b,0],data[a:b,25]);
pl.ylabel('DRN');
#
pl.subplot(715);
pl.plot(data[a:b,0],data[a:b,8]);
pl.ylabel('core w green');
#
pl.subplot(716);
pl.plot(data[a:b,0],data[a:b,14]);
pl.ylabel('VTA');
#
#
pl.subplot(717);
pl.plot(data[a:b,0],data[a:b,12]);
pl.ylabel('core out green');
#
pl.show();
