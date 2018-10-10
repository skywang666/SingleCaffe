#ifndef CAFFE_PARALLEL_HPP_
#define CAFFE_PARALLEL_HPP_

#include <boost/thread.hpp>

#include <string>
#include <vector>

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/internal_thread.hpp"
#include "caffe/layer.hpp"
#include "caffe/proto/caffe.pb.h"
#include "caffe/solver.hpp"
#include "caffe/syncedmem.hpp"
#include "caffe/util/blocking_queue.hpp"
#include "caffe/util/nccl.hpp"

#ifdef USE_OMPI
#include "mpi.h"
#include <omp.h>
#endif

namespace caffe {

// Represents a net parameters. Once a net is created, its parameter buffers can
// be replaced by ones from Params, to allow parallelization. Params ensures
// parameters are allocated in one consecutive array.
template<typename Dtype>
class Params {
 public:
  explicit Params(shared_ptr<Solver<Dtype> > root_solver);
  virtual ~Params() {
  }

  inline size_t size() const {
    return size_;
  }
  inline Dtype* data() const {
    return data_;
  }
  inline Dtype* diff() const {
    return diff_;
  }

 protected:
  const size_t size_;           // Size of buffers
  Dtype* data_;                 // Network parameters
  Dtype* diff_;                 // Gradient

DISABLE_COPY_AND_ASSIGN(Params);
};

#ifdef USE_OMPI
template<typename Dtype>
class CPUParams : public Params<Dtype> {
  public:
    CPUParams(shared_ptr<Solver<Dtype> > solver);
    virtual ~CPUParams();

    void Configure(Solver<Dtype>* solver) const;

   protected:
    using Params<Dtype>::size_;
    using Params<Dtype>::data_;
    using Params<Dtype>::diff_;
};

template<typename Dtype>
class OMPICOM : public CPUParams<Dtype>,
                public Solver<Dtype>::Callback,
                public Net<Dtype>::Callback {
  public:
    OMPICOM(shared_ptr<Solver<Dtype> > solver);
    void Run();
    void syncData();  // Sync data between threads before training
    void syncNodeData();  // Sync data between process before training

    vector<Dtype *> position;  //Store data pointer
    vector<Dtype *> position_diff;  //Store diff pointer
    vector<Dtype *> *score_;
    Dtype *loss_;

  protected:
    void on_start() {}
    void run(int layer) {}
    void on_gradients_ready();
    void on_test(Dtype &loss, vector<Dtype> &score);
    void avgDiff(int offset, int length);

    shared_ptr<Solver<Dtype> > solver_;
    using Params<Dtype>::size_;
    using Params<Dtype>::data_;
    using Params<Dtype>::diff_;
};
#endif

#ifdef USE_NCCL
// Params stored in GPU memory.
template<typename Dtype>
class GPUParams : public Params<Dtype> {
 public:
  GPUParams(shared_ptr<Solver<Dtype> > root_solver, int device);
  virtual ~GPUParams();

  void Configure(Solver<Dtype>* solver) const;

 protected:
  using Params<Dtype>::size_;
  using Params<Dtype>::data_;
  using Params<Dtype>::diff_;
};

template<typename Dtype>
class NCCL : public GPUParams<Dtype>,
             public Solver<Dtype>::Callback,
             public Net<Dtype>::Callback {
 public:
  /**
   * Single process version.
   */
  explicit NCCL(shared_ptr<Solver<Dtype> > solver);
  /**
   * In multi-process settings, first create a NCCL id (new_uid), then
   * pass it to each process to create connected instances.
   */
  NCCL(shared_ptr<Solver<Dtype> > solver, const string& uid);
  ~NCCL();

  boost::barrier* barrier();
  void set_barrier(boost::barrier* value);

  /**
   * In single process settings, create instances without uids and
   * call this to connect them.
   */
  static void InitSingleProcess(vector<NCCL<Dtype>*>* nccls);

  static string new_uid();

  /**
   * Broadcast weights from rank 0 other solvers.
   */
  void Broadcast();

  /**
   * Single process multi-GPU.
   */
  void Run(const vector<int>& gpus, const char* restore);

 protected:
  void Init();
  void on_start() {}
  void run(int layer);  // Net callback
  void on_gradients_ready();

  ncclComm_t comm_;
  cudaStream_t stream_;

  shared_ptr<Solver<Dtype> > solver_;
  // Should not be necessary, https://github.com/NVIDIA/nccl/issues/37
  boost::barrier* barrier_;
  using Params<Dtype>::size_;
  using Params<Dtype>::data_;
  using Params<Dtype>::diff_;
};
#endif  //use NCCL

}  // namespace caffe

#endif  // USE_NCCL
