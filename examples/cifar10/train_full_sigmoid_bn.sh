#!/usr/bin/env sh
set -e

N=1
n=1
t=1
c=$t

TOOLS=/HOME/nsccgz_pcheng_1/skywang/caffe/caffe-skywang/build/tools
LOG=/HOME/nsccgz_pcheng_1/skywang/caffe/caffe-skywang/examples/cifar10/sigmod_bn/full-bn-$N-$n-$t-$c.log
SNAPSHOT=/HOME/nsccgz_pcheng_1/skywang/caffe/caffe-skywang/examples/cifar10/cifar10_full_bn_iter_0.solverstate.h5

yhrun -N $N -n $n --ntasks-per-node=1 -c $c $TOOLS/caffe train \
  --solver=/HOME/nsccgz_pcheng_1/skywang/caffe/caffe-skywang/examples/cifar10/cifar10_full_sigmoid_solver_bn.prototxt \
  --thread=$t\
  &>>$LOG

