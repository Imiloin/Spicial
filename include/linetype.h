#ifndef SPICIAL_LINETYPE_H
#define SPICIAL_LINETYPE_H

#define LINE_TYPE_COMMENT 0

#define _COMPONENT_BASE 100
#define COMPONENT_RESISTOR 1 + _COMPONENT_BASE
#define COMPONENT_CAPACITOR 2 + _COMPONENT_BASE
#define COMPONENT_INDUCTOR 3 + _COMPONENT_BASE
#define COMPONENT_VCVS 4 + _COMPONENT_BASE
#define COMPONENT_CCCS 5 + _COMPONENT_BASE
#define COMPONENT_VCCS 6 + _COMPONENT_BASE
#define COMPONENT_CCVS 7 + _COMPONENT_BASE
#define COMPONENT_VOLTAGE_SOURCE 8 + _COMPONENT_BASE
#define COMPONENT_CURRENT_SOURCE 9 + _COMPONENT_BASE
#define COMPONENT_DIODE 10 + _COMPONENT_BASE

#define _ANALYSIS_BASE 200
#define ANALYSIS_OP 1 + _ANALYSIS_BASE
#define ANALYSIS_DC 2 + _ANALYSIS_BASE
#define ANALYSIS_AC 3 + _ANALYSIS_BASE
#define ANALYSIS_TRAN 4 + _ANALYSIS_BASE
#define ANALYSIS_PRINT 5 + _ANALYSIS_BASE
#define ANALYSIS_PLOT 6 + _ANALYSIS_BASE
#define ANALYSIS_END 7 + _ANALYSIS_BASE
#define ANALYSIS_OPTIONS 8 + _ANALYSIS_BASE

#endif  // SPICIAL_LINETYPE_H
