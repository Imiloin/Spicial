/*******************************************************

Parser for SPICE

bison -v -d -o src/parser.cpp src/parser.yy

*******************************************************/

/* Defining output file name */
/* %output "parser.cpp" */

/* Require bison minimal version */
%require "3.0.4"

/* use newer C++ skeleton file */
%skeleton "lalr1.cc"

/* write out a header file containing the token defines */
%defines

/* namespace to enclose parser in */
%define api.prefix {yy}

/* set the parser's class identifier */
%define api.parser.class {Parser}

/* it will generate a location class which can be used in your lexer */
%locations

%define parse.trace
%define parse.error verbose
%verbose

%code requires
{
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <QDebug>
#include "Circuit.h"
#include "Component.h"
#include "linetype.h"
#include "tokentype.h"
#include "structs.h"


}

/* define input */
%parse-param {Circuit *circuit} 

%union
{
    double f;  // float
    int    n;  // integer
    char  *s;  // string
    std::vector<std::string> *v;  // vector
    struct Variable *var;  // variable
    std::vector<Variable> *var_list;  // variable vector
    struct Option *opt;  // option
    std::vector<Option> *opt_list;  // option vector
    struct Function *func;  // function
};

/* assigning data type for tokens and patterns */
%token<f> FLOAT
/* %token<s> STRING */
%token<n> INTEGER
%token<f> VALUE VALUE_VOLTAGE VALUE_CURRENT VALUE_RESISTANCE VALUE_CAPACITANCE VALUE_INDUCTANCE VALUE_TIME VALUE_LENGTH VALUE_FREQUENCY

// component
%token<s> RESISTOR CAPACITOR INDUCTOR VCVS CCCS VCCS CCVS VOLTAGE_SOURCE CURRENT_SOURCE

%token<s> NODE

// keyword parameter
%token IC_EQUAL

// analysis
%token OP DC AC TRAN PRINT PLOT OPTION

%token TYPE_OP TYPE_DC TYPE_AC TYPE_TRAN

%token TYPE_DEC TYPE_OCT TYPE_LIN

%token OPTION_TYPE_NODE OPTION_TYPE_LIST

%token VAR_TYPE_VOLTAGE_REAL VAR_TYPE_VOLTAGE_IMAG VAR_TYPE_VOLTAGE_MAG VAR_TYPE_VOLTAGE_PHASE VAR_TYPE_VOLTAGE_DB
%token VAR_TYPE_CURRENT_REAL VAR_TYPE_CURRENT_IMAG VAR_TYPE_CURRENT_MAG VAR_TYPE_CURRENT_PHASE VAR_TYPE_CURRENT_DB

%token FUNC_TYPE_SIN FUNC_TYPE_PULSE

%token COMMA LPAREN RPAREN

%token END EOL


%type<f> value value_voltage value_current value_resistance value_capacitance value_inductance value_time value_length value_frequency
%type<n> analysis_type ac_type var_type
%type<f> ic_param_voltage ic_param_current
%type<s> node
%type<v> node_list
%type<var> variable
%type<var_list> variable_list
%type<opt> option
%type<opt_list> option_list
%type<func> function function_sin function_pulse

%{
extern int yylex(yy::Parser::semantic_type *yylval,
                 yy::Parser::location_type *yylloc);

extern char* first_token_of_current_line;

%}

%%

spice: netlist end
;

end: end EOL
   | END
;

netlist: netlist line
        | line
;

line: component EOL
    | analysis EOL
    | EOL
;

component: component_resistor
            | component_capacitor
            | component_inductor
            | component_vccs
            | component_cccs
            | component_vcvs
            | component_ccvs
            | component_voltage_source
            | component_current_source
;



component_resistor: RESISTOR node node value_resistance
        {
            printf("[Component] Device(Resistor) Name(%s) N+(%s) N-(%s) Value(%e)\n", $1, $2, $3, $4);
            circuit->parseResistor($1, $2, $3, $4);
        }
;

component_capacitor: CAPACITOR node node value_capacitance ic_param_voltage
        {
            printf("[Component] Device(Capacitor) Name(%s) N+(%s) N-(%s) Value(%e) IC=(%e)\n", $1, $2, $3, $4, $5);
            circuit->parseCapacitor($1, $2, $3, $4, $5);
        }
        | CAPACITOR node node value_capacitance
        {
            printf("[Component] Device(Capacitor) Name(%s) N+(%s) N-(%s) Value(%e)\n", $1, $2, $3, $4);
            circuit->parseCapacitor($1, $2, $3, $4, 0);
        }
