/*
	Ara 3d Array Library
	Copyright 2018, Ara 3D
	MIT Licenese

	This is a header-only library of array data structures that providing a uniform array interface to different layouts 
	of data in memory, as well as to purely computed data. This is a single header file with no other dependencies (including STL)
	which means it is fast to compile, and easy to include in different projects. 

	This library was motivated by a need to work with many different geometry and math libraries in an efficient way, while 
	eliminating redudnancy of having to write different algorithms. 

	The design goals for this library:

		1. is to provide an efficient abstraction that is similar to std::vector for usage when data is already
			allocated in memory and there is no need to support dynamic growing or shrinking of the array.

		2. provide a compatible abstraction for functionally computed arrays that had O(1) memory consumption patterns
			* continous monotonically increasing values
			* repeated values
			* random sequeneces
			* maps

	Each array data structure provide the same interface: begin()/end() for using range-based for loops, a size() function, and an
	indexing operator. The iterators used are random-access forward only iterators. The data structures are compliant to a substantial 
	subset of the standard. Full standards compliance is possible, but would make the code a lot harder to read, validate, and longer to 
	compile. Only the array data structure allocates and deallocates memory for storage of elements.  
	
	All data structures have the same constructor signature:
		begin - an iterator to the beginning of data 
		size - the number of values in the array 

	The primary data structures are: 
		* array - an array container of contiguous data in memory with ownership semantics (derived from array_view)
		* array_view - a view into contiguous memory without ownership semantics
		* const_array_view - a readonly view of contiguous memory without ownership semantics
		* array_slice - a wrapper that provides access to a range of values in an existing array view   
		* const_array_stride - a readonly wrapper around an array that jumps over N elements at a time 
		* array_mem_stride - an array of values in memory that are a fixed number of bytes apart	
		* const_array_mem_stride - a read only array of values in memory that are a fixed number of bytes apart
		* func_array - an array that generates values on demand using a function 

	This library works well with "ara3d/array_ops.h" and "ara3d/geometry.h"
*/
#pragma once

namespace ara3d
{
	typedef __int64 ptrdiff_t;

	// Iterator for accessing of items at fixed byte offsets in memory 
	template<typename T, size_t OffsetN = sizeof(T)>
	class mem_stride_iterator
	{
		typedef T ValueType;

		const char* _data;

		const T& operator*() const { return *(T*)_data; }
		T& operator*() { return *(T*)_data; }
		mem_stride_iterator(const char* data = nullptr) : _data(data) { }
		bool operator==(const mem_stride_iterator iter) const { return _data == iter._data; }
		bool operator!=(const mem_stride_iterator iter) const { return _data != iter._data; }
		mem_stride_iterator& operator++() { _data += OffsetN; return *this; }
		mem_stride_iterator operator++(int) { mem_stride_iterator r = *this; (*this)++; return r; }
		mem_stride_iterator operator+(size_t n) const { return mem_stride_iterator(_data + OffsetN * n); }
		mem_stride_iterator& operator+=(size_t n) { _data += OffsetN * n; return *this; }
		ptrdiff_t operator-(const mem_stride_iterator& iter) const { return _data - iter._data; }
		const T& operator[](size_t n) const { return *(const T*)(_data + OffsetN * n); }
		T& operator[](size_t n) { return *(T*)(_data + OffsetN * n); }
	};

	// Iterator for read-only access of items at fixed byte offsets in memory 
	template<typename T, size_t OffsetN = sizeof(T)>
	class const_mem_stride_iterator
	{
		typedef T ValueType;

		const char* _data;

		const_mem_stride_iterator(const char* data = nullptr) : _data(data) { }
		const_mem_stride_iterator(mem_stride_iterator<T, OffsetN> other) : _data(other._data) { }
		const T& operator*() const { return *(T*)_data; }
		bool operator==(const const_mem_stride_iterator iter) const { return _data == iter._data; }
		bool operator!=(const const_mem_stride_iterator iter) const { return _data != iter._data; }
		const_mem_stride_iterator& operator++() { _data += OffsetN; return *this; }
		const_mem_stride_iterator operator++(int) { const_mem_stride_iterator r = *this; (*this)++; return r; }
		const_mem_stride_iterator operator+(size_t n) const { return const_mem_stride_iterator(_data + OffsetN * n); }
		const_mem_stride_iterator& operator+=(size_t n) { _data += OffsetN * n; return *this; }
		ptrdiff_t operator-(const const_mem_stride_iterator& iter) const { return _data - iter._data; }
		const T& operator[](size_t n) const { return *(const T*)(_data + OffsetN * n); }
	};

