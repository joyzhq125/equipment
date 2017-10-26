
#ifndef _chrt_h
#define _chrt_h


extern unsigned const short int chrtype[256];

#define CT_UC    0x1u            /* Lower case: a-z */
#define CT_LC    0x2u            /* Upper case: A-Z */
#define CT_D     0x4u            /* Digit: 0-9 */
#define CT_X     0x8u            /* Hex digit: a-f, A-F */
#define CT_M     0x10u           /* Mark: -_.!~*'() */
#define CT_SRES  0x20u           /* Sometimes reserved: :@&=+$, */
#define CT_ARES  0x40u           /* Always reserved: ;/? */
#define CT_DL    0x80u           /* Delimiters: <>#%" */
#define CT_UW    0x100u          /* Unwise: {}|\^[]` */
#define CT_WS    0x200u          /* Whitespace: HT, VT, CR, LF, SP, FF */
#define CT_SP    0x400u          /* Space: ' ' */
#define CT_B     0x800u          /* Blank: SP TAB */
#define CT_C     0x1000u         /* Ctlr character: 0x0-0x1f, 0xff */
#define CT_T     0x2000u         /* Tspecials: ()<>@,;:\"/[]?={}SPHT */
#define CT_ESC   0x4000u         /* Escape character: % */
#define CT_XWS   0x8000u         /* XML whitespace character: HT, CR, LF, SP */

#define ct_isascii(c)         ((unsigned char)(c) < 0x80)
#define ct_isalpha(c)         (chrtype[(unsigned char)(c)] & (CT_UC | CT_LC))
#define ct_isalphanum(c)      (chrtype[(unsigned char)(c)] & (CT_UC | CT_LC | CT_D))
#define ct_islower(c)         (chrtype[(unsigned char)(c)] & CT_LC)
#define ct_isupper(c)         (chrtype[(unsigned char)(c)] & CT_UC)
#define ct_isdigit(c)         (chrtype[(unsigned char)(c)] & CT_D)
#define ct_ishex(c)           (chrtype[(unsigned char)(c)] & (CT_D | CT_X))
#define ct_ishexletter(c)     (chrtype[(unsigned char)(c)] & CT_X)
#define ct_isblank(c)         (chrtype[(unsigned char)(c)] & (CT_B))
#define ct_iswhitespace(c)    (chrtype[(unsigned char)(c)] & CT_WS)
#define ct_isxmlwhitespace(c) (chrtype[(unsigned char)(c)] & CT_XWS)
#define ct_isctrl(c)          (chrtype[(unsigned char)(c)] & CT_C)

#define ct_isescape(c)        (chrtype[(unsigned char)(c)] & CT_ESC)
#define ct_isreserved(c)      (chrtype[(unsigned char)(c)] & (CT_SRES | CT_ARES))
#define ct_isexcluded(c)      (chrtype[(unsigned char)(c)] & (CT_C | CT_SP | CT_DL))
#define ct_isdelimiter(c)     (chrtype[(unsigned char)(c)] & CT_DL)
#define ct_isunwise(c)        (chrtype[(unsigned char)(c)] & CT_UW)
#define ct_istspecial(c)      (chrtype[(unsigned char)(c)] & CT_T)
#define ct_isspecial(c)       (chrtype[(unsigned char)(c)] & (CT_C | CT_SP | CT_SRES | CT_ARES | CT_UW | CT_DL))
#define ct_ispchar(c)         (chrtype[(unsigned char)(c)] & (CT_UC | CT_LC | CT_D | CT_M | CT_SRES))
#define ct_isuric(c)          (chrtype[(unsigned char)(c)] & (CT_SRES | CT_ARES | CT_UC | CT_LC | CT_D | CT_M))

#endif //_chrt_h