;

component_inductor: INDUCTOR node node value_inductance ic_param_current
        {
            printf("[Component] Device(Inductor) Name(%s) N+(%s) N-(%s) Value(%e) IC=(%e)\n", $1, $2, $3, $4, $5);
            circuit->parseInductor($1, $2, $3, $4, $5);
        }
        | INDUCTOR node node value_inductance
        {
            printf("[Component] Device(Inductor) Name(%s) N+(%s) N-(%s) Value(%e)\n", $1, $2, $3, $4);
            circuit->parseInductor($1, $2, $3, $4, 0);
        }
;

component_vcvs: VCVS node node node node value
    {
        printf("[Component] Dependent Source(VCVS) Name(%s) N+(%s) N-(%s) NC+(%s) NC-(%s) Value(%e)\n", $1, $2, $3, $4, $5, $6);
        circuit->parseVCVS($1, $2, $3, $4, $5, $6);
    }
;

component_cccs: CCCS node node node value
    {
        printf("[Component] Dependent Source(CCCS) Name(%s) N+(%s) N-(%s) NC+(%s) Value(%e)\n", $1, $2, $3, $4, $5);
        circuit->parseCCCS($1, $2, $3, $4, $5);
    }
;

component_vccs: VCCS node node node node value
    {
        printf("[Component] Dependent Source(VCCS) Name(%s) N+(%s) N-(%s) NC+(%s) NC-(%s) Value(%e)\n", $1, $2, $3, $4, $5, $6);
        circuit->parseVCCS($1, $2, $3, $4, $5, $6);
    }
;

component_ccvs: CCVS node node node value
    {
        printf("[Component] Dependent Source(CCVS) Name(%s) N+(%s) N-(%s) NC+(%s) Value(%e)\n", $1, $2, $3, $4, $5);
        circuit->parseCCVS($1, $2, $3, $4, $5);
    }
;

component_voltage_source: VOLTAGE_SOURCE node node value_voltage
    {
        printf("[Component] Source(Voltage Source) Name(%s) N+(%s) N-(%s) Value(%f)\n", $1, $2, $3, $4);
        circuit->parseVoltageSource($1, $2, $3, $4, 0, 0);
    }
    | VOLTAGE_SOURCE node node TYPE_DC value_voltage
    {
        printf("[Component] Source(Voltage Source) Name(%s) N+(%s) N-(%s) DC Value(%f)\n", $1, $2, $3, $5);
        circuit->parseVoltageSource($1, $2, $3, $5, 0, 0);
    }
    | VOLTAGE_SOURCE node node value_voltage TYPE_AC value_voltage
    {
        printf("[Component] Source(Voltage Source) Name(%s) N+(%s) N-(%s) DC Value(%f) AC Value(%f)\n", $1, $2, $3, $4, $6);
        circuit->parseVoltageSource($1, $2, $3, $4, $6, 0);
    }
    | VOLTAGE_SOURCE node node value_voltage TYPE_AC value_voltage value
    {
        printf("[Component] Source(Voltage Source) Name(%s) N+(%s) N-(%s) DC Value(%f) AC Value(%f) Phase(%f)\n", $1, $2, $3, $4, $6, $7);
        circuit->parseVoltageSource($1, $2, $3, $4, $6, $7);
    }
    | VOLTAGE_SOURCE node node TYPE_DC value_voltage TYPE_AC value_voltage
    {
        printf("[Component] Source(Voltage Source) Name(%s) N+(%s) N-(%s) DC Value(%f) AC Value(%f)\n", $1, $2, $3, $5, $7);
        circuit->parseVoltageSource($1, $2, $3, $5, $7, 0);
    }
    | VOLTAGE_SOURCE node node TYPE_DC value_voltage TYPE_AC value_voltage value
    {
        printf("[Component] Source(Voltage Source) Name(%s) N+(%s) N-(%s) DC Value(%f) AC Value(%f) Phase(%f)\n", $1, $2, $3, $5, $7, $8);
        circuit->parseVoltageSource($1, $2, $3, $5, $7, $8);
    }
    | VOLTAGE_SOURCE node node function
    {
        switch ($4->type) {
            case TOKEN_FUNC_SIN:
                printf("[Component] Source(Voltage Source) Name(%s) N+(%s) N-(%s) Sin Function(OffsetVolt(%f) Amplitude(%f) Freq(%e) DelayTime(%e) DampingFactor(%e) PhaseDelay(%f))\n", $1, $2, $3, $4->values[0], $4->values[1], $4->values[2], $4->values[3], $4->values[4], $4->values[5]);
                circuit->parseVoltageSource($1, $2, $3, *$4);
                break;
            case TOKEN_FUNC_PULSE:
                printf("[Component] Source(Voltage Source) Name(%s) N+(%s) N-(%s) Pulse Function(LowVolt(%f) HightVolt(%f) Delaytime(%e) Risetime(%e) Falltime(%e) PulseWidth(%e) Period(%e))\n", $1, $2, $3, $4->values[0], $4->values[1], $4->values[2], $4->values[3], $4->values[4], $4->values[5], $4->values[6]);
                circuit->parseVoltageSource($1, $2, $3, *$4);
                break;
            default:
                printf("!No such function type\n");
        }
    }
