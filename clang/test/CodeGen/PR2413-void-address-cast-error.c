// RUN: %clang_cc1 -emit-llvm %s -o -
void f(void)
{
        void *addr;
        addr = (void *)( ((long int)addr + 7L) );
}