	// Iterator that generating items as needed using a function 
	template<typename F>
	struct func_array_iterator
	{
		typedef typename F::result_type value_type;
		typedef F func_type;

		F _func;
		size_t _i;

		func_array_iterator(const F& func, size_t i = 0) : _func(func), _i(i) { }
		value_type operator*() const { return _func(_i); }
		bool operator==(const func_array_iterator iter) const { return _i == iter._i; }
		bool operator!=(const func_array_iterator iter) const { return _i != iter._i; }
		func_array_iterator& operator++() { return this->operator+=(1); }
		func_array_iterator operator++(int) { func_array_iterator r = *this; (*this)++; return r; }
		func_array_iterator& operator+=(size_t n) { _i += n; return *this; }
		func_array_iterator operator+(size_t n) const { return func_array_iterator(_func, _i + n); }
		ptrdiff_t operator-(const func_array_iterator& iter) const { return _i - iter._i; }
		value_type operator[](size_t n) const { return _func(_i + n); }
	};

	// A wrapper around an existing iterator that advances it by N items at a time. 
	// There is no non-const version of this iterator, as it would add complexity to the stride operation
	template<typename IterT>
	struct const_strided_iterator 
	{
		typedef IterT iterator;
		typedef typename iterator::value_type value_type;

		iterator _iter;
		size_t _stride;

		const_strided_iterator(const iterator& iter, size_t stride) : _iter(iter), _stride(stride) { }
		value_type operator*() const { return *_iter; }
		bool operator==(const const_strided_iterator iter) const { return _iter == iter._iter; }
		bool operator!=(const const_strided_iterator iter) const { return _iter != iter._iter; }
		const_strided_iterator& operator++() { _iter += _stride; return *this; }
		const_strided_iterator operator++(int) { const_strided_iterator r = *this; (*this)++; return r; }
		const_strided_iterator& operator+=(size_t n) { _iter += _stride * n; return *this; }
		const_strided_iterator operator+(size_t n) const { return const_strided_iterator(_iter + _stride * n, _stride); }
		ptrdiff_t operator-(const const_strided_iterator& iter) const { return (_iter - iter._iter) / _stride; }
		value_type operator[](size_t n) const { return _iter[n * _stride]; }
	};

	// The base class of all const array implementations 
	template<
		typename T, 
		typename IterT
	>
	struct const_array_base 
	{
		typedef IterT iterator;
		typedef IterT const_iterator;
		typedef T value_type;
		typedef size_t size_type;

		iterator _iter;
		size_t _size;

		const_array_base(iterator begin, size_t size = 0) : _iter(begin), _size(size) { }
		iterator begin() const { return _iter; }
		iterator end() const { return begin() + size(); }
		const value_type& operator[](size_t n) const { return begin()[n]; }
		size_type size() const { return _size; }
		bool empty() const { return size() == 0; }
	};

	// The base class of all array implementations 
	template<
		typename T, 
		typename IterT, 
		typename ConstIterT
	> 
	struct array_base 
	{
		typedef ConstIterT const_iterator;
		typedef IterT iterator;
		typedef T value_type;
		typedef size_t size_type;

		iterator _iter;
		size_t _size;

		array_base(iterator begin = iterator(), size_t size = 0) : _iter(begin), _size(size) { }

		iterator begin() { return _iter; }
		iterator end() { return begin() + size(); }
		const_iterator begin() const { return _iter; }
		const_iterator end() const { return begin() + size(); }
		value_type& operator[](size_t n) { return begin()[n]; }
		const value_type& operator[](size_t n) const { return begin()[n]; }
		size_type size() const { return _size; }
		bool empty() const { return size() == 0; }
	};

