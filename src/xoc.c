#include <xoc_api.h>
#include <xoc_compiler.h>
#include <stdio.h>

static char* test_stmt[] = {
    // stmt_if
    // "if x > 23 / 2 { m = 99 * 21.3 }",
    // "if x > 23 / 2 { m = 99 * 21.3 } else { m = 0 }",
    // "if x > 23 / 2 { m = 99 * 21.3 } else if x < 0 { m = 0 }",
    // "if a <= 10 - 1e-3 { a = a * b } else if a > 10 { s3 = 4 - b * &a  } else { s3 = 9 - b }",
    // "if a <= 10 - 1e-3 { if a > 11 { b = b - 2 } else if x == 3 + 3 { bb = bb + aa } ; a = a * b } else if a > 10 { s3 = 4 - b * &a  } else { s3 = 9 - b }",
    // stmt_switch
    "switch a + 3 { case 1: a = 1; case 2: a = 2; default: a = 0; }",
    "switch a + 3 { case 1: a = 1; break; case 2: a = 2; break; default: a = 0; break; }",
    // stmt_for | 'break' | 'continue' | stmt_return
    "a = 1 + 2 * 3\n",
    "std::ss = (! 'a' / (&0x0f * ~12) + 13e-1 * 6)\n"
};


void test_compiler(int argc, char *argv[]) {
    compiler_t cp;
    compiler_init(&cp, NULL, 
        test_stmt[0], 
        &(compiler_option_t){
            .argc = argc,
            .argv = argv,
            .is_filesys_enabled = true,
            .is_impllib_enabled = true,
        }
    );

    lexer_eat(&cp.lex, XOC_TOK_NONE);   // start
    parser_stmt(&cp.prs);

    compiler_free(&cp);
}

void test_map() {
    map_t map;
    map_init(&map);
    unsigned int key = map_add(&map, "a", 1);
    printf("key: %u, size: %d\n", key, map.size);
    char* val = map_get(&map, key);
    printf("val: %s, size: %d\n", val, map.size);
    char* val2 = map_get(&map, 0);
    printf("val2: %s, size: %d\n", val2, map.size);
    map_del(&map, key);
    printf("size: %d\n", map.size);
    map_free(&map);
}

int main(int argc, char *argv[]) {
    test_compiler(argc, argv);
    return 0;
}