#include "unnamed_ptr.h"
#include "make_deleter.h"

#include <boost/test/unit_test.hpp>
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>

namespace
{
   thread_local size_t allocation_num = 0;
}

void* operator new(std::size_t count)
{
   ++allocation_num;
   auto res = malloc(count);
   if (!res)
      throw std::bad_alloc();
   return res;
}

void operator delete(void * ptr)
{
   free(ptr);
}

namespace
{
   struct S : boost::noncopyable
   {
   };
}

BOOST_AUTO_TEST_SUITE(Alloc)

BOOST_AUTO_TEST_CASE(DirectInitialization)
{
   auto old_allocations_num = allocation_num;
   unnamed_ptr<S> ptr(std::in_place);
   BOOST_CHECK_EQUAL(allocation_num, old_allocations_num + 1);
}

BOOST_AUTO_TEST_CASE(MakeUnnamed)
{
   auto old_allocations_num = allocation_num;
   make_unnamed<S>();
   BOOST_CHECK_EQUAL(allocation_num, old_allocations_num + 1);
}

BOOST_AUTO_TEST_CASE(FuncPtr)
{
   auto s = new S;
   auto old_allocations_num = allocation_num;
   unnamed_ptr<S> ptr(s, boost::checked_delete<S>);
   BOOST_CHECK_EQUAL(allocation_num, old_allocations_num + 1);
}

BOOST_AUTO_TEST_CASE(CompileTimeFuncPtr)
{
   auto s = new S;
   auto old_allocations_num = allocation_num;
   unnamed_ptr<S> ptr(s, MAKE_DELETER(boost::checked_delete<S>));
   BOOST_CHECK_EQUAL(allocation_num, old_allocations_num);
}

BOOST_AUTO_TEST_CASE(DefaultDeleter)
{
   auto s = new S;
   auto old_allocations_num = allocation_num;
   unnamed_ptr<S> ptr(s, std::default_delete<S>());
   BOOST_CHECK_EQUAL(allocation_num, old_allocations_num);
}

BOOST_AUTO_TEST_CASE(LambdaDeleter)
{
   auto s = new S;
   auto old_allocations_num = allocation_num;
   unnamed_ptr<S> ptr(s, [] (S * s) { delete s; });
   BOOST_CHECK_EQUAL(allocation_num, old_allocations_num);
}

BOOST_AUTO_TEST_CASE(LambdaPlusDeleter)
{
   auto s = new S;
   auto old_allocations_num = allocation_num;
   void (*deleter)(S*) = [] (S * s) { delete s; };
   unnamed_ptr<S> ptr(s, deleter);
   BOOST_CHECK_EQUAL(allocation_num, old_allocations_num + 1);
}

BOOST_AUTO_TEST_SUITE_END()