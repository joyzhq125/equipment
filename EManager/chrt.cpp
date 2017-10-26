#include "chrt.h"

const unsigned short int chrtype[256] = {
  CT_C, CT_C, CT_C, CT_C, CT_C, CT_C, CT_C, CT_C,
  CT_C,
  CT_C | CT_WS | CT_B | CT_T | CT_XWS, /* HT */
  CT_C | CT_WS | CT_XWS, /* LF */
  CT_C | CT_WS, /* VT */
  CT_C | CT_WS, /* FF */
  CT_C | CT_WS | CT_XWS, /* CR */
  CT_C, CT_C,
  CT_C, CT_C, CT_C, CT_C, CT_C, CT_C, CT_C, CT_C,
  CT_C, CT_C, CT_C, CT_C, CT_C, CT_C, CT_C, CT_C,
  CT_SP | CT_WS | CT_B | CT_T | CT_XWS, /* space */
  CT_M, /* '!' */
  CT_DL | CT_T, /* '"' */
  CT_DL, /* '#' */
  CT_SRES, /* '$' */
  CT_ESC | CT_DL, /* '%' */
  CT_SRES, /* '&' */
  CT_M, /* '\'' */
  CT_M | CT_T, /* '(' */
  CT_M | CT_T, /* ')' */
  CT_M, /* '*' */
  CT_SRES, /* '+' */
  CT_SRES | CT_T, /* ',' */
  CT_M, /* '-' */
  CT_M, /* '.' */
  CT_ARES | CT_T, /* '/' */
  CT_D, /* '0' */
  CT_D, /* '1' */
  CT_D, /* '2' */
  CT_D, /* '3' */
  CT_D, /* '4' */
  CT_D, /* '5' */
  CT_D, /* '6' */
  CT_D, /* '7' */
  CT_D, /* '8' */
  CT_D, /* '9' */
  CT_SRES | CT_T, /* ':' */
  CT_ARES | CT_T, /* ';' */
  CT_DL | CT_T, /* '<' */
  CT_SRES | CT_T, /* '=' */
  CT_DL | CT_T, /* '>' */
  CT_ARES | CT_T, /* '?' */
  CT_SRES | CT_T, /* '@' */
  CT_UC | CT_X, /* 'A' */
  CT_UC | CT_X, /* 'B' */
  CT_UC | CT_X, /* 'C' */
  CT_UC | CT_X, /* 'D' */
  CT_UC | CT_X, /* 'E' */
  CT_UC | CT_X, /* 'F' */
  CT_UC, /* 'G' */
  CT_UC, /* 'H' */
  CT_UC, /* 'I' */
  CT_UC, /* 'J' */
  CT_UC, /* 'K' */
  CT_UC, /* 'L' */
  CT_UC, /* 'M' */
  CT_UC, /* 'N' */
  CT_UC, /* 'O' */
  CT_UC, /* 'P' */
  CT_UC, /* 'Q' */
  CT_UC, /* 'R' */
  CT_UC, /* 'S' */
  CT_UC, /* 'T' */
  CT_UC, /* 'U' */
  CT_UC, /* 'V' */
  CT_UC, /* 'W' */
  CT_UC, /* 'X' */
  CT_UC, /* 'Y' */
  CT_UC, /* 'Z' */
  CT_UW | CT_T, /* '[' */
  CT_UW | CT_T, /* '\\' */
  CT_UW | CT_T, /* ']' */
  CT_UW, /* '^' */
  CT_M, /* '_' */
  CT_UW, /* '`' */
  CT_LC | CT_X, /* 'a' */
  CT_LC | CT_X, /* 'b' */
  CT_LC | CT_X, /* 'c' */
  CT_LC | CT_X, /* 'd' */
  CT_LC | CT_X, /* 'e' */
  CT_LC | CT_X, /* 'f' */
  CT_LC, /* 'g' */
  CT_LC, /* 'h' */
  CT_LC, /* 'i' */
  CT_LC, /* 'j' */
  CT_LC, /* 'k' */
  CT_LC, /* 'l' */
  CT_LC, /* 'm' */
  CT_LC, /* 'n' */
  CT_LC, /* 'o' */
  CT_LC, /* 'p' */
  CT_LC, /* 'q' */
  CT_LC, /* 'r' */
  CT_LC, /* 's' */
  CT_LC, /* 't' */
  CT_LC, /* 'u' */
  CT_LC, /* 'v' */
  CT_LC, /* 'w' */
  CT_LC, /* 'x' */
  CT_LC, /* 'y' */
  CT_LC, /* 'z' */
  CT_UW | CT_T, /* '{' */
  CT_UW, /* '|' */
  CT_UW | CT_T, /* '}' */
  CT_M, /* '~' */
  CT_C,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};