;

component_current_source: CURRENT_SOURCE node node value_current
    {
        printf("[Component] Source(Current Source) Name(%s) N+(%s) N-(%s) Value(%f)\n", $1, $2, $3, $4);
        circuit->parseCurrentSource($1, $2, $3, $4, 0, 0);
    }
    | CURRENT_SOURCE node node TYPE_DC value_current
    {
        printf("[Component] Source(Current Source) Name(%s) N+(%s) N-(%s) DC Value(%f)\n", $1, $2, $3, $5);
        circuit->parseCurrentSource($1, $2, $3, $5, 0, 0);
    }
    | CURRENT_SOURCE node node value_current TYPE_AC value_current
    {
        printf("[Component] Source(Current Source) Name(%s) N+(%s) N-(%s) DC Value(%f) AC Value(%f)\n", $1, $2, $3, $4, $6);
        circuit->parseCurrentSource($1, $2, $3, $4, $6, 0);
    }
    | CURRENT_SOURCE node node value_current TYPE_AC value_current value
    {
        printf("[Component] Source(Current Source) Name(%s) N+(%s) N-(%s) DC Value(%f) AC Value(%f) Phase(%f)\n", $1, $2, $3, $4, $6, $7);
        circuit->parseCurrentSource($1, $2, $3, $4, $6, $7);
    }
    | CURRENT_SOURCE node node TYPE_DC value_current TYPE_AC value_current
    {
        printf("[Component] Source(Current Source) Name(%s) N+(%s) N-(%s) DC Value(%f) AC Value(%f)\n", $1, $2, $3, $5, $7);
        circuit->parseCurrentSource($1, $2, $3, $5, $7, 0);
    }
    | CURRENT_SOURCE node node TYPE_DC value_current TYPE_AC value_current value
    {
        printf("[Component] Source(Current Source) Name(%s) N+(%s) N-(%s) DC Value(%f) AC Value(%f) Phase(%f)\n", $1, $2, $3, $5, $7, $8);
        circuit->parseCurrentSource($1, $2, $3, $5, $7, $8);
    }
;

value_voltage: value
    {
        $$ = $1;
    }
    | VALUE_VOLTAGE
    {
        $$ = $1;
    }
;

value_current: value
    {
        $$ = $1;
    }
    | VALUE_CURRENT
    {
        $$ = $1;
    }
;

value_resistance: value
    {
        $$ = $1;
    }
    | VALUE_RESISTANCE
    {
        $$ = $1;
    }
;

value_capacitance: value
    {
        $$ = $1;
    }
    | VALUE_CAPACITANCE
    {
        $$ = $1;
    }
;

value_inductance: value
    {
        $$ = $1;
    }
    | VALUE_INDUCTANCE
    {
        $$ = $1;
    }
;

value_time: value
    {
        $$ = $1;
    }
    | VALUE_TIME
    {
        $$ = $1;
    }
;

value_length: value
    {
        $$ = $1;
    }
    | VALUE_LENGTH
    {
        $$ = $1;
    }
;

value_frequency: value
    {
        $$ = $1;
    }
    | VALUE_FREQUENCY
    {
        $$ = $1;
    }
;

ic_param_voltage: IC_EQUAL value_voltage
    {
        $$ = $2;
    }
;

ic_param_current: IC_EQUAL value_current
    {
        $$ = $2;
    }
;

function: function_sin
    {
        $$ = $1;
    }
    | function_pulse
    {
        $$ = $1;
    }
;

