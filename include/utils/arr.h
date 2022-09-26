#ifndef rascal_arr_h
#define rascal_arr_h

#include "core.h"

typedef struct array_t
{
  arity_t len;
  arity_t cap;

  void   *data;
} array_t;

// generic utilities
size_t pad_alist_size(size_t oldl, size_t newl, size_t oldc);
size_t pad_stack_size(size_t oldl, size_t newl, size_t oldc, size_t minc);
void   resize_array(array_t *array, size_t newl, size_t elsize);

// genericized array operations (NB: not portable, change these to macros at some point)

#define aref(array, n, buf)					\
  ({								\
    typeof(array) _array_ = array;				\
    int _n_ = n; 						\
    bool _r_ = false;						\
    if (_n_<0)							\
      _n_ += _array_->len;					\
    if (_n_ => 0 && _n_<_array_->len)				\
      {								\
	if (buf)						\
	  *buf = _array_->data[_n_];				\
	_r_ = true;						\
      }								\
    _r_;							\
  })

#define aset(array, n, val, buf)		\
  ({						\
    typeof(array) _array_ = array;		\
    typeof(val) _val_ = _val_;			\
    int _n_ = n;				\
    bool _r_ = false;				\
    if (_n_<0)					\
      _n_ += _array_->len;			\
    if (_n_ >= 0 && _n_<_array_->len)		\
      {						\
	if (buf)				\
	  *buf = _array_->data[_n_];		\
	_array_->data[_n_] = _val_;		\
	_r_ = true;				\
      }						\
    _r_;					\
  })

#define apop(array, buf)						\
  ({									\
    typeof(array) _array_ = array;					\
    typeof(buf) _buf_ = buf;						\
    arity_t _n_ = _array_->len;						\
    bool _r_ = false;							\
    if (_n_)								\
      {									\
	if (_buf_)							\
	  {								\
	    *_buf_ = _array_->data[_n_-1];				\
	  }								\
	_r_ = true;							\
	resize_array((array_t*)_array_,					\
		     _n_-1,						\
		     sizeof(_array_->data[0]));				\
      }									\
    _r_;								\
  })

#define push(array, val)			\
  ({						\
    typeof(array) _array_ = array;		\
    typeof(val) _val_ = val;			\
    arity_t _n_ = _array_->len;			\
    resize_array( (array_t*)_array_,		\
		  _n_+1,			\
		  sizeof(_array_->data[0]));	\
    _array_->data[_n_] = _val_;			\
    _n_;					\
  })

#define pushn(array, n)				\
  ({						\
    typeof(array) _array_ = array;		\
    typeof(n) _n_ = n;				\
    arity_t _r_ = _array_->len;			\
    resize_array( (array_t*)_array_,		\
		  _r_+_n_,			\
		  sizeof(_array_->data[0]));	\
    _r_;					\
  })

#define pop(array)						\
  ({								\
    typeof(array) _array_ = array;				\
    arity_t _n_ = _array_->len;					\
    assert(_n_ > 0);						\
    typeof(*(_array_->data)) _val_ = _array_->data[_n_];	\
    resize_array( (array_t*)_array_,				\
		  _n_-1,					\
		  sizeof(_val_) );				\
    _val_;							\
  })

#define popn(array, n)							\
  ({									\
    typeof(array) _array_ = array;					\
    arity_t _n_ = n;							\
    arity_t _i_ = _array_->len;						\
    assert(_i_ > 0);							\
    assert(_i_ >= _n_);							\
    typeof(*(_array_->data)) _val_ = _array_->data[_i_-1];		\
    resize_array((array_t*)_array_,					\
		 _i_-_n_,						\
		 sizeof(_val_) );					\
    _val_;								\
  })

#define readn(array, src, n)					\
  ({								\
    typeof(array) _array_ = array;				\
    typeof(src) _src_ = src;					\
    typeof(n) _n_ = n;						\
    resize_array( (array_t*)_array_,				\
		  _array_->len+_n_,				\
		  sizeof(*_src_));				\
    memcpy( _array_->data, _src_, _n_ * sizeof(*_src_));	\
    _array_->len;						\
  })

#define writen(dst, array, n)					\
  ({								\
    typeof(array) _array_ = array;				\
    typeof(dst) _dst_ = dst;					\
    arity_t _n_ = n;						\
    _n_ = min( _n_, _array_->len );				\
    memcpy( _dst_, _array_->data, _n_ * sizeof(*_src_));	\
    _n_;							\
  })

#endif
