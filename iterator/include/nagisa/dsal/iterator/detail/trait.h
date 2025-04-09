#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN


template<class T>
concept primary_iterator_trait =
#if defined(__GLIBCXX__)
::std::__primary_traits_iter<T>
#elif defined(__LIBCXX__)
::std::__is_primary_template<T>::value
#elif defined(_MSVC_STL_UPDATE)
::std::_Is_from_primary<T>
#else
#	error "fuck you"
#endif
;

template<class T>
using iter_traits = ::std::conditional_t<primary_iterator_trait<::std::iterator_traits<T>>, T, ::std::iterator_traits<T>>;

template<class T>
consteval auto iter_concept() noexcept
{
	using ::std::type_identity;
	if constexpr (requires{typename iter_traits<T>::iterator_concept; })
		return type_identity<typename iter_traits<T>::iterator_concept>{};
	else if constexpr (requires {typename iter_traits<T>::iterator_category; })
		return type_identity<typename iter_traits<T>::iterator_category>{};
	else if constexpr (primary_iterator_trait<::std::iterator_traits<T>>)
		return type_identity<::std::random_access_iterator_tag>{};
	else
		return type_identity<void>{};
}

template<class T>
using iter_concept_t = typename decltype(details::iter_concept<T>())::type;

template<class T>
consteval auto iter_value() noexcept
{
	using ::std::type_identity;
	if constexpr (requires { typename ::std::iter_value_t<T>; })
		return type_identity<::std::iter_value_t<T>>{};
	else if constexpr (requires { typename ::std::iter_reference_t<T>; })
		return type_identity<::std::remove_cvref_t<::std::iter_reference_t<T>>>{};
	else
		return type_identity<void>{};
}

template<class T>
using iter_value_t = typename decltype(details::iter_value<T>())::type;

template<class T>
consteval auto iter_difference() noexcept
{
	using ::std::type_identity;
	if constexpr (requires { typename ::std::iter_difference_t<T>; })
		return type_identity<::std::iter_difference_t<T>>{};
	else
		return type_identity<void>{};
}

template<class T>
using iter_difference_t = typename decltype(details::iter_difference<T>())::type;


NAGISA_BUILD_LIB_DETAIL_END