#!/usr/bin/env sh
set -e

N=1
n=2
t=1
c=$t

TOOLS=/HOME/nsccgz_pcheng_1/skywang/caffe/caffe-skywang/build/tools
LOG=/HOME/nsccgz_pcheng_1/skywang/caffe/caffe-skywang/examples/cifar10/result/quick-${tag}-$N-$n-$t-$c.log
SNAPSHOT=/HOME/nsccgz_pcheng_1/skywang/caffe/caffe-skywang/examples/cifar10/cifar10_quick_iter_0.solverstate.h5

yhrun -N $N -n $n --ntasks-per-node=2 -c $c $TOOLS/caffe train \
  --solver=/HOME/nsccgz_pcheng_1/skywang/caffe/caffe-skywang/examples/cifar10/cifar10_quick_solver.prototxt \
  --thread=$t\
  &>>$LOG

# reduce learning rate by factor of 10 after 8 epochs
#yhrun -N $N -n $n -c $c $TOOLS/caffe train \
#  --solver=/HOME/nsccgz_xliao_1/skywang/caffe/caffe-skywang/examples/cifar10/cifar10_quick_solver_lr1.prototxt \
#  --snapshot=examples/cifar10/cifar10_quick_iter_4000.solverstate\
#  --thread=$t
