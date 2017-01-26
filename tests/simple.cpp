#include "unnamed_ptr.h"

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>

namespace
{
    struct S : boost::noncopyable
    {
       explicit S(boost::logic::tribool & state)
          : is_alive(state)
       {
          is_alive = true;
       }

       ~S()
       {
          BOOST_CHECK_EQUAL(is_alive, true);
          is_alive = false;
       }

       boost::logic::tribool & is_alive;
    };

    using SPtr = unnamed_ptr<S>;
}

BOOST_AUTO_TEST_SUITE(Simple)

BOOST_AUTO_TEST_CASE(DirectConstruction)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      SPtr s(new S(object_state));
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(DirectConstructionWithDefaultDeleter)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      SPtr s(new S(object_state), std::default_delete<S>());
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(DirectConstructionWithLambdaDeleter)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      SPtr s(new S(object_state), [] (S * s) { delete s; });
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(DirectConstructionWithFuncDeleter)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      SPtr s(new S(object_state), &boost::checked_delete<S>);
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(InPlaceConstruction)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      SPtr s(std::in_place, object_state);
      BOOST_CHECK(object_state == true);
   }

   BOOST_CHECK(object_state == false);

   {
      struct AggregateInitialization
      {
         boost::logic::tribool state;
      };

      unnamed_ptr<AggregateInitialization> s(std::in_place, boost::logic::indeterminate);
      BOOST_CHECK(boost::logic::indeterminate(s->state));
   }
}

BOOST_AUTO_TEST_CASE(MakeUnnamed)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      auto s = make_unnamed<S>(object_state);
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(FromUniquePointer)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      SPtr s = std::make_unique<S>(object_state);
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(FromUniquePointerWithCustomDeleter)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      auto deleter =  [] (S * s) {
         auto & state = s->is_alive;
         delete s;
         BOOST_CHECK(state == false);
         state = boost::logic::indeterminate;
      };
      SPtr s = std::unique_ptr<S, decltype(deleter)>(new S(object_state), deleter);
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(boost::logic::indeterminate(object_state));
}

BOOST_AUTO_TEST_SUITE_END()