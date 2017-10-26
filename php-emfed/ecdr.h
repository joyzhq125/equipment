
#ifndef ECDR_H
#define ECDR_H




/**************************************************************************************
*   Type Define Section
**************************************************************************************/
/* Encoder, used for encode. */
typedef struct tagSt_ecdrEncoder
{
    int        iLength;    /* The length of encoded data */
    int        iPosition;  /* The current position */
    unsigned char       *pucData;    /* The encoded data */
} St_ecdrEncoder;

/* Decoder, used for decode */
typedef struct tagSt_ecdrDecoder
{
    int        iLength;    /* The length of decoded data */
    int        iPosition;  /* The current position */
    unsigned char       *pucData;    /* The decoded data */
} St_ecdrDecoder;

/***************************************************************************************
*   Prototype Declare Section
***************************************************************************************/
St_ecdrEncoder *ecdr_CreateEncoder
(
    int     iLength
);

/* Destroy the encoder structure */
int ecdr_DestroyEncoder
(
    St_ecdrEncoder *pstEncoder
);

/* Change the current buffer position of the conversion object */
int ecdr_ChangeEncodePosition
(
    St_ecdrEncoder  *pstEncoder,
    int            iOffset
);

/* encode fuctions */
int ecdr_EncodeUint8
(
    St_ecdrEncoder   *pstEncoder,
    unsigned char            *pucSrc
);

int ecdr_EncodeInt8
(
    St_ecdrEncoder   *pstEncoder,
    char             *pcSrc
);

int ecdr_EncodeUint16
(
    St_ecdrEncoder   *pstEncoder,
    unsigned short           *pusSrc
);

int ecdr_EncodeInt16
(
    St_ecdrEncoder   *pstEncoder,
    short            *psSrc
);

int ecdr_EncodeUint32
(
    St_ecdrEncoder   *pstEncoder,
    unsigned int           *puiSrc
);

int ecdr_EncodeInt32
(
    St_ecdrEncoder   *pstEncoder,
    int            *piSrc
);

int ecdr_EncodeUintVar
(
    St_ecdrEncoder   *pstEncoder,
    unsigned int            *puiSrc
);

int ecdr_EncodeOctets
(
    St_ecdrEncoder   *pstEncoder,
    char              *pcSrc,
    int             iSrcLength
);

/* Create a new Decoder structure */
St_ecdrDecoder *ecdr_CreateDecoder
(
    unsigned char    *pucData,
    unsigned int    uiDataLength
);

int ecdr_DestroyDecoder
(
    St_ecdrDecoder   *pstDecoder
);

/* Change the current buffer position of the conversion object */
int ecdr_ChangeDecodePosition
(
    St_ecdrDecoder  *pstDecoder,
    int            iOffset
);


/* decode fuctions */
int ecdr_DecodeUint8
(
    St_ecdrDecoder   *pstDecoder,
    unsigned char            *pucDst
);

int ecdr_DecodeInt8
(
    St_ecdrDecoder   *pstDecoder,
    char             *pcDst
);

int ecdr_DecodeUint16
(
    St_ecdrDecoder   *pstDecoder,
    unsigned short           *pusDst
);

int ecdr_DecodeInt16
(
    St_ecdrDecoder   *pstDecoder,
    short            *psDst
);

int ecdr_DecodeUint32
(
    St_ecdrDecoder   *pstDecoder,
    unsigned int           *puiDst
);

int ecdr_DecodeInt32
(
    St_ecdrDecoder   *pstDecoder,
    int            *piDst
);

int ecdr_DecodeUintVar
(
    St_ecdrDecoder   *pstDecoder,
    unsigned int            *puiDst
);

/* pcDst is Malloced by Invoker */
int ecdr_DecodeOctets
(
    St_ecdrDecoder   *pstDecoder,
    char              *pcDst,
    int            iDstLength
);

/* Get encode size */
int ecdr_GetEncodeUint8Size( void );

int ecdr_GetEncodeInt8Size( void );

int ecdr_GetEncodeUint16Size( void );

int ecdr_GetEncodeInt16Size( void );

int ecdr_GetEncodeUint32Size( void );

int ecdr_GetEncodeInt32Size( void );

/* Get uint var type length after encode */
int ecdr_GetEncodeUintVarSize
(
    unsigned int *puiSrc
);

int ecdr_GetEncodeOctetsSize
(
    int    iSrcLength
);

/* Return the number of bytes remaining in the Encoder data */
int ecdr_GetRemainLength
(
    St_ecdrEncoder *pstEncoder
);

int
ecdr_EncodeString (St_ecdrEncoder   *pstEncoder, char  *pcSrc);

int
ecdr_DecodeString (St_ecdrDecoder   *pstEncoder, char  **ppcDst);

#endif  /* end WAP_ECDR_H */

