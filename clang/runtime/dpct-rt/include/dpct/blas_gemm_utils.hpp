//==---- blas_gemm_utils.hpp ----------------------*- C++ -*----------------==//
//
// Copyright (C) Intel Corporation
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// See https://llvm.org/LICENSE.txt for license information.
//
//===----------------------------------------------------------------------===//
// This file contains the implementation of GEneral Matrix Multiplication by
// using oneDNN. More datatype combinations and the epilogue can be supported by
// oneDNN, which is not available in blas_utils.hpp using oneMKL.
//===----------------------------------------------------------------------===//

#ifndef __DPCT_BLAS_GEMM_UTILS_HPP__
#define __DPCT_BLAS_GEMM_UTILS_HPP__

#include "compat_service.hpp"
#include "blas_utils.hpp"
#include "dnnl_utils.hpp"

namespace dpct {
namespace blas_gemm {
namespace experimental {
enum class order_t : std::uint8_t {
  col,
  row,
  col32,
  col4_4r2_8c,
  col32_2r_4r4
};
enum class pointer_mode_t {
  host,
  device,
  device_vector,
  alpha_device_vector_beta_zero,
  alpha_device_vector_beta_host
};
enum class epilogue_t {
  nop = 1,
  relu,
  bias,
  gelu,
  gelu_bias,
  gelu_aux,
  gelu_aux_bias,
  bgradb
};

class descriptor;
using descriptor_ptr = descriptor *;
class matrix_layout_t;
using matrix_layout_ptr = matrix_layout_t *;
class matmul_desc_t;
using matmul_desc_ptr = matmul_desc_t *;
class transform_desc_t;
using transform_desc_ptr = transform_desc_t *;

class descriptor {
public:
  descriptor() {}
  void init(sycl::queue *q_ptr) {
    _engine = ::dnnl::sycl_interop::make_engine(q_ptr->get_device(),
                                                q_ptr->get_context());
    _engine_stream = ::dnnl::sycl_interop::make_stream(_engine, *q_ptr);
  }
  ::dnnl::engine get_engine() const noexcept { return _engine; }
  ::dnnl::stream get_engine_stream() const noexcept { return _engine_stream; };

private:
  ::dnnl::engine _engine;
  ::dnnl::stream _engine_stream;
};

class matrix_layout_t {
public:
  enum class attribute {
    type,
    order,
    rows,
    cols,
    ld,
    batch_count,
    strided_batch_offset
  };

  matrix_layout_t(library_data_t type, std::uint64_t rows, std::uint64_t cols,
                  std::int64_t ld)
      : _type(type), _rows(rows), _cols(cols), _ld(ld) {}

  void set_attribute(attribute attr, const void *mem) {
    get_set_attr<true>(attr, const_cast<void *>(mem));
  }
  void get_attribute(attribute attr, void *mem) {
    get_set_attr<false>(attr, mem);
  }

private:
  template <bool is_set> void get_set_attr(attribute attr, void *mem) {
#define CASE(tag)                                                              \
  case attribute::tag:                                                         \
    if constexpr (is_set) {                                                    \
      _##tag = *static_cast<decltype(_##tag) *>(mem);                          \
    } else {                                                                   \
      *static_cast<decltype(_##tag) *>(mem) = _##tag;                          \
    }                                                                          \
    break;
    switch (attr) {
      CASE(type)
      CASE(order)
      CASE(rows)
      CASE(cols)
      CASE(ld)
      CASE(batch_count)
      CASE(strided_batch_offset)
    }
#undef CASE
  }

  library_data_t _type;
  order_t _order = order_t::col;
  std::uint64_t _rows;
  std::uint64_t _cols;
  std::int64_t _ld;
  std::uint64_t _batch_count = 1;
  std::uint64_t _strided_batch_offset = 0;

  friend sycl::event matmul(descriptor_ptr handle, matmul_desc_ptr computeDesc,
                            const void *alpha, const void *a,
                            matrix_layout_ptr a_desc, const void *b,
                            matrix_layout_ptr b_desc, const void *beta,
                            const void *c, matrix_layout_ptr c_desc, void *d,
                            matrix_layout_ptr d_desc,
                            ::dpct::cs::queue_ptr q_ptr);
  friend sycl::event
  matrix_transform(transform_desc_ptr transform_desc, const void *alpha,
                   const void *a, matrix_layout_ptr a_desc, const void *beta,
                   const void *b, matrix_layout_ptr b_desc, void *c,
                   matrix_layout_ptr c_desc, ::dpct::cs::queue_ptr q_ptr);
};

class matmul_desc_t {
public:
  enum class attribute {
    compute_type,
    scale_type,
    bias_type,
    bias_pointer,
    pointer_mode,
    trans_a,
    trans_b,
    trans_c,
    epilogue,
    epilogue_aux_ld,
    epilogue_aux_pointer,
    epilogue_aux_data_type,
    a_scale_pointer,
    b_scale_pointer,
    d_scale_pointer,
    absmax_d_pointer,
    unsupport
  };

  matmul_desc_t(compute_type compute_type, library_data_t scale_type)
      : _compute_type(compute_type), _scale_type(scale_type) {}

