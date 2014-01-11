#!/usr/bin/python

# Expected BLEU calculation for
# -- r1 = "a b"
# --- h1,1 = "a b",    f1,1="fa=1"
# --- h1,2 = "a",      f1,2="fb=1"
# --- h1,3 = "a b c",  f1,3="fc=1"
# --- h1,4 = "c",      f1,4="fd=1"
# -- r2 = "a b c d"
# -- h2,1 = "a b c d", f2,1="fe=1"

import math
def phi(x):
    try:
        return (math.exp(x)-1)/(1+math.exp(10000*x)) + 1.0
    except OverflowError:
        return 1.0

def phiprime(x):
    try:
        e = math.exp(x)
        e1 = math.exp(10000*x)
        return ( e*(1+e1) - (e-1)*10000*e1 )/pow(1+e1, 2)
    except OverflowError:
        return 0.0

stats = [
  (0.5 , 0.25, 0.0, 0.0, 0.5 , 0.25, 0.0 , 0.0, 0.5),
  (0.25, 0.0 , 0.0, 0.0, 0.25, 0.0 , 0.0 , 0.0, 0.5),
  (0.5 , 0.25, 0.0, 0.0, 0.75, 0.5 , 0.25, 0.0, 0.5),
  (0.0 , 0.0 , 0.0, 0.0, 0.25, 0.0 , 0.0 , 0.0, 0.5),
  (4.0 , 3.0 , 2.0, 1.0, 4.0 , 3.0 , 2.0 , 1.0, 4.0)
]
t = (5.25, 3.5, 2.0, 1.0, 5.75, 3.75, 2.25, 1.0, 6.0)

P=(math.log(t[0])+math.log(t[1])+math.log(t[2])+math.log(t[3])-math.log(t[4])-math.log(t[5])-math.log(t[6])-math.log(t[7]))/4
expP = math.exp(P)

R = t[4]/t[0]
mR = -R
B = phi(1-R)
Bprime = phiprime(1-R)

left = map(lambda x: (expP*Bprime*mR*(x[8]/t[8] - x[4]/t[4])), stats)
right = map(lambda x: (expP*(x[0]/t[0]+x[1]/t[1]+x[2]/t[2]+x[3]/t[3]-x[4]/t[4]-x[5]/t[5]-x[6]/t[6]-x[7]/t[7])/4*B), stats)
grads = map(lambda x: (expP*Bprime*mR*(x[8]/t[8] - x[4]/t[4]) + expP*(x[0]/t[0]+x[1]/t[1]+x[2]/t[2]+x[3]/t[3]-x[4]/t[4]-x[5]/t[5]-x[6]/t[6]-x[7]/t[7])/4*B), stats)

feats = [0.0, 0.0, 0.0, 0.0]
for k in range(4):
    for kp in range(4):
        feats[kp] += grads[k] * (0.75 if (kp == k) else -0.25)
        print "%r %r: %r %r %r" % (k, kp, grads[k], (0.75 if (kp == k) else -0.25), feats[kp])

print "P=%r\nexpP=%r\nR=%r\nB=%r\nBprime=%r\ngrads=%r\nleft=%r\nright=%r\nfeats=%r" % (P, expP, R, B, Bprime, grads, left, right, feats)


# Expected BLEU calculation for
# w(fb)=ln(0.5) w(fc)=ln(0.5)
# -- r1 = "a b"
# --- h1,1 = "a b",    f1,1="fa=1"
# --- h1,2 = "a",      f1,2="fb=1"
# --- h1,3 = "c",      f1,3="fc=1"
# -- r2 = "a b c d"
# -- h2,1 = "a b c d", f2,1="fd=1"

stats = [
  (1.0 , 0.5 , 0.0, 0.0, 1.0 , 0.5, 0.0, 0.0, 1.0),
  (0.25, 0.0 , 0.0, 0.0, 0.25, 0.0, 0.0, 0.0, 0.5),
  (0.0 , 0.0 , 0.0, 0.0, 0.25, 0.0, 0.0, 0.0, 0.5),
  (4.0 , 3.0 , 2.0, 1.0, 4.0 , 3.0, 2.0, 1.0, 4.0)
]
t = (5.25, 3.5, 2.0, 1.0, 5.5, 3.5, 2.0, 1.0, 6.0)

P=(math.log(t[0])+math.log(t[1])+math.log(t[2])+math.log(t[3])-math.log(t[4])-math.log(t[5])-math.log(t[6])-math.log(t[7]))/4
expP = math.exp(P)

R = t[4]/t[0]
mR = -R
B = phi(1-R)
Bprime = phiprime(1-R)

left = map(lambda x: (expP*Bprime*mR*(x[8]/t[8] - x[4]/t[4])), stats)
right = map(lambda x: (expP*(x[0]/t[0]+x[1]/t[1]+x[2]/t[2]+x[3]/t[3]-x[4]/t[4]-x[5]/t[5]-x[6]/t[6]-x[7]/t[7])/4*B), stats)
grads = map(lambda x: (expP*Bprime*mR*(x[8]/t[8] - x[4]/t[4]) + expP*(x[0]/t[0]+x[1]/t[1]+x[2]/t[2]+x[3]/t[3]-x[4]/t[4]-x[5]/t[5]-x[6]/t[6]-x[7]/t[7])/4*B), stats)

probs = [0.5, 0.25, 0.25]
feats = [0.0, 0.0, 0.0]
cross = [0.0, math.log(0.5), math.log(0.5)]
scale = 0.0
for k in range(3):
    for kp in range(3):
        feats[kp] += grads[k] * ((1 if (kp == k) else 0)-probs[kp])
        scale += grads[k] * cross[kp] * ((1 if (kp == k) else 0)-probs[kp])
        print "%r %r: %r %r %r" % (k, kp, grads[k], (1 if (kp == k) else 0)-probs[kp], feats[kp])

print "P=%r\nexpP=%r\nR=%r\nB=%r\nBprime=%r\ngrads=%r\nleft=%r\nright=%r\nfeats=%r\nscale=%r" % (P, expP, R, B, Bprime, grads, left, right, feats, scale)
