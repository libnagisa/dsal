#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<class C, class V, class D>
struct iterator_interface_base
{
	using iterator_concept = C;
	using value_type = V;
	using difference_type = D;
};
template<class C, class V>
struct iterator_interface_base<C, V, void>
{
	using iterator_concept = C;
	using value_type = V;
};
template<class C, class D>
struct iterator_interface_base<C, void, D>
{
	using iterator_concept = C;
	using difference_type = D;
};
template<class V, class D>
struct iterator_interface_base<void, V, D>
{
	using value_type = V;
	using difference_type = D;
};
template<class C>
struct iterator_interface_base<C, void, void>
{
	using iterator_concept = C;
};
template<class V>
struct iterator_interface_base<void, V, void>
{
	using value_type = V;
};
template<class D>
struct iterator_interface_base<void, void, D>
{
	using difference_type = D;
};
template<>
struct iterator_interface_base<void, void, void> {};

/// \brief Provides a customizable interface for iterators.
/// \tparam Derived The derived class implementing the iterator. model ::std::derived_from<Derived, iterator_interface>.
/// \tparam IteratorConcept The iterator concept (e.g., input_iterator, output_iterator).
/// \tparam DifferenceType The difference type of the iterator (void will deduce `difference_type` or `operator-`'s return type).
/// \tparam ValueType The value type of the iterator (void will deduce `value_type` or `::std::remove_cvref_t<::std::iter_reference_t>).
template<class Derived, class IteratorConcept = void, class DifferenceType = void, class ValueType = void>
struct iterator_interface : iterator_interface_base<IteratorConcept, ValueType, DifferenceType>
{
private:
	using derived_type = Derived;
	constexpr auto&& derived() noexcept { return static_cast<Derived&>(*this); }
	constexpr auto&& derived() const noexcept { return static_cast<Derived const&>(*this); }
public:
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
