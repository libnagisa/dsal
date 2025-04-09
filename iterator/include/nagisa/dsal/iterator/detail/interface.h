#pragma once

#include "./optional_type.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<class Derived, class IteratorConcept, class ValueType = void, class DifferenceType = void>
struct iterator_interface : iterator_optional_base<ValueType, DifferenceType>
{
private:
	using derived_type = Derived;
	constexpr auto&& derived() noexcept { return static_cast<Derived&>(*this); }
	constexpr auto&& derived() const noexcept { return static_cast<Derived const&>(*this); }
public:
	using iterator_concept = IteratorConcept;

	constexpr auto operator->() noexcept(noexcept(::std::addressof(*derived())))
		requires ::std::is_reference_v<::std::iter_reference_t<derived_type>>
	{
		return ::std::addressof(*derived());
	}
	constexpr auto operator->() const noexcept(noexcept(::std::addressof(*derived())))
		requires ::std::is_reference_v<::std::iter_reference_t<derived_type>>
	{
		return ::std::addressof(*derived());
	}
};

NAGISA_BUILD_LIB_DETAIL_END
