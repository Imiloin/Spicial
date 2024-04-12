#ifndef SPICIAL_FUNCTION_H
#define SPICIAL_FUNCTION_H

#include <QDebug>
#include <cmath>
#include "structs.h"
#include "tokentype.h"

double calcFunctionAtTime(const Function* func,
                          double time,
                          double tstep,
                          double tstop);

#endif  // SPICIAL_FUNCTION_H
