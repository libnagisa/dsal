#pragma push_macro("NAGISA_DSAL_THROW")
#if __cpp_exceptions
#	define NAGISA_DSAL_THROW(error) throw error
#else
#	define NAGISA_DSAL_THROW(error) ::std::abort()
#endif