#include "unnamed_ptr.h"

#include <cstdio>

unnamed_ptr<FILE> open_file(const char * filepath)
{
    return { std::fopen(filepath, "r"), std::fclose };
}

int main()
{
    open_file("file.cpp");
    unnamed_ptr<void> f = open_file("file.cpp");
}
