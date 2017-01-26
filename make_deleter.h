#pragma once

namespace details
{
   template<typename ReturnType, typename Argument>
   constexpr ReturnType function_pointer_return_type(ReturnType(*)(Argument));

   template<typename ReturnType, typename Argument>
   constexpr Argument function_pointer_argument(ReturnType(*)(Argument));

   template<typename ReturnType, typename Argument>
   constexpr ReturnType function_pointer_return_type(ReturnType(Argument));

   template<typename ReturnType, typename Argument>
   constexpr Argument function_pointer_argument(ReturnType(Argument));

   template<typename ReturnType, typename Argument, ReturnType(*func_ptr)(Argument)>
   struct deleter
   {
      void operator() (Argument arg)
      {
         func_ptr(arg);
      }
   };
}

#define MAKE_DELETER(func_ptr)                                                      \
	::details::deleter<decltype(::details::function_pointer_return_type(func_ptr)),  \
                      decltype(::details::function_pointer_argument(func_ptr)),     \
                      func_ptr>()

template<typename Func, Func* func_ptr>
auto make_deleter()
{
   return details::deleter<
      decltype(details::function_pointer_return_type(func_ptr)),
      decltype(details::function_pointer_argument(func_ptr)),
      func_ptr
   >();
}
