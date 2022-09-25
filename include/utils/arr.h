#ifndef rascal_arr_h
#define rascal_arr_h

#include "core.h"

// genericized array operations (NB: not portable, change these to macros at some point)
static inline size_t pad_alist_size(size_t oldl, size_t newl, size_t oldc)
{
  if (oldc >= newl && newl >= (oldc>>1))			
    return oldc;							
  arity_t newc = ((size_t)newl+(newl>>3)+6)&~(size_t)3;	
  if (newl - oldl > newc - oldc)				
    newc = ((size_t)newl+3)&~(size_t)3;			       
  return newc;
}

static inline size_t pad_stack_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  size_t newc = oldc;
  if (newc < minc)
    newc = minc;
  if (newl > oldl)						
    while (newl > newc)				       
      newc <<= 1;
  else if (oldl > newl)
    while (newc > minc && newl < (newc >> 1))
      newc >>= 1;
  return newc;
}

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
	  *buf = _array_->vals[_n_];				\
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
	  *buf = _array_->vals[_n_];		\
	_array_->vals[_n_] = _val_;		\
	_r_ = true;				\
      }						\
    _r_;					\
  })

#define apop(array, buf)			\
  ({						\
    typeof(array) _array_ = array;		\
    typeof(buf) _buf_ = buf;			\
    arity_t _n_ = _array_->len;			\
    bool _r_ = false;				\
    if (_n_)					\
      {						\
	if (_buf_)				\
	  {					\
	    *_buf_ = _array_->vals[_n_-1];	\
	  }					\
	_r_ = true;				\
	resize((obj_t*)_array_, _n_, _n_-1);	\
      }						\
    _r_;					\
  })

#define push(array, val)			\
  ({						\
    typeof(array) _array_ = array;		\
    typeof(val) _val_ = val;			\
    arity_t _n_ = _array_->len;			\
    resize( (obj_t*)_array_, _n_, _n_+1 );	\
    _array_->vals[_n_] = _val_;			\
    _n_;					\
  })

#define pushn(array, n)				\
  ({						\
    typeof(array) _array_ = array;		\
    typeof(n) _n_ = n;				\
    arity_t _r_ = _array_->len;			\
    resize( (obj_t*)_array_, _r_, _r_+_n_);	\
    _r_;					\
  })

#define pop(array)						\
  ({								\
    typeof(array) _array_ = array;				\
    arity_t _n_ = _array_->len;					\
    assert(_n_ > 0);						\
    typeof(*(_array_->vals)) _val_ = _array_->vals[_n_];	\
    resize( (obj_t*)_array_, _n_, _n_-1);			\
    _val_;							\
  })

#define popn(array, n)							\
  ({									\
    typeof(array) _array_ = array;					\
    arity_t _n_ = n;							\
    arity_t _i_ = _array_->len;						\
    assert(_i_ > 0);							\
    assert(_i_ >= _n_);							\
    typeof(*(_array_->vals)) _val_ = _array_->vals[_i_-1];		\
    resize((obj_t*)_array_, _i_, _i_-_n_);				\
    _val_;								\
  })

#define readn(array, src, n)					\
  ({								\
    typeof(array) _array_ = array;				\
    typeof(src) _src_ = src;					\
    typeof(n) _n_ = n;						\
    resize( (obj_t*)_array_, _array_->len, _array_->len+_n_ );	\
    memcpy( _array_->vals, _src_, _n_ * sizeof(*_src_));	\
    _array_->len;						\
  })

#define writen(dst, array, n)					\
  ({								\
    typeof(array) _array_ = array;				\
    typeof(dst) _dst_ = dst;					\
    arity_t _n_ = n;						\
    _n_ = min( _n_, _array_->len );				\
    memcpy( _dst_, _array_->vals, _n_ * sizeof(*_src_));	\
    _n_;							\
  })

#endif