function_sin: FUNC_TYPE_SIN value_voltage value_voltage value_frequency
    {
        $$ = new Function{ TOKEN_FUNC_SIN, { $2, $3, $4, 0, 0, 0 } };
    }
    | FUNC_TYPE_SIN LPAREN value_voltage value_voltage value_frequency RPAREN
    {
        $$ = new Function{ TOKEN_FUNC_SIN, { $3, $4, $5, 0, 0, 0 } };
    }
    | FUNC_TYPE_SIN value_voltage value_voltage value_frequency value_time
    {
        $$ = new Function{ TOKEN_FUNC_SIN, { $2, $3, $4, $5, 0, 0 } };
    }
    | FUNC_TYPE_SIN LPAREN value_voltage value_voltage value_frequency value_time RPAREN
    {
        $$ = new Function{ TOKEN_FUNC_SIN, { $3, $4, $5, $6, 0, 0 } };
    }
    | FUNC_TYPE_SIN value_voltage value_voltage value_frequency value_time value
    {
        $$ = new Function{ TOKEN_FUNC_SIN, { $2, $3, $4, $5, $6, 0 } };
    }
    | FUNC_TYPE_SIN LPAREN value_voltage value_voltage value_frequency value_time value RPAREN
    {
        $$ = new Function{ TOKEN_FUNC_SIN, { $3, $4, $5, $6, $7, 0 } };
    }
    | FUNC_TYPE_SIN value_voltage value_voltage value_frequency value_time value value
    {
        $$ = new Function{ TOKEN_FUNC_SIN, { $2, $3, $4, $5, $6, $7 } };
    }
    | FUNC_TYPE_SIN LPAREN value_voltage value_voltage value_frequency value_time value value RPAREN
    {
        $$ = new Function{ TOKEN_FUNC_SIN, { $3, $4, $5, $6, $7, $8 } };
    }
;

function_pulse: FUNC_TYPE_PULSE value_voltage value_voltage value_time value_time value_time value_time
    {
        $$ = new Function{ TOKEN_FUNC_PULSE, { $2, $3, $4, $5, $6, $7, -1 } };
    }
    | FUNC_TYPE_PULSE LPAREN value_voltage value_voltage value_time value_time value_time value_time RPAREN
    {
        $$ = new Function{ TOKEN_FUNC_PULSE, { $3, $4, $5, $6, $7, $8, -1 } };
    }
    | FUNC_TYPE_PULSE value_voltage value_voltage value_time value_time value_time value_time value_time
    {
        $$ = new Function{ TOKEN_FUNC_PULSE, { $2, $3, $4, $5, $6, $7, $8 } };
    }
    | FUNC_TYPE_PULSE LPAREN value_voltage value_voltage value_time value_time value_time value_time value_time RPAREN
    {
        $$ = new Function{ TOKEN_FUNC_PULSE, { $3, $4, $5, $6, $7, $8, $9 } };
    }
;



analysis: op
        | dc
        | ac
        | tran
        | print
        | plot
        | options
;

op: OP
    {
        printf("[Analysis] Command(OP)\n");
    }
;

dc: DC VOLTAGE_SOURCE value_voltage value_voltage value_voltage
    {
        printf("[Analysis] Command(DC) Source(%s) Start(%f) End(%f) Step(%f)\n", $2, $3, $4, $5);
        circuit->DCSimulation(COMPONENT_VOLTAGE_SOURCE, $2, $3, $4, $5);
    }
    | DC CURRENT_SOURCE value_current value_current value_current
    {
        printf("[Analysis] Command(DC) Source(%s) Start(%f) End(%f) Step(%f)\n", $2, $3, $4, $5);
        circuit->DCSimulation(COMPONENT_CURRENT_SOURCE, $2, $3, $4, $5);
    }
;

ac: AC ac_type value value_frequency value_frequency
    {
        switch($2) {
            case TOKEN_DEC:
                printf("[Analysis] Command(AC) Type(DEC) PointsPerDec(%f) FreqStart(%e) FreqEnd(%e)\n", $3, $4, $5);
                circuit->ACSimulation(TOKEN_DEC, $3, $4, $5);
                break;
            case TOKEN_OCT:
                printf("[Analysis] Command(AC) Type(OCT) PointsPerOct(%f) FreqStart(%e) FreqEnd(%e)\n", $3, $4, $5);
                circuit->ACSimulation(TOKEN_OCT, $3, $4, $5);
                break;
            case TOKEN_LIN:
                printf("[Analysis] Command(AC) Type(LIN) Points(%f) FreqStart(%e) FreqEnd(%e)\n", $3, $4, $5);
                circuit->ACSimulation(TOKEN_LIN, $3, $4, $5);
                break;
            default:
                printf("!No such AC type\n");
        }
    }