  void set_attribute(attribute attr, const void *mem) {
    if (attr != attribute::unsupport)
      get_set_attr<true>(attr, const_cast<void *>(mem));
  }
  void get_attribute(attribute attr, void *mem) {
    if (attr != attribute::unsupport)
      get_set_attr<false>(attr, mem);
  }

private:
  template <bool is_set> void get_set_attr(attribute attr, void *mem) {
#define CASE(tag)                                                              \
  case attribute::tag:                                                         \
    if constexpr (is_set) {                                                    \
      _##tag = *static_cast<decltype(_##tag) *>(mem);                          \
    } else {                                                                   \
      *static_cast<decltype(_##tag) *>(mem) = _##tag;                          \
    }                                                                          \
    break;
    switch (attr) {
      CASE(compute_type)
      CASE(scale_type)
      CASE(bias_type)
      CASE(bias_pointer)
      CASE(pointer_mode)
      CASE(trans_a)
      CASE(trans_b)
      CASE(trans_c)
      CASE(epilogue)
      CASE(a_scale_pointer)
      CASE(b_scale_pointer)
      CASE(d_scale_pointer)
      CASE(absmax_d_pointer)
      CASE(epilogue_aux_ld)
      CASE(epilogue_aux_pointer)
      CASE(epilogue_aux_data_type)
    default:
      break;
    }
#undef CASE
  }

  compute_type _compute_type;
  library_data_t _scale_type;
  library_data_t _bias_type = library_data_t::real_float;
  pointer_mode_t _pointer_mode = pointer_mode_t::host;
  oneapi::mkl::transpose _trans_a = oneapi::mkl::transpose::nontrans;
  oneapi::mkl::transpose _trans_b = oneapi::mkl::transpose::nontrans;
  oneapi::mkl::transpose _trans_c = oneapi::mkl::transpose::nontrans;
  epilogue_t _epilogue = epilogue_t::nop;
  library_data_t _epilogue_aux_data_type = library_data_t::real_float;
  size_t _epilogue_aux_ld = 0;
  void *_a_scale_pointer = nullptr;
  void *_b_scale_pointer = nullptr;
  void *_d_scale_pointer = nullptr;
  void *_absmax_d_pointer = nullptr;
  void *_bias_pointer = nullptr;
  void *_epilogue_aux_pointer = nullptr;

  friend sycl::event matmul(descriptor_ptr handle, matmul_desc_ptr computeDesc,
                            const void *alpha, const void *a,
                            matrix_layout_ptr a_desc, const void *b,
                            matrix_layout_ptr b_desc, const void *beta,
                            const void *c, matrix_layout_ptr c_desc, void *d,
                            matrix_layout_ptr d_desc,
                            ::dpct::cs::queue_ptr q_ptr);
};

namespace detail {
/// Sacling each row of matrix D with the corresponding element of vector alpha.
template <class T, class Tscale>
sycl::event scale_new_a_impl(::dpct::cs::queue_ptr q_ptr, int rows, int cols,
                             void *a_ptr, const void *alpha_ptr,
                             bool vector_alpha, bool device_alpha,
                             const void *a_scale_ptr, const void *b_scale_ptr,
                             const std::vector<sycl::event> &dependencies) {
  std::vector<sycl::event> deps = dependencies;
  T *a = (T *)a_ptr;
  Tscale *alpha = (Tscale *)alpha_ptr;
  Tscale *a_scale = (Tscale *)a_scale_ptr;
  Tscale *b_scale = (Tscale *)b_scale_ptr;

  if (!device_alpha) {
    Tscale *alpha_host = alpha;
    alpha = (Tscale *)::dpct::cs::malloc(sizeof(Tscale), *q_ptr);
    deps.push_back(
        ::dpct::cs::memcpy(*q_ptr, alpha, alpha_host, sizeof(Tscale)));
  }
  if (!a_scale_ptr) {
    a_scale = (Tscale *)::dpct::cs::malloc(sizeof(Tscale), *q_ptr);
    deps.push_back(::dpct::cs::fill<Tscale>(*q_ptr, a_scale, 1.0, 1));
  }
  if (!b_scale_ptr) {
    b_scale = (Tscale *)::dpct::cs::malloc(sizeof(Tscale), *q_ptr);
    deps.push_back(::dpct::cs::fill<Tscale>(*q_ptr, b_scale, 1.0, 1));
  }

  sycl::event e = q_ptr->submit([&](sycl::handler &cgh) {
    cgh.depends_on(deps);
#ifdef DPCT_USM_LEVEL_NONE
    access_wrapper<T *> a_acc(a, cgh);
    access_wrapper<Tscale *> alpha_acc(alpha, cgh);
    access_wrapper<Tscale *> a_scale_acc(a_scale, cgh);
    access_wrapper<Tscale *> b_scale_acc(b_scale, cgh);
#endif
    cgh.parallel_for<
        ::dpct::cs::kernel_name<class scale_with_alpha, T, Tscale>>(
        sycl::range<2>(rows, cols), [=](sycl::id<2> index) {
#ifdef DPCT_USM_LEVEL_NONE
          T *a_data = a_acc.get_raw_pointer();
          Tscale *alpha_data = alpha_acc.get_raw_pointer();
          Tscale *a_scale_data = a_scale_acc.get_raw_pointer();
          Tscale *b_scale_data = b_scale_acc.get_raw_pointer();
#else
          T *a_data = a;
          const Tscale *alpha_data = alpha;
          const Tscale *a_scale_data = a_scale;
          const Tscale *b_scale_data = b_scale;
#endif
          size_t row_idx = index.get(0);
          size_t col_idx = index.get(1);
          size_t idx = rows * col_idx + row_idx;

          Tscale ab_scale = a_scale_data[0] * b_scale_data[0];

          if (vector_alpha)
            a_data[idx] = a_data[idx] * alpha_data[row_idx] * ab_scale;
          else
            a_data[idx] = a_data[idx] * alpha_data[0] * ab_scale;
        });
  });
  return q_ptr->submit([&](sycl::handler &cgh) {
    cgh.depends_on(e);
    cgh.host_task([=] {
      if (!device_alpha)
        ::dpct::cs::free(alpha, *q_ptr);
      if (!a_scale_ptr)
        ::dpct::cs::free(a_scale, *q_ptr);
      if (!b_scale_ptr)
        ::dpct::cs::free(b_scale, *q_ptr);
    });
  });
}

// a is col major without padding
inline sycl::event scale_new_a(::dpct::cs::queue_ptr q_ptr, int rows, int cols,
                               void *a, library_data_t a_type,
                               const void *alpha, library_data_t scale_type,
                               bool vector_alpha, bool device_alpha,
                               const void *a_scale, const void *b_scale,
                               const std::vector<sycl::event> &deps) {
  std::uint64_t key = dpct::detail::get_type_combination_id(a_type, scale_type);
  sycl::event e;
  switch (key) {
#define __SCALE_NEW_A_IMPL_CASE(A_TYPE_ENUM, SCALE_TYPE_ENUM, A_TYPE,          \
                                SCALE_TYPE)                                    \
  case dpct::detail::get_type_combination_id(                                  \
      library_data_t::A_TYPE_ENUM, library_data_t::SCALE_TYPE_ENUM): {         \
    e = scale_new_a_impl<A_TYPE, SCALE_TYPE>(q_ptr, rows, cols, a, alpha,      \
                                             vector_alpha, device_alpha,       \
                                             a_scale, b_scale, deps);          \
    break;                                                                     \
  }
    __SCALE_NEW_A_IMPL_CASE(real_int8, real_float, std::int8_t, float)
    __SCALE_NEW_A_IMPL_CASE(real_int32, real_float, int, float)
    __SCALE_NEW_A_IMPL_CASE(real_int8, real_int32, std::int8_t, int)
    __SCALE_NEW_A_IMPL_CASE(real_float, real_float, float, float)
#undef __SCALE_NEW_A_IMPL_CASE
  default:
    throw std::runtime_error("dpct::blas_gemm::experimental::detail::scale_new_"
                             "a_impl() does not support the data "
                             "type combination currently.");
  }
  return e;
}

/// Get a linear idx map for a 2D point (row_idx, col_idx) between src_order and
/// dst_order.
inline std::tuple<size_t, size_t>
get_linear_idx_map(size_t rows, size_t cols, size_t src_ld, order_t src_order,
                   size_t dst_ld, order_t dst_order, size_t row_idx,
                   size_t col_idx) {
#define COMBINE(from, to)                                                      \
  static_cast<std::uint16_t>(from) << 8 | static_cast<std::uint8_t>(to)

  size_t from_linear_idx, to_linear_idx;
  switch (COMBINE(src_order, dst_order)) {
  case COMBINE(order_t::col, order_t::row): {
    from_linear_idx = src_ld * col_idx + row_idx;
    to_linear_idx = dst_ld * row_idx + col_idx;
    break;
  }
  case COMBINE(order_t::row, order_t::col): {
    from_linear_idx = src_ld * row_idx + col_idx;
    to_linear_idx = dst_ld * col_idx + row_idx;
    break;
  }
  case COMBINE(order_t::col, order_t::col32): {
    from_linear_idx = src_ld * col_idx + row_idx;
    to_linear_idx = dst_ld * (col_idx / 32) + 32 * row_idx + col_idx % 32;
    break;
  }
  case COMBINE(order_t::col32, order_t::col): {
    from_linear_idx = src_ld * (col_idx / 32) + 32 * row_idx + col_idx % 32;
    to_linear_idx = dst_ld * col_idx + row_idx;
    break;
  }
  case COMBINE(order_t::col, order_t::col4_4r2_8c): {
    from_linear_idx = src_ld * col_idx + row_idx;

    size_t from_row_in_row8_col32 = row_idx % 8;
    size_t from_col_in_row8_col32 = col_idx % 32;

    size_t to_row_in_row8_col32 =
        4 * (from_row_in_row8_col32 % 2) + from_col_in_row8_col32 / 8;
    size_t to_col_in_row8_col32 = 16 * ((from_col_in_row8_col32 / 4) % 2) +
                                  4 * (from_row_in_row8_col32 / 2) +
                                  from_col_in_row8_col32 % 4;
    size_t to_linear_idx_in_row8_col32 =
        to_row_in_row8_col32 * 32 + to_col_in_row8_col32;

    to_linear_idx = dst_ld * (col_idx / 32) + (row_idx / 8) * (32 * 8) +
                    to_linear_idx_in_row8_col32;
    break;
  }
  case COMBINE(order_t::col4_4r2_8c, order_t::col): {
    to_linear_idx = dst_ld * col_idx + row_idx;

    size_t to_row_in_row8_col32 = row_idx % 8;
    size_t to_col_in_row8_col32 = col_idx % 32;

    size_t from_row_in_row8_col32 =
        4 * (to_row_in_row8_col32 % 2) + to_col_in_row8_col32 / 8;
    size_t from_col_in_row8_col32 = 16 * ((to_col_in_row8_col32 / 4) % 2) +
                                    4 * (to_row_in_row8_col32 / 2) +
                                    to_col_in_row8_col32 % 4;
    size_t from_linear_idx_in_row8_col32 =
        from_row_in_row8_col32 * 32 + from_col_in_row8_col32;

    from_linear_idx = src_ld * (col_idx / 32) + (row_idx / 8) * (32 * 8) +
                      from_linear_idx_in_row8_col32;
    break;
  }
  case COMBINE(order_t::col, order_t::col32_2r_4r4): {
    from_linear_idx = src_ld * col_idx + row_idx;

    size_t from_row_in_row32_col32 = row_idx % 32;
    size_t from_col_in_row32_col32 = col_idx % 32;

    size_t to_row_in_row32_col32 = 8 * ((from_row_in_row32_col32 % 8) / 2) +
                                   (from_row_in_row32_col32 / 8) * 2 +
                                   from_row_in_row32_col32 % 2;
    size_t to_col_in_row32_col32 = from_col_in_row32_col32;
    size_t to_linear_idx_in_row32_col32 =
        to_row_in_row32_col32 * 32 + to_col_in_row32_col32;

    to_linear_idx = dst_ld * (col_idx / 32) + (row_idx / 32) * (32 * 32) +
                    to_linear_idx_in_row32_col32;
    break;
  }
  case COMBINE(order_t::col32_2r_4r4, order_t::col): {
    to_linear_idx = dst_ld * col_idx + row_idx;

    size_t to_row_in_row32_col32 = row_idx % 32;
    size_t to_col_in_row32_col32 = col_idx % 32;

    size_t from_row_in_row32_col32 = 8 * ((to_row_in_row32_col32 % 8) / 2) +
                                     (to_row_in_row32_col32 / 8) * 2 +
                                     to_row_in_row32_col32 % 2;
    size_t from_col_in_row32_col32 = to_col_in_row32_col32;
    size_t from_linear_idx_in_row32_col32 =
        from_row_in_row32_col32 * 32 + from_col_in_row32_col32;

    from_linear_idx = src_ld * (col_idx / 32) + (row_idx / 32) * (32 * 32) +
                      from_linear_idx_in_row32_col32;
    break;
  }
  }
#undef COMBINE
  return std::make_tuple(from_linear_idx, to_linear_idx);
}

template <template <typename> typename functor_t, typename... args_t>
inline auto type_dispatch(library_data_t type, args_t &&...args) {
  switch (type) {
  case library_data_t::real_float:
    return functor_t<float>()(std::forward<args_t>(args)...);
  case library_data_t::real_int8:
    return functor_t<std::int8_t>()(std::forward<args_t>(args)...);
  case library_data_t::real_int32:
    return functor_t<int>()(std::forward<args_t>(args)...);
  default:
    throw std::runtime_error("the data type is unsupported");
  }
}

template <typename T> struct matrix_transform_impl {
  sycl::event operator()(::dpct::cs::queue_ptr q_ptr, size_t rows, size_t cols,
                         size_t a_ld, order_t a_order, const void *a,
                         size_t c_ld, order_t c_order, void *c,
                         std::vector<sycl::event> deps) {
    if ((a_order != order_t::col && c_order != order_t::col) ||
        (a_order == order_t::col && c_order == order_t::col)) {
      throw std::runtime_error("dpct::blas_gemm::experimental::detail::matrix_"
                               "transform_impl() does not "
                               "support the order combination currently.");
    }

    return q_ptr->submit([&](sycl::handler &cgh) {
      cgh.depends_on(deps);
#ifdef DPCT_USM_LEVEL_NONE
      access_wrapper<const T *> a_acc(a, cgh);
      access_wrapper<T *> c_acc(c, cgh);
#endif
      cgh.parallel_for<
          ::dpct::cs::kernel_name<class matrix_transform_col_to_row, T>>(
          sycl::range<2>(a_ld, cols), [=](sycl::id<2> index) {
#ifdef DPCT_USM_LEVEL_NONE
            auto a_data = a_acc.get_raw_pointer();
            auto c_data = c_acc.get_raw_pointer();
#else
            auto a_data = (const T *)a;
            auto c_data = (T *)c;
#endif
            size_t row_idx = index.get(0);
            size_t col_idx = index.get(1);
            if (row_idx < rows) {
              size_t from_linear_idx, to_linear_idx;
              std::tie(from_linear_idx, to_linear_idx) = get_linear_idx_map(
                  rows, cols, a_ld, a_order, c_ld, c_order, row_idx, col_idx);
              c_data[to_linear_idx] = a_data[from_linear_idx];
            }
          });
    });
  }
};

#ifdef DPCT_USM_LEVEL_NONE
template <typename T> struct scale_d_impl {
  sycl::event operator()(const void *d_scale_ptr, void *d, size_t ld,
                         size_t rows, size_t cols, ::dpct::cs::queue_ptr q_ptr,
                         dpct::library_data_t scale_type,
                         std::vector<sycl::event> deps) {
    if (scale_type == dpct::library_data_t::real_float)
      return q_ptr->submit([&](sycl::handler &cgh) {
        cgh.depends_on(deps);
        access_wrapper<const float *> d_scale_acc(
            static_cast<const float *>(d_scale_ptr), cgh);
        access_wrapper<T *> d_acc(d, cgh);
        cgh.parallel_for<::dpct::cs::kernel_name<class scale_d_float, T>>(
            sycl::range<2>(ld, cols), [=](sycl::id<2> idx) {
              float scale_factor = d_scale_acc.get_raw_pointer()[0];
              auto d_data = d_acc.get_raw_pointer();
              size_t row_idx = idx.get(0);
              size_t col_idx = idx.get(1);
              if (row_idx < rows) {
                size_t linear_idx = row_idx + ld * col_idx;
                d_data[linear_idx] = d_data[linear_idx] * scale_factor;
              }
            });
      });
    else {
      // int type
      return q_ptr->submit([&](sycl::handler &cgh) {
        cgh.depends_on(deps);
        access_wrapper<const int *> d_scale_acc(
            static_cast<const int *>(d_scale_ptr), cgh);
        access_wrapper<T *> d_acc(d, cgh);
        cgh.parallel_for<::dpct::cs::kernel_name<class scale_d_int, T>>(
            sycl::range<2>(ld, cols), [=](sycl::id<2> idx) {
              float scale_factor =
                  static_cast<float>(d_scale_acc.get_raw_pointer()[0]);
              auto d_data = d_acc.get_raw_pointer();
              size_t row_idx = idx.get(0);
              size_t col_idx = idx.get(1);
              if (row_idx < rows) {
                size_t linear_idx = row_idx + ld * col_idx;
                d_data[linear_idx] = d_data[linear_idx] * scale_factor;
              }
            });
      });
    }
  }
};

template <typename T> struct set_buffer_impl {
  void operator()(::dnnl::memory *dnnl_memory, const void *ptr) {
    auto buf = get_buffer<std::int8_t>(ptr);
    ::dnnl::sycl_interop::set_buffer(*dnnl_memory, buf);
  }
};
#else
template <typename T> struct scale_d_impl {
  sycl::event operator()(const void *d_scale_ptr, void *d, size_t ld,
                         size_t rows, size_t cols, ::dpct::cs::queue_ptr q_ptr,
                         dpct::library_data_t scale_type,
                         std::vector<sycl::event> deps) {
    return q_ptr->submit([&](sycl::handler &cgh) {
      cgh.depends_on(deps);
      cgh.parallel_for<::dpct::cs::kernel_name<class scale_d, T>>(
          sycl::range<2>(ld, cols), [=](sycl::id<2> idx) {
            float scale_factor;
            if (scale_type == dpct::library_data_t::real_float)
              scale_factor = static_cast<const float *>(d_scale_ptr)[0];
            else {
              // int type
              scale_factor =
                  static_cast<float>(static_cast<const int *>(d_scale_ptr)[0]);
            }
            auto d_data = (T *)d;
            size_t row_idx = idx.get(0);
            size_t col_idx = idx.get(1);
            if (row_idx < rows) {
              size_t linear_idx = row_idx + ld * col_idx;
              d_data[linear_idx] = d_data[linear_idx] * scale_factor;
            }
          });
    });
  }
};
#endif

template <typename T> struct get_beta_value_impl {
  int operator()(const void *beta, ::dpct::cs::queue_ptr q_ptr) {
    T beta_host;
    ::dpct::cs::memcpy(*q_ptr, &beta_host, beta, sizeof(T),
                       ::dpct::cs::memcpy_direction::automatic)
        .wait();
    T zero = T(0);
    T one = T(1);
    if (beta_host == zero)
      return 0;
    else if (beta_host == one)
      return 1;
    return -1;
  }
};

template <typename T> struct abs_max_op {
  auto operator()(const T &lhs, const T &rhs) {
    T abs_lhs = lhs >= 0 ? lhs : -lhs;
    T abs_rhs = rhs >= 0 ? rhs : -rhs;
    return (abs_lhs < abs_rhs) ? abs_rhs : abs_lhs;
  }
};

template <typename T> struct absmax_impl {
  sycl::event operator()(void *absmax_ptr, const void *new_d, size_t ld,
                         size_t rows, size_t cols, ::dpct::cs::queue_ptr q_ptr,
                         std::vector<sycl::event> deps) {
    return q_ptr->submit([&](sycl::handler &cgh) {
#ifdef DPCT_USM_LEVEL_NONE
      auto absmax_reduction = sycl::reduction(
          get_buffer<T>(absmax_ptr), cgh, T(0), abs_max_op<T>(),
          {sycl::property::reduction::initialize_to_identity()});
      access_wrapper<const T *> new_d_acc(new_d, cgh);
#else
      auto absmax_reduction = sycl::reduction(
          (T *)(absmax_ptr), T(0), abs_max_op<T>(),
          {sycl::property::reduction::initialize_to_identity()});
#endif
      cgh.depends_on(deps);
      cgh.parallel_for<::dpct::cs::kernel_name<class absmax_reduction, T>>(
          sycl::range<2>(ld, cols), absmax_reduction,
          [=](sycl::id<2> idx, auto &absmax) {
#ifdef DPCT_USM_LEVEL_NONE
            auto new_d_data = new_d_acc.get_raw_pointer();
#else
            auto new_d_data = (const T *)new_d;
#endif
            size_t row_idx = idx.get(0);
            size_t col_idx = idx.get(1);
            if (row_idx < rows) {
              size_t linear_idx = row_idx + ld * col_idx;
              absmax.combine(new_d_data[linear_idx]);
            }
          });
    });
  }
};
} // namespace detail

/// This function does the following operations:
/// (1) D_temp = epilogue(alpha * scale_a * op_a(A) * scale_b * op_b(B) + beta * C)
/// (2) Amax = absmax(D_temp) when matmul_desc_t::attribute::absmax_d_pointer is specified
/// (3) D = scale_d * D_temp
///   "op_a" is specified by the matmul_desc_t::attribute::trans_a
///   (default is nontrans)
///   "op_b" is specified by the matmul_desc_t::attribute::trans_b
///   (default is nontrans)
///   "scale_a" is specified by the matmul_desc_t::attribute::a_scale_pointer
///   (default is nullptr, which behaves as 1.0)
///   "scale_b" is specified by the matmul_desc_t::attribute::b_scale_pointer
///   (default is nullptr, which behaves as 1.0)
///   "scale_d" is specified by the matmul_desc_t::attribute::d_scale_pointer
///   (default is nullptr, which behaves as 1.0)
///   "alpha" can be a scalar value or a vector, which is specified by the
///   matmul_desc_tattribute::::pointer_mode
///   "epilogue" is specified by the matmul_desc_t::attribute::epilogue
/// Currently, this function only supports the following type combinations:
///   scale_type==int32 && a_type==int8 && b_type==int8 && c_type==int32;
///   scale_type==float && a_type==int8 && b_type==int8 && c_type==int8;
///   scale_type==float && a_type==int8 && b_type==int8 && c_type==int32;
///   scale_type==float && a_type==float && b_type==float && c_type==float.
/// Currently, this function only supports beta==0 or beta==1.
/// Currently, this function only supports the relu, bias, gelu, gelu_bias,
/// gelu_aux and gelu_aux_bias epilogue.
/// NOTE: Non-col-major matrix will be converted to col-major matrix before.
/// TODO: Impl row-major matmul without layout conversion.
/// multiplication and converted back after multiplication.
/// \param [in] handle A handle containing context info.
/// \param [in] compute_desc Describe the computation.
/// \param [in] alpha Scaling factor alpha.
/// \param [in] a Input matrix A.
/// \param [in] a_desc Describe the matrix A.
/// \param [in] b Input matrix B.
/// \param [in] b_desc Describe the matrix B.
/// \param [in] beta Scaling factor beta.
/// \param [in] c Input matrix C.
/// \param [in] c_desc Describe the matrix C.
/// \param [out] d Output matrix D.
/// \param [in] d_desc Describe the matrix D.
/// \param [in] q_ptr The queue where the routine should be executed.
inline sycl::event matmul(descriptor_ptr handle, matmul_desc_ptr compute_desc,
                          const void *alpha, const void *a,
                          matrix_layout_ptr a_desc, const void *b,
                          matrix_layout_ptr b_desc, const void *beta,
                          const void *c, matrix_layout_ptr c_desc, void *d,
                          matrix_layout_ptr d_desc,
                          ::dpct::cs::queue_ptr q_ptr) {
  const size_t m = compute_desc->_trans_a == oneapi::mkl::transpose::nontrans
                       ? a_desc->_rows
                       : a_desc->_cols;
  const size_t n = d_desc->_cols;
  const size_t k = compute_desc->_trans_b == oneapi::mkl::transpose::nontrans
                       ? b_desc->_rows
                       : b_desc->_cols;
  const library_data_t a_type = a_desc->_type;
  const library_data_t b_type = b_desc->_type;
  const library_data_t c_type = c_desc->_type;
  const library_data_t d_type = d_desc->_type;
  const library_data_t scale_type = compute_desc->_scale_type;

  if (!q_ptr)
    q_ptr = &::dpct::cs::get_default_queue();
  handle->init(q_ptr);
  bool vector_alpha = false;
  bool device_alpha = false;
  if (compute_desc->_pointer_mode == pointer_mode_t::device_vector ||
      compute_desc->_pointer_mode ==
          pointer_mode_t::alpha_device_vector_beta_zero ||
      compute_desc->_pointer_mode ==
          pointer_mode_t::alpha_device_vector_beta_host) {
    vector_alpha = true;
    device_alpha = true;
  } else if (compute_desc->_pointer_mode == pointer_mode_t::device) {
    device_alpha = true;
  }

  bool beta_is_zero = true;
  if (beta != nullptr) {
    int beta_value = detail::type_dispatch<detail::get_beta_value_impl>(
        compute_desc->_scale_type, beta, q_ptr);
    if (beta_value != 0) {
      beta_is_zero = false;
      if (beta_value != 1)
        throw std::runtime_error(
            "dpct::blas_gemm::experimental::matmul() does "
            "not support non-zero and non-one beta currently.");
    }
  }

  if (compute_desc->_epilogue != epilogue_t::nop &&
      compute_desc->_epilogue != epilogue_t::relu &&
      compute_desc->_epilogue != epilogue_t::bias &&
      compute_desc->_epilogue != epilogue_t::gelu &&
      compute_desc->_epilogue != epilogue_t::gelu_bias &&
      compute_desc->_epilogue != epilogue_t::gelu_aux &&
      compute_desc->_epilogue != epilogue_t::gelu_aux_bias) {
    throw std::runtime_error("dpct::blas_gemm::experimental::matmul() only "
                             "supports relu, bias, gelu, gelu_bias, gelu_aux "
                             "and gelu_aux_bias epilogue currently.");
  }

  if (!(compute_desc->_scale_type == library_data_t::real_int32 &&
        a_desc->_type == library_data_t::real_int8 &&
        b_desc->_type == library_data_t::real_int8 &&
        c_desc->_type == library_data_t::real_int32) &&
      !(compute_desc->_scale_type == library_data_t::real_float &&
        a_desc->_type == library_data_t::real_int8 &&
        b_desc->_type == library_data_t::real_int8 &&
        c_desc->_type == library_data_t::real_int8) &&
      !(compute_desc->_scale_type == library_data_t::real_float &&
        a_desc->_type == library_data_t::real_int8 &&
        b_desc->_type == library_data_t::real_int8 &&
        c_desc->_type == library_data_t::real_int32) &&
      !(compute_desc->_scale_type == library_data_t::real_float &&
        a_desc->_type == library_data_t::real_float &&
        b_desc->_type == library_data_t::real_float &&
        c_desc->_type == library_data_t::real_float)) {
    throw std::runtime_error(
        "dpct::blas_gemm::experimental::matmul() only supports data type "
        "combinataions:\n  scale_type==int32 && a_type==int8 && b_type==int8"
        "&& c_type==int32,\n  scale_type==float && a_type==int8 && "
        "b_type==int8 && c_type==int8,\n  scale_type==float && a_type==int8"
        "&& b_type==int8 && c_type==int32 or\n  scale_type==float && "
        "a_type==float"
        "&& b_type==float && c_type==float.");
  }

  // For non-col_major matrix, convert it to col_major.
  const void *new_a = a;
  const void *new_b = b;
  const void *new_c = c;
  void *new_d = d;
  bool new_b_allocated = false;
  bool new_c_allocated = false;
  bool new_d_allocated = false;
  size_t new_lda = a_desc->_ld, new_ldb = b_desc->_ld, new_ldc = c_desc->_ld,
         new_ldd = d_desc->_ld;
  std::vector<sycl::event> transform_events;

  if (a_desc->_order != order_t::col)
    new_lda = a_desc->_rows;
  size_t size_of_element =
      dpct::detail::library_data_size[static_cast<unsigned int>(
          a_desc->_type)] /
      8;
  new_a = ::dpct::cs::malloc(size_of_element * a_desc->_cols * new_lda, *q_ptr);
  sycl::event e_init;
  if (a_desc->_order != order_t::col)
    e_init = detail::type_dispatch<detail::matrix_transform_impl>(
        a_desc->_type, q_ptr, a_desc->_rows, a_desc->_cols, a_desc->_ld,
        a_desc->_order, (const std::int8_t *)a, new_lda, order_t::col,
        (std::int8_t *)new_a, std::vector<sycl::event>{});
  else
    e_init = ::dpct::cs::memcpy(*q_ptr, (void *)new_a, a,
                                size_of_element * a_desc->_cols * new_lda,
                                ::dpct::cs::memcpy_direction::device_to_device);

  // alpha = alpha * scale_a * scale_b
  sycl::event e_scale_new_a = detail::scale_new_a(
      q_ptr, m, k, (void *)new_a, a_type, alpha, scale_type, vector_alpha,
      device_alpha, compute_desc->_a_scale_pointer,
      compute_desc->_b_scale_pointer, {e_init});

  transform_events.push_back(e_scale_new_a);

  if (b_desc->_order != order_t::col) {
    new_ldb = b_desc->_rows;
    size_t size_of_element =
        dpct::detail::library_data_size[static_cast<unsigned int>(
            b_desc->_type)] /
        8;
    new_b =
        ::dpct::cs::malloc(size_of_element * b_desc->_cols * new_ldb, *q_ptr);
    new_b_allocated = true;
    sycl::event e = detail::type_dispatch<detail::matrix_transform_impl>(
        b_desc->_type, q_ptr, b_desc->_rows, b_desc->_cols, b_desc->_ld,
        b_desc->_order, b, new_ldb, order_t::col, const_cast<void *>(new_b),
        std::vector<sycl::event>{});
    transform_events.push_back(e);
  }

  if (!beta_is_zero && c_desc->_order != order_t::col) {
    new_ldc = c_desc->_rows;
    size_t size_of_element =
        dpct::detail::library_data_size[static_cast<unsigned int>(
            c_desc->_type)] /
        8;
    new_c =
        ::dpct::cs::malloc(size_of_element * c_desc->_cols * new_ldc, *q_ptr);
    new_c_allocated = true;
    sycl::event e = detail::type_dispatch<detail::matrix_transform_impl>(
        c_desc->_type, q_ptr, c_desc->_rows, c_desc->_cols, c_desc->_ld,
        c_desc->_order, c, new_ldc, order_t::col, const_cast<void *>(new_c),
        std::vector<sycl::event>{});
    transform_events.push_back(e);
  }

  if (d_desc->_order != order_t::col) {
    new_ldd = d_desc->_rows;
    size_t size_of_element =
        dpct::detail::library_data_size[static_cast<unsigned int>(
            d_desc->_type)] /
        8;
    new_d =
        ::dpct::cs::malloc(size_of_element * d_desc->_cols * new_ldd, *q_ptr);
    new_d_allocated = true;
  }

  // start to call oneDNN matmul primitive
  // a,d are col_major, b is row_major
  const ::dnnl::memory::dim M = m;
  const ::dnnl::memory::dim N = n;
  const ::dnnl::memory::dim K = k;

  ::dnnl::memory::dims src_dims = {M, K};
  ::dnnl::memory::dims weights_dims = {K, N};
  ::dnnl::memory::dims bias_dims = {M, N};
  ::dnnl::memory::dims dst_dims = {M, N};

  const ::dnnl::memory::dims src_strides =
      compute_desc->_trans_a == oneapi::mkl::transpose::nontrans
          ? ::dnnl::memory::dims{1, static_cast<long>(new_lda)}
          : ::dnnl::memory::dims{static_cast<long>(new_lda), 1};
  const ::dnnl::memory::dims weights_strides =
      compute_desc->_trans_b == oneapi::mkl::transpose::nontrans
          ? ::dnnl::memory::dims{1, static_cast<long>(new_ldb)}
          : ::dnnl::memory::dims{static_cast<long>(new_ldb), 1};
  const ::dnnl::memory::dims bias_strides =
      ::dnnl::memory::dims{1, static_cast<long>(new_ldc)};
  const ::dnnl::memory::dims dst_strides =
      ::dnnl::memory::dims{1, static_cast<long>(new_ldd)};

  auto src_md = ::dnnl::memory::desc(
      src_dims, dpct::dnnl::memory_desc_ext::to_dnnl_data_type(a_type),
      src_strides);
  auto weights_md = ::dnnl::memory::desc(
      weights_dims, dpct::dnnl::memory_desc_ext::to_dnnl_data_type(b_type),
      weights_strides);
  auto bias_md = ::dnnl::memory::desc(
      bias_dims, dpct::dnnl::memory_desc_ext::to_dnnl_data_type(c_type),
      bias_strides);
  auto dst_md = ::dnnl::memory::desc(
      dst_dims, dpct::dnnl::memory_desc_ext::to_dnnl_data_type(d_type),
      dst_strides);

  auto *src_mem =
      new ::dnnl::memory(src_md, handle->get_engine(), DNNL_MEMORY_NONE);
  auto *weights_mem =
      new ::dnnl::memory(weights_md, handle->get_engine(), DNNL_MEMORY_NONE);
  auto *bias_mem =
      new ::dnnl::memory(bias_md, handle->get_engine(), DNNL_MEMORY_NONE);
  auto *dst_mem =
      new ::dnnl::memory(dst_md, handle->get_engine(), DNNL_MEMORY_NONE);

#ifdef DPCT_USM_LEVEL_NONE
  detail::type_dispatch<detail::set_buffer_impl>(a_type, src_mem, new_a);
  detail::type_dispatch<detail::set_buffer_impl>(b_type, weights_mem, new_b);
  if (!beta_is_zero)
    detail::type_dispatch<detail::set_buffer_impl>(c_type, bias_mem, new_c);
  detail::type_dispatch<detail::set_buffer_impl>(d_type, dst_mem, new_d);
#else
  src_mem->set_data_handle(const_cast<void *>(new_a));
  weights_mem->set_data_handle(const_cast<void *>(new_b));
  if (!beta_is_zero)
    bias_mem->set_data_handle(const_cast<void *>(new_c));
  dst_mem->set_data_handle(new_d);
#endif

  std::unordered_map<int, ::dnnl::memory> matmul_args;
  matmul_args.insert({DNNL_ARG_SRC, *src_mem});
  matmul_args.insert({DNNL_ARG_WEIGHTS, *weights_mem});
  matmul_args.insert({DNNL_ARG_DST, *dst_mem});
  ::dnnl::primitive_attr matmul_attr;

  ::dnnl::post_ops matmul_ops;
  if (!beta_is_zero) {
    matmul_ops.append_binary(::dnnl::algorithm::binary_add, bias_md);
    matmul_args.insert(
        {DNNL_ARG_ATTR_MULTIPLE_POST_OP(matmul_ops.len() - 1) | DNNL_ARG_SRC_1,
         *bias_mem});
  }

  ::dnnl::memory *po_bias_mem = nullptr;
  auto po_bias_md = ::dnnl::memory::desc(
      ::dnnl::memory::dims{M, 1},
      dpct::dnnl::memory_desc_ext::to_dnnl_data_type(compute_desc->_bias_type),
      ::dnnl::memory::dims{1, M});
  if (compute_desc->_epilogue == epilogue_t::bias ||
      compute_desc->_epilogue == epilogue_t::gelu_bias ||
      compute_desc->_epilogue == epilogue_t::gelu_aux_bias) {
    po_bias_mem =
        new ::dnnl::memory(po_bias_md, handle->get_engine(), DNNL_MEMORY_NONE);
#ifdef DPCT_USM_LEVEL_NONE
    detail::type_dispatch<detail::set_buffer_impl>(
        compute_desc->_bias_type, po_bias_mem, compute_desc->_bias_pointer);
#else
    po_bias_mem->set_data_handle(compute_desc->_bias_pointer);
#endif
  }

  switch (compute_desc->_epilogue) {
  case epilogue_t::relu:
    matmul_ops.append_eltwise(::dnnl::algorithm::eltwise_relu, 0.f, 0.f);
    break;
  case epilogue_t::bias:
  case epilogue_t::gelu_bias: {
    matmul_ops.append_binary(::dnnl::algorithm::binary_add, po_bias_md);
    matmul_args.insert(
        {DNNL_ARG_ATTR_MULTIPLE_POST_OP(matmul_ops.len() - 1) | DNNL_ARG_SRC_1,
         *po_bias_mem});
    if (compute_desc->_epilogue == epilogue_t::gelu_bias)
      matmul_ops.append_eltwise(::dnnl::algorithm::eltwise_gelu_tanh, 0.f, 0.f);
    break;
  }
  case epilogue_t::gelu:
    matmul_ops.append_eltwise(::dnnl::algorithm::eltwise_gelu_tanh, 0.f, 0.f);
    break;
  default:
    break;
  }

  matmul_attr.set_post_ops(matmul_ops);

  auto matmul_pd = ::dnnl::matmul::primitive_desc(
      handle->get_engine(), src_md, weights_md, dst_md, matmul_attr);
  auto matmul_prim = ::dnnl::matmul(matmul_pd);
  sycl::event matmul_prim_event = ::dnnl::sycl_interop::execute(
      matmul_prim, handle->get_engine_stream(), matmul_args, transform_events);

  // post-op implemented by separate primitives
  sycl::event post_op_prim_event;
  if (compute_desc->_epilogue == epilogue_t::gelu_aux ||
      compute_desc->_epilogue == epilogue_t::gelu_aux_bias) {
    sycl::event prev_event = matmul_prim_event;
    if (compute_desc->_epilogue == epilogue_t::gelu_aux_bias) {
      auto po_bias_pd = ::dnnl::binary::primitive_desc(
          handle->get_engine(), ::dnnl::algorithm::binary_add, dst_md,
          po_bias_md, dst_md);
      auto po_bias_prim = ::dnnl::binary(po_bias_pd);
      std::unordered_map<int, ::dnnl::memory> po_bias_args;
      po_bias_args.insert({DNNL_ARG_SRC_0, *dst_mem});
      po_bias_args.insert({DNNL_ARG_SRC_1, *po_bias_mem});
      po_bias_args.insert({DNNL_ARG_DST, *dst_mem});
      sycl::event prev_event = ::dnnl::sycl_interop::execute(
          po_bias_prim, handle->get_engine_stream(), po_bias_args,
          {matmul_prim_event});
    }
    size_t size_of_element =
        dpct::detail::library_data_size[static_cast<unsigned int>(
            d_desc->_type)] /
        8;
    sycl::event copy_e = dpct::blas::matrix_mem_copy_async(
        compute_desc->_epilogue_aux_pointer, new_d,
        compute_desc->_epilogue_aux_ld, d_desc->_ld, m, n, size_of_element,
        ::dpct::cs::memcpy_direction::automatic, *q_ptr, {prev_event});

    auto gelu_pd = ::dnnl::eltwise_forward::primitive_desc(
        handle->get_engine(), ::dnnl::prop_kind::forward_training,
        ::dnnl::algorithm::eltwise_gelu_tanh, dst_md, dst_md);
    auto gelu_prim = ::dnnl::eltwise_forward(gelu_pd);
    std::unordered_map<int, ::dnnl::memory> gelu_args;
    gelu_args.insert({DNNL_ARG_SRC, *dst_mem});
    gelu_args.insert({DNNL_ARG_DST, *dst_mem});
    post_op_prim_event = ::dnnl::sycl_interop::execute(
        gelu_prim, handle->get_engine_stream(), gelu_args, {copy_e});
  }

  // end of calling oneDNN

  sycl::event absmax_d_event;
  if (auto absmax_ptr = compute_desc->_absmax_d_pointer) {
    absmax_d_event = detail::type_dispatch<detail::absmax_impl>(
        d_desc->_type, absmax_ptr, new_d, new_ldd, d_desc->_rows, d_desc->_cols,
        q_ptr, std::vector<sycl::event>{matmul_prim_event, post_op_prim_event});
  }

  sycl::event scale_d_event;
  if (auto d_scale_ptr = compute_desc->_d_scale_pointer) {
    scale_d_event = detail::type_dispatch<detail::scale_d_impl>(
        d_desc->_type, d_scale_ptr, new_d, new_ldd, d_desc->_rows,
        d_desc->_cols, q_ptr, compute_desc->_scale_type,
        std::vector<sycl::event>{matmul_prim_event, absmax_d_event,
                                 post_op_prim_event});
  }

  sycl::event transform_d_event;
  if (d_desc->_order != order_t::col) {
    detail::type_dispatch<detail::matrix_transform_impl>(
        d_desc->_type, q_ptr, d_desc->_rows, d_desc->_cols, new_ldd,
        order_t::col, new_d, d_desc->_ld, d_desc->_order, d,
        std::vector<sycl::event>{matmul_prim_event, absmax_d_event,
                                 post_op_prim_event});
  }

  sycl::event free_event = q_ptr->submit([&](sycl::handler &cgh) {
    cgh.depends_on({transform_d_event, matmul_prim_event, absmax_d_event,
                    post_op_prim_event});
    cgh.host_task([=] {
      delete src_mem;
      delete weights_mem;
      delete bias_mem;
      delete dst_mem;
      if (po_bias_mem)
        delete po_bias_mem;
      ::dpct::cs::free((void *)new_a, *q_ptr);
      if (new_b_allocated)
        ::dpct::cs::free((void *)new_b, *q_ptr);
      if (new_c_allocated)
        ::dpct::cs::free((void *)new_c, *q_ptr);
      if (new_d_allocated)
        ::dpct::cs::free((void *)new_d, *q_ptr);
    });
  });
  return free_event;
}

class transform_desc_t {
public:
  enum class attribute { scale_type, pointer_mode, trans_a, trans_b };

