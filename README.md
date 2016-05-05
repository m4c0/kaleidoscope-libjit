# kaleidoscope-libjit

LLVM's Kaleidoscope example rewritten as an LIBJIT example.

It's working nicely for "closures" (libjit concept of "native pointer to functions"), but "jit_function_apply" is returning garbage.

I'm using Lex/Yacc instead of a hand-made parser, just because the focus here is on LibJIT, and not on the parsing.

Also, this version does not consider memory leaks.