;

tran: TRAN value_time value_time
    {
        printf("[Analysis] Command(TRAN) TimeStep(%e) StopTime(%e)\n", $2, $3);
        circuit->TranSimulation($2, $3, 0);
    }
    |
    TRAN value_time value_time value_time
    {
        printf("[Analysis] Command(TRAN) TimeStep(%e) StopTime(%e) StartTime(%e)\n", $2, $3, $4);
        circuit->TranSimulation($2, $3, $4);
    }
;

print: PRINT analysis_type variable_list
    {
        switch($2) {
            case TOKEN_ANALYSIS_OP:
                printf("[Analysis] Command(PRINT) Type(OP) ");
                break;
            case TOKEN_ANALYSIS_DC:
                printf("[Analysis] Command(PRINT) Type(DC) ");
                break;
            case TOKEN_ANALYSIS_AC:
                printf("[Analysis] Command(PRINT) Type(AC) ");
                break;
            case TOKEN_ANALYSIS_TRAN:
                printf("[Analysis] Command(PRINT) Type(TRAN) ");
                break;
            default:
                printf("!No such analysis type\n");
        }
        printf("Variables(");
        for (const auto& var : *$3) {
            switch(var.type) {
                case TOKEN_VAR_VOLTAGE_REAL:
                    printf("VR(");
                    break;
                case TOKEN_VAR_VOLTAGE_IMAG:
                    printf("VI(");
                    break;
                case TOKEN_VAR_VOLTAGE_MAG:
                    printf("VM(");
                    break;
                case TOKEN_VAR_VOLTAGE_PHASE:
                    printf("VP(");
                    break;
                case TOKEN_VAR_VOLTAGE_DB:
                    printf("VDB(");
                    break;
                case TOKEN_VAR_CURRENT_REAL:
                    printf("IR(");
                    break;
                case TOKEN_VAR_CURRENT_IMAG:
                    printf("II(");
                    break;
                case TOKEN_VAR_CURRENT_MAG:
                    printf("IM(");
                    break;
                case TOKEN_VAR_CURRENT_PHASE:
                    printf("IP(");
                    break;
                case TOKEN_VAR_CURRENT_DB:
                    printf("IDB(");
                    break;
                default:
                    printf("!No such variable type\n");
            }
            for (const auto& node : var.nodes) {
                printf("%s, ", node.c_str());
            }
            printf("\b\b), ");  // \b is not recommended
        }
        printf("\b\b)\n");
    }
;

plot: PLOT analysis_type variable_list
    {
        switch($2) {
            case TOKEN_ANALYSIS_OP:
                printf("[Analysis] Command(PLOT) Type(OP) ");
                break;
            case TOKEN_ANALYSIS_DC:
                printf("[Analysis] Command(PLOT) Type(DC) ");
                break;
            case TOKEN_ANALYSIS_AC:
                printf("[Analysis] Command(PLOT) Type(AC) ");
                break;
            case TOKEN_ANALYSIS_TRAN:
                printf("[Analysis] Command(PLOT) Type(TRAN) ");
                break;
            default:
                printf("!No such analysis type\n");
        }
        printf("Variables(");
        for (const auto& var : *$3) {
            switch(var.type) {
                case TOKEN_VAR_VOLTAGE_REAL:
                    printf("VR(");
                    break;
                case TOKEN_VAR_VOLTAGE_IMAG:
                    printf("VI(");
                    break;
                case TOKEN_VAR_VOLTAGE_MAG:
                    printf("VM(");
                    break;
                case TOKEN_VAR_VOLTAGE_PHASE:
                    printf("VP(");
                    break;
                case TOKEN_VAR_VOLTAGE_DB:
                    printf("VDB(");
                    break;
                case TOKEN_VAR_CURRENT_REAL:
                    printf("IR(");
                    break;
                case TOKEN_VAR_CURRENT_IMAG:
                    printf("II(");
                    break;
                case TOKEN_VAR_CURRENT_MAG:
                    printf("IM(");
                    break;
                case TOKEN_VAR_CURRENT_PHASE:
                    printf("IP(");
                    break;
                case TOKEN_VAR_CURRENT_DB:
                    printf("IDB(");
                    break;
                default:
                    printf("!No such variable type\n");
            }
            for (const auto& node : var.nodes) {
                printf("%s, ", node.c_str());
            }
            printf("\b\b), ");  // \b is not recommended
        }
        printf("\b\b)\n");
    }
;