  transform_desc_t(library_data_t scale_type) : _scale_type(scale_type) {}
  void set_attribute(attribute attr, const void *mem) {
    get_set_attr<true>(attr, const_cast<void *>(mem));
  }
  void get_attribute(attribute attr, void *mem) {
    get_set_attr<false>(attr, mem);
  }

private:
  template <bool is_set> void get_set_attr(attribute attr, void *mem) {
#define CASE(tag)                                                              \
  case attribute::tag:                                                         \
    if constexpr (is_set) {                                                    \
      _##tag = *static_cast<decltype(_##tag) *>(mem);                          \
    } else {                                                                   \
      *static_cast<decltype(_##tag) *>(mem) = _##tag;                          \
    }                                                                          \
    break;
    switch (attr) {
      CASE(scale_type)
      CASE(pointer_mode)
      CASE(trans_a)
      CASE(trans_b)
    }
#undef CASE
  }

  library_data_t _scale_type;
  pointer_mode_t _pointer_mode = pointer_mode_t::host;
  oneapi::mkl::transpose _trans_a = oneapi::mkl::transpose::nontrans;
  oneapi::mkl::transpose _trans_b = oneapi::mkl::transpose::nontrans;

  friend sycl::event
  matrix_transform(transform_desc_ptr transform_desc, const void *alpha,
                   const void *a, matrix_layout_ptr a_desc, const void *beta,
                   const void *b, matrix_layout_ptr b_desc, void *c,
                   matrix_layout_ptr c_desc, ::dpct::cs::queue_ptr q_ptr);
};

/// This function does operation:
/// C = alpha*transformation(A) + beta*transformation(B).
/// The "transformation" includes matrix transpose and layout conversion.
/// Currently it only supports alpha==1 && beta==0.
/// \param [in] transform_desc Describe the transformation.
/// \param [in] alpha Scaling factor alpha.
/// \param [in] a Input matrix A.
/// \param [in] a_desc Describe the matrix A.
/// \param [in] beta Scaling factor beta.
/// \param [in] b Input matrix B.
/// \param [in] b_desc Describe the matrix B.
/// \param [out] c Output matrix C.
/// \param [in] c_desc Describe the matrix C.
/// \param [in] q_ptr The queue where the routine should be executed.
inline sycl::event matrix_transform(transform_desc_ptr transform_desc,
                                    const void *alpha, const void *a,
                                    matrix_layout_ptr a_desc, const void *beta,
                                    const void *b, matrix_layout_ptr b_desc,
                                    void *c, matrix_layout_ptr c_desc,
                                    ::dpct::cs::queue_ptr q_ptr) {
  if (!q_ptr)
    q_ptr = &::dpct::cs::get_default_queue();

  if (transform_desc->_pointer_mode != pointer_mode_t::host) {
    throw std::runtime_error(
        "dpct::blas_gemm::experimental::matrix_transform() "
        "only supports pointer_mode_t::host as pointer_mode currently.");
  }
  if (transform_desc->_scale_type != library_data_t::real_float) {
    throw std::runtime_error(
        "dpct::blas_gemm::experimental::matrix_transform() "
        "only supports library_data_t::real_float as scale_type currently.");
  }

  if (alpha != nullptr) {
    if (1.0f != *reinterpret_cast<const float *>(alpha))
      throw std::runtime_error(
          "dpct::blas_gemm::experimental::matrix_transform() does not "
          "support non-one alpha currently.");
  }

  if (beta != nullptr) {
    if (0.0f != *reinterpret_cast<const float *>(beta))
      throw std::runtime_error(
          "dpct::blas_gemm::experimental::matrix_transform() does not "
          "support non-zero beta currently.");
  }

  if (b != nullptr) {
    throw std::runtime_error(
        "dpct::blas_gemm::experimental::matrix_transform() does not "
        "support matrix B currently.");
  }

  if ((a_desc->_type != library_data_t::real_int8 ||
       c_desc->_type != library_data_t::real_int8) &&
      (a_desc->_type != library_data_t::real_int32 ||
       c_desc->_type != library_data_t::real_int32)) {
    throw std::runtime_error(
        "dpct::blas_gemm::experimental::matrix_transform() only supports "
        "combinations of data types: a_type==real_int8&&c_type==real_int8, "
        "a_type==real_int32&&c_type==real_int32.");
  }

  return detail::type_dispatch<detail::matrix_transform_impl>(
      a_desc->_type, q_ptr, a_desc->_rows, a_desc->_cols, a_desc->_ld,
      a_desc->_order, a, c_desc->_ld, c_desc->_order, c,
      std::vector<sycl::event>{});
}
} // namespace experimental
} // namespace blas_gemm
} // namespace dpct
#endif // __DPCT_BLAS_GEMM_UTILS_HPP__
