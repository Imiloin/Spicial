#ifndef SPICIAL_CALL_PARSER_H
#define SPICIAL_CALL_PARSER_H

#include <iostream>
#include <unordered_set>
#include <sys/stat.h>
#include "Circuit.h"
#include "../src/parser/scanner.hpp"
#include "../src/parser/parser.hpp"

Circuit *callNetlistParser(const char* fileName);

#endif  // SPICIAL_CALL_PARSER_H
