#include "incomplete_class.h"

class MyClass
{
    
};

MyUnnamedPtr create_unnamed_ptr()
{
    return make_unnamed<MyClass>();
}

MyUniquePtr create_unique_ptr()
{
    return std::make_unique<MyClass>();
}

void consume(MyUnnamedPtr)
{
}

void consume(MyUniquePtr)
{
}
