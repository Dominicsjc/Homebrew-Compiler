#ifndef L2_PARSER_H
#define L2_PARSER_H

#include <L2.h>

namespace L2 {
    Program parse_file(char *fileName);

    Program parse_function_file(char *fileName);

    Program parse_spill_file(char *fileName);
}

#endif //L2_PARSER_H
