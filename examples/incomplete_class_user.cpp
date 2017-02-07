#include "incomplete_class.h"

int main()
{
    consume(create_unnamed_ptr());
    // shall not compile
    // consume(create_unique_ptr());
}