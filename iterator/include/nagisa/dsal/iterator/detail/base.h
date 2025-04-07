#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

// difference_type is ::std::make_signed_t<decltype(::std::declval<derived_type>() - ::std::declval<derived_type>())>
// so need `requires { ::std::declval<derived_type>() - ::std::declval<derived_type>(); }` is true
struct deduce_difference_type;

template<class DifferenceType>
struct iterator_base { using difference_type = DifferenceType; };
template<>
struct iterator_base<deduce_difference_type> {};


template<class DifferenceType>
void derived_iterator_helper(iterator_base<DifferenceType> const&);

template<typename D>
concept derived_iter = requires (D d) { details::derived_iterator_helper(d); };

template<typename D>
constexpr auto operator+(D it, ::std::iter_difference_t<D> n) noexcept(noexcept(it += n))
	requires derived_iter<D>&& requires { it += n; }
{
	return it += n;
}
template<typename D>
constexpr auto operator+(::std::iter_difference_t<D> n, D it) noexcept(noexcept(it += n))
	requires derived_iter<D>&& requires { it += n; }
{
	return it += n;
}
template<typename D>
constexpr auto operator-(D it, ::std::iter_difference_t<D> n) noexcept(noexcept(it += -n))
	requires derived_iter<D>&& requires { it += -n; }
{
	return it += -n;
}

constexpr auto operator==(derived_iter auto&& lhs, derived_iter auto&& rhs)
noexcept(noexcept((::std::forward<decltype(lhs)>(lhs) - ::std::forward<decltype(rhs)>(rhs)) == 0))
	requires requires { ::std::forward<decltype(lhs)>(lhs) - ::std::forward<decltype(rhs)>(rhs); }
{
	return (::std::forward<decltype(lhs)>(lhs) - ::std::forward<decltype(rhs)>(rhs)) == 0;
}

constexpr auto operator<=>(derived_iter auto lhs, derived_iter auto rhs)
	requires requires { rhs - lhs; }
{
	auto diff = rhs - lhs;
	return diff < 0 ? ::std::strong_ordering::less :
		0 < diff ? ::std::strong_ordering::greater :
		::std::strong_ordering::equal;
}

NAGISA_BUILD_LIB_DETAIL_END