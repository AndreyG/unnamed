#include "unnamed_ptr.h"
#include "make_deleter.h"

#include <cstdio>
#include <iostream>

unnamed_ptr<FILE> open_file(const char * filepath)
{
    auto file = std::fopen(filepath, "r");
    if (!file)
    {
        std::cerr << "attempt to read file '" << filepath << "' failed\n";
        return nullptr;
    }
    return { file, make_deleter<int(FILE *), std::fclose>() };
}

int main(int argc, char * argv[])
{
    if (argc != 2)
        std::cerr << "exactly one argument expected: path to file\n";

    if (unnamed_ptr<void> f = open_file(argv[1]))
        std::cout << "file successfuly opened\n";
}
