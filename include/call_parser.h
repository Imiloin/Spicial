#ifndef CALL_PARSER_H
#define CALL_PARSER_H

#include <iostream>
#include <unordered_set>
#include <sys/stat.h>
#include "Circuit.h"
#include "../src/parser/scanner.hpp"
#include "../src/parser/parser.hpp"

Circuit *callNetlistParser(const char* fileName);

#endif  // CALL_PARSER_H
