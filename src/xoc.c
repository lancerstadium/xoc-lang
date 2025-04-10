
#include <xoc_api.h>
#include <xoc_lexer.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

    const_t a = { .f32 = 1 };
    printf("Hello, World: %f!\n", a.f32);
    return 0;
}