#pragma once

#include <stdexcept>
#include <cassert>

//如果条件expr满足时就抛出异常，并将信息传递给what
namespace mystl
{

#define MYSTL_DEBUG(expr)	\
	assert(expr)

#define THROW_LENGTH_ERROR_IF(expr,what)	\
	if((expr)) throw std::length_error(what)

#define THROW_OUT_OF_RANGE_IF(expr,what)	\
	if((expr)) throw std::out_of_range(what)

#define THROW_RUNTIME_ERROR_IF(expr,what)	\
	if((expr)) throw std::runtime_error(what)
}