	// Represents N elements from any array compatible type (works with vectors)
	template<
		typename ArrayT, 
		typename ValueT = typename ArrayT::value_type, 
		typename IterT = typename ArrayT::iterator, 
		typename ConstIterT = typename ArrayT::const_iterator, 
		typename BaseT = array_base<ValueT, IterT, ConstIterT>
	>
	struct array_slice : public BaseT
	{
		array_slice(IterT begin = IterT(), size_t size = 0) : BaseT(begin, size) { }
	};

	// Represents N mutable elements from any array compatible type (works with vectors)
	template<
		typename ArrayT, 
		typename ValueT = typename ArrayT::value_type, 
		typename IterT = typename ArrayT::iterator, 
		typename BaseT = const_array_base<ValueT, IterT>
	>
	struct const_array_slice : public BaseT
	{
		const_array_slice(IterT begin = IterT(), size_t size = 0) : BaseT(begin, size) { }
	};

	// Strides over elements in an array. 	
	template<
		typename ArrayT, 
		typename ValueT = typename ArrayT::value_type, 
		typename IterT = const_strided_iterator<typename ArrayT::iterator>, 
		typename BaseT = const_array_base<ValueT, IterT>
	>
	struct const_array_stride : public BaseT
	{
		const_array_stride(typename ArrayT::iterator begin = ArrayT::iterator(), size_t size = 0, size_t stride = 0) : const_array_base(IterT(begin, stride), size) { }
	};

	// A mutable view into a contiguous buffer of data without ownership semantics and which can be indexed and sliced. 
	template<
		typename ValueT, 
		typename IterT = ValueT*, 
		typename BaseT = array_base<ValueT, IterT, const ValueT*>
	>
	struct array_view : public BaseT
	{
		array_view(IterT begin = IterT(), size_t size = 0) : BaseT(begin, size) { }
	};

	// A non-mutable view into a contiguous buffer of data of that has no ownership semantics but which can be indexed and sliced. 
	template<
		typename ValueT, 
		typename IterT = const ValueT* , 
		typename BaseT = const_array_base<ValueT, IterT>
	>
	struct const_array_view : public BaseT
	{
		const_array_view(IterT begin = IterT(), size_t size = 0) : BaseT(begin, size) { }
	};

	// An immutable view of a set of values that are in a contigous block of memory, but offset from each other a fixed number of bytes	
	template<
		typename ValueT, size_t OffsetN, 
		typename IterT = const_mem_stride_iterator <ValueT , OffsetN > , 
		typename BaseT = const_array_base<ValueT, IterT >
	>
	struct const_array_mem_stride : public BaseT
	{
		const_array_mem_stride(const ValueT* begin = nullptr, size_t size = 0) : BaseT(IterT(begin), size) { }
	};

	// An mutable view of a set of values that are in a contigous block of memory, but offset from each other an arbitrary number of bytes
	template<
		typename ValueT, 
		size_t OffsetN, 
		typename IterT = mem_stride_iterator <ValueT, OffsetN >, 
		typename ConstIterT = const_mem_stride_iterator <ValueT, OffsetN >, 
		typename BaseT = array_base<ValueT, IterT, ConstIterT >
	>
	struct array_mem_stride : public BaseT, ConstIterT
	{
		array_mem_stride(const ValueT* begin = nullptr, size_t size = 0) : BaseT(IterT(begin), size) { }
	};
	
	// Provides an array interface around a function and a size. Requires functors or std::function to work. 
	template<typename F, typename ValueT = typename F::result_type, typename IterT = func_array_iterator<F>, typename BaseT = const_array_base<ValueT, IterT>>
	struct func_array : public BaseT 
	{	
		func_array(F func, size_t size) : BaseT(IterT(func), size) { }
	};

	// An array container (owns memory) with a run-time defined size. 
	template<typename T, typename BaseT = array_view<T>>
	struct array : public BaseT
	{
		array(size_t size = 0) : BaseT(new T[size], size) { }	
		~array() { delete[] BaseT::begin(); }
	};

	// An array of bytes 
	typedef array<unsigned char> buffer;
}