options: OPTION option_list
    {
        printf("[Analysis] Command(OPTION) Options(");
        for (const auto& opt : *$2) {
            switch(opt.type) {
                case TOKEN_OPTION_NODE:
                    printf("Node, ");
                    break;
                case TOKEN_OPTION_LIST:
                    printf("List, ");
                    break;
                default:
                    printf("!No such option type\n");
            }
        }
        printf("\b\b)\n");
    }
;

option_list: option
    {
        $$ = new std::vector<Option>{ *$1 };
        delete $1;
    }
    | option_list option
    {
        $1->push_back(*$2);
        $$ = $1;
    }
;

option: OPTION_TYPE_NODE
    {
        $$ = new Option{ TOKEN_OPTION_NODE, -1.0 };
    }
    | OPTION_TYPE_LIST
    {
        $$ = new Option{ TOKEN_OPTION_LIST, -1.0 };
    }
;

analysis_type: TYPE_OP
    {
        $$ = TOKEN_ANALYSIS_OP;
    }
    | TYPE_DC
    {
        $$ = TOKEN_ANALYSIS_DC;
    }
    | TYPE_AC
    {
        $$ = TOKEN_ANALYSIS_AC;
    }
    | TYPE_TRAN
    {
        $$ = TOKEN_ANALYSIS_TRAN;
    }
;

variable_list: variable
    {
        $$ = new std::vector<Variable>{ *$1 };
        delete $1;
    }
    | variable_list variable
    {
        $1->push_back(*$2);
        $$ = $1;
    }
;

variable: var_type LPAREN node_list RPAREN
    {
        $$ = new Variable{ $1, *$3 };
        delete $3;
    }
;

node_list: node
    {
        $$ = new std::vector<std::string>{ $1 };
    }
    | node_list COMMA node
    {
        $1->push_back($3);
        $$ = $1;
    }
;

node: NODE
    {
        $$ = new char[strlen($1) + 1];
        strcpy($$, $1);
        std::transform($$, $$ + strlen($1), $$, ::tolower);  // convert to lower case
        $$[strlen($1)] = '\0';
    }
;


var_type: VAR_TYPE_VOLTAGE_REAL
    {
        $$ = TOKEN_VAR_VOLTAGE_REAL;
    }
    | VAR_TYPE_VOLTAGE_IMAG
    {
        $$ = TOKEN_VAR_VOLTAGE_IMAG;
    }
    | VAR_TYPE_VOLTAGE_MAG
    {
        $$ = TOKEN_VAR_VOLTAGE_MAG;
    }
    | VAR_TYPE_VOLTAGE_PHASE
    {
        $$ = TOKEN_VAR_VOLTAGE_PHASE;
    }
    | VAR_TYPE_VOLTAGE_DB
    {
        $$ = TOKEN_VAR_VOLTAGE_DB;
    }
    | VAR_TYPE_CURRENT_REAL
    {
        $$ = TOKEN_VAR_CURRENT_REAL;
    }
    | VAR_TYPE_CURRENT_IMAG
    {
        $$ = TOKEN_VAR_CURRENT_IMAG;
    }
    | VAR_TYPE_CURRENT_MAG
    {
        $$ = TOKEN_VAR_CURRENT_MAG;
    }
    | VAR_TYPE_CURRENT_PHASE
    {
        $$ = TOKEN_VAR_CURRENT_PHASE;
    }
    | VAR_TYPE_CURRENT_DB
    {
        $$ = TOKEN_VAR_CURRENT_DB;
    }
;

ac_type: TYPE_DEC
    {
        $$ = TOKEN_DEC;
    }
    | TYPE_OCT
    {
        $$ = TOKEN_OCT;
    }
    | TYPE_LIN
    {
        $$ = TOKEN_LIN;
    }
;

value: VALUE
     {
        $$ = $1;
     }
     | FLOAT
     {
        $$ = $1;
     }
     | INTEGER
     {
        $$ = $1;
     }
;

%%

namespace yy
{
	void Parser::error(const location_type& loc, const std::string &s)
	{
        std::cerr << "Error: line " << loc.begin.line << ", column " << loc.begin.column << ", " << s << std::endl;
        std::cerr << loc << std::endl;

        std::cerr << "Error: Parsing failed in line " << loc.begin.line << ", ";
        if (first_token_of_current_line != NULL) {
            std::cerr << "first token of the line: " << first_token_of_current_line << std::endl;
        }
        else {
            std::cerr << "no token found in the line." << std::endl;  // should not happen
        }
        delete circuit;
    }
}
