#pragma once

namespace details
{
   template<typename ReturnType, typename Argument>
   constexpr auto function_pointer_return_type(ReturnType(*)(Argument))->ReturnType;

   template<typename ReturnType, typename Argument>
   constexpr auto function_pointer_argument(ReturnType(*)(Argument))->Argument;

   template<typename ReturnType, typename Argument>
   constexpr auto function_pointer_return_type(ReturnType(Argument))->ReturnType;

   template<typename ReturnType, typename Argument>
   constexpr auto function_pointer_argument(ReturnType(Argument))->Argument;

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

template<typename FuncPtr, FuncPtr func_ptr>
details::deleter<
   decltype(function_pointer_return_type(func_ptr)),
   decltype(function_pointer_argument(func_ptr)), 
   func_ptr
> make_deleter()
{
   return{};
}
