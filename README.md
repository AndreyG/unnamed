`unnamed_ptr` is a smart pointer with the same ownership semantics as `std::unique_ptr` and type erased deleter. It is the nearest relative of `std::unique_ptr<T, std::function<void (void *)>>` but it is more lightweight for such common cases when deleter is default or function pointer (for instance `fclose` for `FILE`).

`sizeof(unnamed_ptr<T>) == 3 * sizeof(void*)`

`sizeof(std::unique_ptr<T, std::function<void (void *)>>)`

|Arch/Compiler| MSVC | GCC |Clang (libc++)|
|---|:------:|:-----:|:--------------:|
|x86|  48  | 20  |       32     |
|x64|  72  | 40  |       64     |
