#pragma once

#include "unnamed_ptr.h"

class MyClass;
using MyUnnamedPtr  = unnamed_ptr<MyClass>;
using MyUniquePtr   = std::unique_ptr<MyClass>;

MyUnnamedPtr create_unnamed_ptr();
MyUniquePtr  create_unique_ptr();

void consume(MyUnnamedPtr);
void consume(MyUniquePtr);
