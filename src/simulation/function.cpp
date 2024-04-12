#include "function.h"

double calcFunctionAtTime(const Function* func,
                          double time,
                          double tstep,
                          double tstop) {
    // 默认func不为空
    if (time < 0) {
        qDebug() << "!calcFunctionAtTime() time < 0";
        return 0;
    } else if (time > tstop) {
        qDebug() << "!calcFunctionAtTime() time > tstop";
        return 0;
    }
    switch (func->type) {
        case TOKEN_FUNC_SIN: {
            double vo = func->values[0];
            double va = func->values[1];
            double freq =
                std::max(func->values[2], 1 / tstop);  // default 1/tstop
            double td = func->values[3];               // default 0
            double theta = func->values[4];            // default 0
            double phi = func->values[5];              // default 0

            if (time <= td) {
                return vo + va * sin(2 * M_PI * phi / 360);
            } else {  // td < time <= tstop
                return vo +
                       va * exp(-theta * (time - td)) *
                           sin(2 * M_PI * (freq * (time - td) + phi / 360));
            }
        }

        case TOKEN_FUNC_PULSE: {
            double v1 = func->values[0];
            double v2 = func->values[1];
            double td = func->values[2];                    // default 0
            double tr = std::max(func->values[3], tstep);   // default tstep
            double tf = std::max(func->values[4], tstep);   // default tstep
            double pw = std::max(func->values[5], tstop);   // default tstop
            double per = std::max(func->values[6], tstop);  // default tstop

            time = fmod(time, per);
            if (time <= td) {
                return v1;
            } else if (time <= td + tr) {
                return v1 + (v2 - v1) / tr * (time - td);
            } else if (time <= td + tr + pw) {
                return v2;
            } else if (time <= td + tr + pw + tf) {
                return v2 + (v1 - v2) / tf * (time - td - tr - pw);
            } else {  // td + tr + pw + tf < time <= tstop
                return v1;
            }
        }
            // add more function types here

        default:
            qDebug() << "!No such function type";
            return -1;
    }
}
