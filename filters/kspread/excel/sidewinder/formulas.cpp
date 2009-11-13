/* Swinder - Portable library for spreadsheet
   Copyright (C) 2003-2005 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2006,2009 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA
 */
#include "formulas.h"

#include <iostream>
#include <iomanip>

#include "excel.h"
#include "utils.h"

namespace Swinder
{

//=============================================
//          FormulaToken
//=============================================

class FormulaToken::Private
{
public:
    unsigned ver;
    unsigned id;
    std::vector<unsigned char> data;
};

FormulaToken::FormulaToken()
{
    d = new Private;
    d->ver = Excel97;
    d->id = Unused;
}

FormulaToken::FormulaToken(unsigned t)
{
    d = new Private;
    d->ver = Excel97;
    d->id = t;
}

FormulaToken::FormulaToken(const FormulaToken& token)
{
    d = new Private;
    d->ver = token.d->ver;
    d->id = token.id();

    d->data.resize(token.d->data.size());
    for (unsigned i = 0; i < d->data.size(); i++)
        d->data[i] = token.d->data[i];
}

FormulaToken::~FormulaToken()
{
    delete d;
}

unsigned FormulaToken::version() const
{
    return d->ver;
}

void FormulaToken::setVersion(unsigned v)
{
    d->ver = v;
}

unsigned FormulaToken::id() const
{
    return d->id;
}

const char* FormulaToken::idAsString() const
{
    const char* s = 0;

    switch (d->id) {
        case Matrix:       s = "Matrix"; break;
        case Table:        s = "Table"; break;
        case Add:          s = "Add"; break;
        case Sub:          s = "Sub"; break;
        case Mul:          s = "Mul"; break;
        case Div:          s = "Div"; break;
        case Power:        s = "Power"; break;
        case Concat:       s = "Concat"; break;
        case LT:           s = "LT"; break;
        case LE:           s = "LE"; break;
        case EQ:           s = "EQ"; break;
        case GE:           s = "GE"; break;
        case GT:           s = "GT"; break;
        case NE:           s = "NE"; break;
        case Intersect:    s = "Intersect"; break;
        case List:         s = "List"; break;
        case Range:        s = "Range"; break;
        case UPlus:        s = "UPlus"; break;
        case UMinus:       s = "UMinus"; break;
        case Percent:      s = "Percent"; break;
        case Paren:        s = "Paren"; break;
        case String:       s = "String"; break;
        case MissArg:      s = "MissArg"; break;
        case ErrorCode:    s = "ErrorCode"; break;
        case Bool:         s = "Bool"; break;
        case Integer:      s = "Integer"; break;
        case Array:        s = "Array"; break;
        case Function:     s = "Function"; break;
        case FunctionVar:  s = "FunctionVar"; break;
        case Name:         s = "Name"; break;
        case Ref:          s = "Ref"; break;
        case RefErr:       s = "RefErr"; break;
        case RefN:         s = "RefN"; break;
        case Area:         s = "Area"; break;
        case AreaErr:      s = "AreaErr"; break;
        case AreaN:        s = "AreaN"; break;
        case NameX:        s = "NameX"; break;
        case Ref3d:        s = "Ref3d"; break;
        case RefErr3d:     s = "RefErr3d"; break;
        case Float:        s = "Float"; break;
        case Area3d:       s = "Area3d"; break;
        case AreaErr3d:    s = "AreaErr3d"; break;
        default:           s = "Unknown"; break;
    }

    return s;
}


unsigned FormulaToken::size() const
{
    unsigned s = 0; // on most cases no data

    switch (d->id) {
        case Add:
        case Sub:
        case Mul:
        case Div:
        case Power:
        case Concat:
        case LT:
        case LE:
        case EQ:
        case GE:
        case GT:
        case NE:
        case Intersect:
        case List:
        case Range:
        case UPlus:
        case UMinus:
        case Percent:
        case Paren:
        case MissArg:
            s = 0; break;

        case Attr:
            s = 3; break;

        case ErrorCode:
        case Bool:
            s = 1; break;

        case Integer:
            s = 2; break;

        case Array:
            s = 7; break;

        case Function:
            s = 2; break;

        case FunctionVar:
            s = 3; break;

        case Matrix:
        case Table:
            s = (d->ver == Excel97) ? 4 : 3; break;

        case Name:
            s = (d->ver == Excel97) ? 4 : 14; break;

        case Ref:
        case RefErr:
        case RefN:
            s = (d->ver == Excel97) ? 4 : 3; break;

        case Area:
        case AreaErr:
        case AreaN:
            s = (d->ver == Excel97) ? 8 : 6; break;

        case NameX:
            s = (d->ver == Excel97) ? 6 : 24; break;

        case Ref3d:
        case RefErr3d:
            s = (d->ver == Excel97) ? 6 : 17; break;

        case Float:
            s = 8; break;

        case Area3d:
        case AreaErr3d:
            s = (d->ver == Excel97) ? 10 : 20; break;

        default:
            // WARNING this is unhandled case
            break;
    }

    return s;
}

void FormulaToken::setData(unsigned size, const unsigned char* data)
{
    d->data.resize(size);
    for (unsigned i = 0; i < size; i++)
        d->data[i] = data[i];
}

Value FormulaToken::value() const
{
    Value result;

    unsigned char* buf;
    buf = new unsigned char[d->data.size()];
    for (unsigned k = 0; k < d->data.size(); k++)
        buf[k] = d->data[k];

    // FIXME sanity check: verify size of data
    switch (d->id) {
        case ErrorCode:
            result = errorAsValue(buf[0]);
            break;

        case Bool:
            result = Value(buf[0] != 0);
            break;

        case Integer:
            result = Value((int)readU16(buf));
            break;

        case Float:
            result = Value(readFloat64(buf));
            break;

        case String: {
            EString estr = (version() == Excel97) ?
                       EString::fromUnicodeString(buf, false, d->data.size()) :
                       EString::fromByteString(buf, false, d->data.size());
            result = Value(estr.str());
            break;
        }

        default: break;
    }

    delete [] buf;

    return result;
}

unsigned FormulaToken::functionIndex() const
{
    // FIXME check data size
    unsigned index = 0;
    unsigned char buf[2];

    if (d->id == Function) {
        buf[0] = d->data[0];
        buf[1] = d->data[1];
        index = readU16(buf);
    }

    if (d->id == FunctionVar) {
        buf[0] = d->data[1];
        buf[1] = d->data[2];
        index = readU16(buf);
    }

    return index;
}

struct FunctionEntry {
    const char *name;
    int params;
};

static const FunctionEntry FunctionEntries[] = {
    { "COUNT",           1 },     // 0
    { "IF",              0 },     // 1
    { "ISNV",            1 },     // 2
    { "ISERROR",         1 },     // 3
    { "SUM",             0 },     // 4
    { "AVERAGE",         0 },     // 5
    { "MIN",             0 },     // 6
    { "MAX",             0 },     // 7
    { "ROW",             0 },     // 8
    { "COLUMN",          0 },     // 9
    { "NOVALUE",         0 },     // 10
    { "NPV",             0 },     // 11
    { "STDEV",           0 },     // 12
    { "DOLLAR",          0 },     // 13
    { "FIXED",           0 },     // 14
    { "SIN",             1 },     // 15
    { "COS",             1 },     // 16
    { "TAN",             1 },     // 17
    { "ATAN",            1 },     // 18
    { "PI",              0 },     // 19
    { "SQRT",            1 },     // 20
    { "EXP",             1 },     // 21
    { "LN",              1 },     // 22
    { "LOG10",           1 },     // 23
    { "ABS",             1 },     // 24
    { "INT",             1 },     // 25
    { "SIGN",            1 },     // 26
    { "ROUND",           2 },     // 27
    { "LOOKUP",          0 },     // 28
    { "INDEX",           0 },     // 29
    { "REPT",            2 },     // 30
    { "MID",             3 },     // 31
    { "LEN",             1 },     // 32
    { "VALUE",           1 },     // 33
    { "TRUE",            0 },     // 34
    { "FALSE",           0 },     // 35
    { "AND",             0 },     // 36
    { "OR",              0 },     // 37
    { "NOT",             1 },     // 38
    { "MOD",             2 },     // 39
    { "DCOUNT",          3 },     // 40
    { "DSUM",            3 },     // 41
    { "DAVERAGE",        3 },     // 42
    { "DMIN",            3 },     // 43
    { "DMAX",            3 },     // 44
    { "DSTDEV",          3 },     // 45
    { "VAR",             0 },     // 46
    { "DVAR",            3 },     // 47
    { "TEXT",            2 },     // 48
    { "LINEST",          0 },     // 49
    { "TREND",           0 },     // 50
    { "LOGEST",           0 },     // 51
    { "GROWTH",          0 },     // 52
    { "GOTO",            0 },     // 53
    { "HALT",            0 },     // 54
    { "Unknown55",       0 },     // 55
    { "PV",              0 },     // 56
    { "FV",              0 },     // 57
    { "NPER",            0 },     // 58
    { "PMT",             0 },     // 59
    { "RATE",            0 },     // 60
    { "MIRR",            3 },     // 61
    { "IRR",             0 },     // 62
    { "RAND",            0 },     // 63
    { "MATCH",           0 },     // 64
    { "DATE",            3 },     // 65
    { "TIME",            3 },     // 66
    { "DAY",             1 },     // 67
    { "MONTH",           1 },     // 68
    { "YEAR",            1 },     // 69
    { "DAYOFWEEK",       0 },     // 70
    { "HOUR",            1 },     // 71
    { "MIN",             1 },     // 72
    { "SEC",             1 },     // 73
    { "NOW",             0 },     // 74
    { "AREAS",           1 },     // 75
    { "ROWS",            1 },     // 76
    { "COLUMNS",         1 },     // 77
    { "OFFSET",          0 },     // 78
    { "ABSREF",          2 },     // 79
    { "RELREF",          0 },     // 80
    { "ARGUMENT",        0 },     // 81
    { "SEARCH",          0 },     // 82
    { "TRANSPOSE",       1 },     // 83
    { "ERROR",           0 },     // 84
    { "STEP",            0 },     // 85
    { "TYPE",            1 },     // 86
    { "ECHO",            0 },
    { "SETNAME",         0 },
    { "CALLER",          0 },
    { "DEREF",           0 },
    { "WINDOWS",         0 },
    { "SERIES",          4 },
    { "DOCUMENTS",       0 },
    { "ACTIVECELL",      0 },
    { "SELECTION",       0 },
    { "RESULT",          0 },
    { "ATAN2",           2 },     // 97
    { "ASIN",            1 },     // 98
    { "ACOS",            1 },     // 99
    { "CHOOSE",          0 },     // 100
    { "HLOOKUP",         0 },     // 101
    { "VLOOKUP",         0 },     // 102
    { "LINKS",           0 },
    { "INPUT",           0 },
    { "ISREF",           1 },     // 105
    { "GETFORMULA",      0 },
    { "GETNAME",         0 },
    { "SETVALUE",        0 },
    { "LOG",             0 },     // 109
    { "EXEC",            0 },
    { "CHAR",            1 },     // 111
    { "LOWER",           1 },     // 112
    { "UPPER",           1 },     // 113
    { "PROPER",          1 },     // 114
    { "LEFT",            0 },     // 115
    { "RIGHT",           0 },     // 116
    { "EXACT",           2 },     // 117
    { "TRIM",            1 },     // 118
    { "REPLACE",         4 },     // 119
    { "SUBSTITUTE",      0 },     // 120
    { "CODE",            1 },     // 121
    { "NAMES",           0 },
    { "DIRECTORY",       0 },
    { "FIND",            0 },     // 124
    { "CELL",            0 },     // 125
    { "ISERR",           1 },     // 126
    { "ISTEXT",          1 },     // 127
    { "ISNUMBER",        1 },     // 128
    { "ISBLANK",         1 },     // 129
    { "T",               1 },     // 130
    { "N",               1 },     // 131
    { "FOPEN",           0 },
    { "FCLOSE",          0 },
    { "FSIZE",           0 },
    { "FREADLN",         0 },
    { "FREAD",           0 },
    { "FWRITELN",        0 },
    { "FWRITE",          0 },
    { "FPOS",            0 },
    { "DATEVALUE",       1 },     // 140
    { "TIMEVALUE",       1 },     // 141
    { "SLN",             3 },     // 142
    { "SYD",             4 },     // 143
    { "DDB",             0 },     // 144
    { "GETDEF",          0 },
    { "REFTEXT",         0 },
    { "TEXTREF",         0 },
    { "INDIRECT",        0 },     // 148
    { "REGISTER",        0 },
    { "CALL",            0 },
    { "ADDBAR",          0 },
    { "ADDMENU",         0 },
    { "ADDCOMMAND",      0 },
    { "ENABLECOMMAND",   0 },
    { "CHECKCOMMAND",    0 },
    { "RENAMECOMMAND",   0 },
    { "SHOWBAR",         0 },
    { "DELETEMENU",      0 },
    { "DELETECOMMAND",   0 },
    { "GETCHARTITEM",    0 },
    { "DIALOGBOX",       0 },
    { "CLEAN",           1 },     // 162
    { "MDETERM",         1 },     // 163
    { "MINVERSE",        1 },     // 164
    { "MMULT",           2 },     // 165
    { "FILES",           0 },
    { "IPMT",            0 },     // 167
    { "PPMT",            0 },     // 168
    { "COUNTA",          0 },     // 169
    { "CANCELKEY",       1 },
    { "Unknown171",      0 },
    { "Unknown172",      0 },
    { "Unknown173",      0 },
    { "Unknown174",      0 },
    { "INITIATE",        0 },
    { "REQUEST",         0 },
    { "POKE",            0 },
    { "EXECUTE",         0 },
    { "TERMINATE",       0 },
    { "RESTART",         0 },
    { "HELP",            0 },
    { "GETBAR",          0 },
    { "PRODUCT",         0 },     // 183
    { "FACT",            1 },     // 184
    { "GETCELL",         0 },
    { "GETWORKSPACE",    0 },
    { "GETWINDOW",       0 },
    { "GETDOCUMENT",     0 },
    { "DPRODUCT",        3 },     // 189
    { "ISNONTEXT",       1 },     // 190
    { "GETNOTE",         0 },
    { "NOTE",            0 },
    { "STDEVP",          0 },     // 193
    { "VARP",            0 },     // 194
    { "DSTDEVP",         3 },     // 195
    { "DVARP",           3 },     // 196
    { "TRUNC",           0 },     // 197
    { "ISLOGICAL",       1 },     // 198
    { "DCOUNTA",         3 },     // 199
    { "DELETEBAR",       0 },
    { "UNREGISTER",      0 },
    { "Unknown202",      0 },
    { "Unknown203",      0 },
    { "USDOLLAR",        0 },
    { "FINDB",           0 },
    { "SEARCHB",         0 },
    { "REPLACEB",        0 },
    { "LEFTB",           0 },
    { "RIGHTB",          0 },
    { "MIDB",            0 },
    { "LENB",            0 },
    { "ROUNDUP",         2 },     // 212
    { "ROUNDDOWN",       2 },     // 213
    { "ASC",             0 },
    { "DBCS",            0 },
    { "RANK",            0 },     // 216
    { "Unknown217",      0 },
    { "Unknown218",      0 },
    { "ADDRESS",         0 },     // 219
    { "GETDIFFDATE360",  0 },     // 220
    { "CURRENTDATE",     0 },     // 221
    { "VBD",             0 },     // 222
    { "Unknown223",      0 },
    { "Unknown224",      0 },
    { "Unknown225",      0 },
    { "Unknown226",      0 },
    { "MEDIAN",          0 },     // 227
    { "SUMPRODUCT",      0 },     // 228
    { "SINH",            1 },     // 229
    { "COSH",            1 },     // 230
    { "TANH",            1 },     // 231
    { "ASINH",           1 },     // 232
    { "ACOSH",           1 },     // 233
    { "ATANH",           1 },     // 234
    { "DGET",            3 },     // 235
    { "CREATEOBJECT",    0 },
    { "VOLATILE",        0 },
    { "LASTERROR",       0 },
    { "CUSTOMUNDO",      0 },
    { "CUSTOMREPEAT",    0 },
    { "FORMULACONVERT",  0 },
    { "GETLINKINFO",     0 },
    { "TEXTBOX",         0 },
    { "INFO",            1 },     // 244
    { "GROUP",           0 },
    { "GETOBJECT",       0 },
    { "DB",              0 },     // 247
    { "PAUSE",           0 },
    { "Unknown249",      0 },
    { "Unknown250",      0 },
    { "RESUME",          0 },
    { "FREQUENCY",       2 },     // 252
    { "ADDTOOLBAR",      0 },
    { "DELETETOOLBAR",   0 },
    { "Unknown255",      0 },
    { "RESETTOOLBAR",    0 },
    { "EVALUATE",        0 },
    { "GETTOOLBAR",      0 },
    { "GETTOOL",         0 },
    { "SPELLINGCHECK",   0 },
    { "ERRORTYPE",       1 },     // 261
    { "APPTITLE",        0 },
    { "WINDOWTITLE",     0 },
    { "SAVETOOLBAR",     0 },
    { "ENABLETOOL",      0 },
    { "PRESSTOOL",       0 },
    { "REGISTERID",      0 },
    { "GETWORKBOOK",     0 },
    { "AVEDEV",          0 },     // 269
    { "BETADIST",        0 },     // 270
    { "GAMMALN",         1 },     // 271
    { "BETAINV",         0 },     // 272
    { "BINOMDIST",       4 },     // 273
    { "CHIDIST",         2 },     // 274
    { "CHIINV",          2 },     // 275
    { "COMBIN",          2 },     // 276
    { "CONFIDENCE",      3 },     // 277
    { "CRITBINOM",       3 },     // 278
    { "EVEN",            1 },     // 279
    { "EXPONDIST",       3 },     // 280
    { "FDIST",           3 },     // 281
    { "FINV",            3 },     // 282
    { "FISHER",          1 },     // 283
    { "FISHERINV",       1 },     // 284
    { "FLOOR",           2 },     // 285
    { "GAMMADIST",       4 },     // 286
    { "GAMMAINV",        3 },     // 287
    { "CEIL",            2 },     // 288
    { "HYPGEOMDIST",     4 },     // 289
    { "LOGNORMDIST",     3 },     // 290
    { "LOGINV",          3 },     // 291
    { "NEGBINOMDIST",    3 },     // 292
    { "NORMDIST",        4 },     // 293
    { "NORMSDIST",       1 },     // 294
    { "NORMINV",         3 },     // 295
    { "NORMSINV",        1 },     // 296
    { "STANDARDIZE",     3 },     // 297
    { "ODD",             1 },     // 298
    { "PERMUT",          2 },     // 299
    { "POISSON",         3 },     // 300
    { "TDIST",           3 },     // 301
    { "WEIBULL",         4 },     // 302
    { "SUMXMY2",         2 },     // 303
    { "SUMX2MY2",        2 },     // 304
    { "SUMX2DY2",        2 },     // 305
    { "CHITEST",         2 },     // 306
    { "CORREL",          2 },     // 307
    { "COVAR",           2 },     // 308
    { "FORECAST",        3 },     // 309
    { "FTEST",           2 },     // 310
    { "INTERCEPT",       2 },     // 311
    { "PEARSON",         2 },     // 312
    { "RSQ",             2 },     // 313
    { "STEYX",           2 },     // 314
    { "SLOPE",           2 },     // 315
    { "TTEST",           4 },     // 316
    { "PROB",            0 },     // 317
    { "DEVSQ",           0 },     // 318
    { "GEOMEAN",         0 },     // 319
    { "HARMEAN",         0 },     // 320
    { "SUMSQ",           0 },     // 321
    { "KURT",            0 },     // 322
    { "SKEW",            0 },     // 323
    { "ZTEST",           0 },     // 324
    { "LARGE",           2 },     // 325
    { "SMALL",           2 },     // 326
    { "QUARTILE",        2 },     // 327
    { "PERCENTILE",      2 },     // 328
    { "PERCENTRANK",     0 },     // 329
    { "MODALVALUE",      0 },     // 330
    { "TRIMMEAN",        2 },     // 331
    { "TINV",            2 },     // 332
    { "Unknown333",      0 },
    { "MOVIECOMMAND",    0 },
    { "GETMOVIE",        0 },
    { "CONCATENATE",     0 },     // 336
    { "POWER",           2 },     // 337
    { "PIVOTADDDATA",    0 },
    { "GETPIVOTTABLE",   0 },
    { "GETPIVOTFIELD",   0 },
    { "GETPIVOTITEM",    0 },
    { "RADIANS",         1 },     // 342
    { "DEGREES",         1 },     // 343
    { "SUBTOTAL",        0 },     // 344
    { "SUMIF",           0 },     // 345
    { "COUNTIF",         2 },     // 346
    { "COUNTBLANK",      1 },     // 347
    { "SCENARIOGET",     0 },
    { "OPTIONSLISTSGET", 0 },
    { "ISPMT",           4 },
    { "DATEDIF",         3 },
    { "DATESTRING",      0 },
    { "NUMBERSTRING",    0 },
    { "ROMAN",           0 },     // 354
    { "OPENDIALOG",      0 },
    { "SAVEDIALOG",      0 },
    { "VIEWGET",         0 },
    { "GETPIVOTDATA",    2 },     // 358
    { "HYPERLINK",       1 },
    { "PHONETIC",        0 },
    { "AVERAGEA",        0 },     // 361
    { "MAXA",            0 },     // 362
    { "MINA",            0 },     // 363
    { "STDEVPA",         0 },     // 364
    { "VARPA",           0 },     // 365
    { "STDEVA",          0 },     // 366
    { "VARA",            0 },     // 367
};

const char* FormulaToken::functionName() const
{
    if (functionIndex() > 367) return 0;
    return FunctionEntries[ functionIndex()].name;
}

unsigned FormulaToken::functionParams() const
{
    unsigned params = 0;

    if (d->id == Function) {
        if (functionIndex() > 367) return 0;
        params = FunctionEntries[ functionIndex()].params;
    }

    if (d->id == FunctionVar) {
        params = (unsigned)d->data[0];
        params &= 0x7f;
    }

    return params;
}

unsigned FormulaToken::attr() const
{
    unsigned attr = 0;
    if (d->id == Attr) {
        attr = (unsigned) d->data[0];
    }
    return attr;
}

unsigned FormulaToken::nameIndex() const
{
    // FIXME check data size !
    unsigned ni = 0;
    unsigned char buf[2];

    if (d->id == NameX)
        if (d->ver == Excel97) {
            buf[0] = d->data[2];
            buf[1] = d->data[3];
            ni = readU16(buf);
        }

    if (d->id == NameX)
        if (d->ver == Excel95) {
            buf[0] = d->data[10];
            buf[1] = d->data[11];
            ni = readU16(buf);
        }

    return ni;
}


UString FormulaToken::area(unsigned row, unsigned col, bool relative) const
{
    // FIXME check data size !
    unsigned char buf[2];
    int row1Ref, row2Ref, col1Ref, col2Ref;
    bool row1Relative, col1Relative;
    bool row2Relative, col2Relative;

    if (version() == Excel97) {
        buf[0] = d->data[0];
        buf[1] = d->data[1];
        row1Ref = readU16(buf);

        buf[0] = d->data[2];
        buf[1] = d->data[3];
        row2Ref = readU16(buf);

        buf[0] = d->data[4];
        buf[1] = d->data[5];
        col1Ref = readU16(buf);

        buf[0] = d->data[6];
        buf[1] = d->data[7];
        col2Ref = readU16(buf);

        row1Relative = col1Ref & 0x8000;
        col1Relative = col1Ref & 0x4000;
        col1Ref &= 0x3fff;

        row2Relative = col2Ref & 0x8000;
        col2Relative = col2Ref & 0x4000;
        col2Ref &= 0x3fff;

        if (relative) {
            if (row1Ref & 0x8000) row1Ref -= 0x10000;
            if (row2Ref & 0x8000) row2Ref -= 0x10000;
            if (col1Ref & 0x80) col1Ref -= 0x100;
            if (col2Ref & 0x80) col2Ref -= 0x100;
        }
    } else {
        buf[0] = d->data[0];
        buf[1] = d->data[1];
        row1Ref = readU16(buf);

        buf[0] = d->data[2];
        buf[1] = d->data[3];
        row2Ref = readU16(buf);

        buf[0] = d->data[4];
        buf[1] = 0;
        col1Ref = readU16(buf);

        buf[0] = d->data[5];
        buf[1] = 0;
        col2Ref = readU16(buf);

        row1Relative = row2Ref & 0x8000;
        col1Relative = row2Ref & 0x4000;
        row1Ref &= 0x3fff;

        row2Relative = row2Ref & 0x8000;
        col2Relative = row2Ref & 0x4000;
        row2Ref &= 0x3fff;

        if (relative) {
            if (row1Ref & 0x2000) row1Ref -= 0x4000;
            if (row2Ref & 0x2000) row2Ref -= 0x4000;
            if (col1Ref & 0x80) col1Ref -= 0x100;
            if (col2Ref & 0x80) col2Ref -= 0x100;
        }
    }

    if (relative) {
        row1Ref += row;
        row2Ref += row;
        col1Ref += col;
        col2Ref += col;
    }

    UString result;
    result.append(UString("["));    // OpenDocument format

    if (!col1Relative)
        result.append(UString("$"));
    result.append(Cell::columnLabel(col1Ref));
    if (!row1Relative)
        result.append(UString("$"));
    result.append(UString::from(row1Ref + 1));
    result.append(UString(":"));
    if (!col2Relative)
        result.append(UString("$"));
    result.append(Cell::columnLabel(col2Ref));
    if (!row2Relative)
        result.append(UString("$"));
    result.append(UString::from(row2Ref + 1));

    result.append(UString("]"));  // OpenDocument format

    return result;
}

UString FormulaToken::area3d(const std::vector<UString>& externSheets, unsigned /*row*/, unsigned /*col*/) const
{
    if (version() != Excel97) {
        return UString("Unknown");
    }

    unsigned sheetRef = readU16(&d->data[0]);

    // FIXME check data size !
    unsigned char buf[2];
    int row1Ref, row2Ref, col1Ref, col2Ref;
    bool row1Relative, col1Relative;
    bool row2Relative, col2Relative;

    buf[0] = d->data[2];
    buf[1] = d->data[3];
    row1Ref = readU16(buf);

    buf[0] = d->data[4];
    buf[1] = d->data[5];
    row2Ref = readU16(buf);

    buf[0] = d->data[6];
    buf[1] = d->data[7];
    col1Ref = readU16(buf);

    buf[0] = d->data[8];
    buf[1] = d->data[9];
    col2Ref = readU16(buf);

    row1Relative = col1Ref & 0x8000;
    col1Relative = col1Ref & 0x4000;
    col1Ref &= 0x3fff;

    row2Relative = col2Ref & 0x8000;
    col2Relative = col2Ref & 0x4000;
    col2Ref &= 0x3fff;

    UString result;
    result.append(UString("["));    // OpenDocument format

    if (sheetRef >= externSheets.size())
        result.append(UString("Error"));
    else
        result.append(externSheets[sheetRef]);
    result.append(UString("."));

    if (!col1Relative)
        result.append(UString("$"));
    result.append(Cell::columnLabel(col1Ref));
    if (!row1Relative)
        result.append(UString("$"));
    result.append(UString::from(row1Ref + 1));
    result.append(UString(":"));
    if (!col2Relative)
        result.append(UString("$"));
    result.append(Cell::columnLabel(col2Ref));
    if (!row2Relative)
        result.append(UString("$"));
    result.append(UString::from(row2Ref + 1));

    result.append(UString("]"));  // OpenDocument format

    return result;
}

UString FormulaToken::ref(unsigned /*row*/, unsigned /*col*/) const
{
    // FIXME check data size !
    // FIXME handle shared formula
    unsigned char buf[2];
    int rowRef, colRef;
    bool rowRelative, colRelative;

    if (version() == Excel97) {
        buf[0] = d->data[0];
        buf[1] = d->data[1];
        rowRef = readU16(buf);

        buf[0] = d->data[2];
        buf[1] = d->data[3];
        colRef = readU16(buf);

        rowRelative = colRef & 0x8000;
        colRelative = colRef & 0x4000;
        colRef &= 0x3fff;
    } else {
        buf[0] = d->data[0];
        buf[1] = d->data[1];
        rowRef = readU16(buf);

        buf[0] = d->data[2];
        buf[1] = 0;
        colRef = readU16(buf);

        rowRelative = rowRef & 0x8000;
        colRelative = rowRef & 0x4000;
        rowRef &= 0x3fff;
    }

    UString result;

    result.append(UString("["));    // OpenDocument format

    if (!colRelative)
        result.append(UString("$"));
    result.append(Cell::columnLabel(colRef));
    if (!rowRelative)
        result.append(UString("$"));
    result.append(UString::from(rowRef + 1));

    result.append(UString("]"));  // OpenDocument format

    return result;
}

UString FormulaToken::refn(unsigned row, unsigned col) const
{
    // FIXME check data size !
    // FIXME handle shared formula
    unsigned char buf[2];
    int rowRef, colRef;
    bool rowRelative, colRelative;

    if (version() == Excel97) {
        buf[0] = d->data[0];
        buf[1] = d->data[1];
        rowRef = readS16(buf);

        buf[0] = d->data[2];
        buf[1] = d->data[3];
        colRef = readU16(buf);

        rowRelative = colRef & 0x8000;
        colRelative = colRef & 0x4000;
        colRef &= 0xff;
        if (colRef & 0x80) {
            colRef = colRef - 0x100;
        }
    } else {
        buf[0] = d->data[0];
        buf[1] = d->data[1];
        rowRef = readU16(buf);

        buf[0] = d->data[2];
        colRef = readS8(buf);

        rowRelative = rowRef & 0x8000;
        colRelative = rowRef & 0x4000;
        rowRef &= 0x3fff;
        if (rowRef & 0x2000) {
            rowRef = rowRef - 0x4000;
        }
    }

    colRef += col;
    rowRef += row;

    UString result;

    result.append(UString("["));    // OpenDocument format

    if (!colRelative)
        result.append(UString("$"));
    result.append(Cell::columnLabel(colRef));
    if (!rowRelative)
        result.append(UString("$"));
    result.append(UString::from(rowRef + 1));

    result.append(UString("]"));  // OpenDocument format

    return result;
}

UString FormulaToken::ref3d(const std::vector<UString>& externSheets, unsigned /*row*/, unsigned /*col*/) const
{
    if (version() != Excel97) {
        return UString("Unknown");
    }

    unsigned sheetRef = readU16(&d->data[0]);

    // FIXME check data size !
    // FIXME handle shared formula
    unsigned char buf[2];
    int rowRef, colRef;
    bool rowRelative, colRelative;

    buf[0] = d->data[2];
    buf[1] = d->data[3];
    rowRef = readU16(buf);

    buf[0] = d->data[4];
    buf[1] = d->data[5];
    colRef = readU16(buf);

    rowRelative = colRef & 0x8000;
    colRelative = colRef & 0x4000;
    colRef &= 0x3fff;

    UString result;

    result.append(UString("["));    // OpenDocument format

    if (sheetRef >= externSheets.size())
        result.append(UString("Error"));
    else
        result.append(externSheets[sheetRef]);
    result.append(UString("."));

    if (!colRelative)
        result.append(UString("$"));
    result.append(Cell::columnLabel(colRef));
    if (!rowRelative)
        result.append(UString("$"));
    result.append(UString::from(rowRef + 1));

    result.append(UString("]"));  // OpenDocument format


    return result;
}

std::pair<unsigned, unsigned> FormulaToken::baseFormulaRecord() const
{
    if (version() == Excel97) {
        return std::make_pair(readU16(&d->data[0]), readU16(&d->data[2]));
    } else {
        return std::make_pair(readU16(&d->data[0]), (unsigned)d->data[2]);
    }
}

std::ostream& operator<<(std::ostream& s,  Swinder::FormulaToken token)
{
    s << std::setw(2) << std::hex << token.id() << std::dec;
    // s  << "  Size: " << std::dec << token.size();
    s << "  ";

    switch (token.id()) {
    case FormulaToken::ErrorCode:
    case FormulaToken::Bool:
    case FormulaToken::Integer:
    case FormulaToken::Float:
    case FormulaToken::String: {
        Value v = token.value();
        s << v;
    }
    break;

    case FormulaToken::Function:
        s << "Function " << token.functionName();
        break;

    default:
        s << token.idAsString();
        break;
    }

    return s;
}


} // namespace Swinder