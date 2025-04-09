#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<class V, class D>
struct iterator_optional_base
{
	using value_type = V;
	using difference_type = D;
};

template<class V>
struct iterator_optional_base<V, void>
{
	using value_type = V;
};
template<class D>
struct iterator_optional_base<void, D>
{
	using difference_type = D;
};
template<>
struct iterator_optional_base<void, void>
{

};

NAGISA_BUILD_LIB_DETAIL_END