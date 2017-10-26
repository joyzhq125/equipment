
#include "em.h"
#include "ecdr.h"
St_ecdrEncoder *ecdr_CreateEncoder
(
    int     iLength
)
{
    St_ecdrEncoder   *pstEncoder = 
        (St_ecdrEncoder*)malloc (sizeof(St_ecdrEncoder) );

    if( (NULL == pstEncoder) || (iLength < 1) )
    {
        if (pstEncoder != NULL)
        {
            free(pstEncoder);
        }
        return NULL;
    }

    pstEncoder->pucData = (unsigned char*)malloc(iLength );
    if( NULL == pstEncoder->pucData )
    {
        free(pstEncoder);
        return NULL;
    }
    memset( (void *)(pstEncoder->pucData), 0, (unsigned int)iLength );
    pstEncoder->iLength = iLength;
    pstEncoder->iPosition = 0;
    
    return pstEncoder;
}


int ecdr_DestroyEncoder
(
    St_ecdrEncoder *pstEncoder
)
{
    if( NULL == pstEncoder )
    {
        return -1;
    }
    
    if( NULL != pstEncoder->pucData )
    {
        free( pstEncoder->pucData );
    }
    free( pstEncoder );
    return 0;
}


int ecdr_ChangeEncodePosition
(
    St_ecdrEncoder  *pstEncoder,
    int            iOffset
)
{
    int iCurrentPos = 0;
    
    if( NULL == pstEncoder )
    {
        return -1;
    }
    iCurrentPos = pstEncoder->iPosition + iOffset;
    if( iCurrentPos < 0 )
    {
        return -1;
    }
    pstEncoder->iPosition = iCurrentPos;
    return 0;
}


int ecdr_EncodeUint8
(
    St_ecdrEncoder   *pstEncoder,
    unsigned char            *pucSrc
)
{
    int iTmp = 0;
    
    if( (NULL == pstEncoder) || (NULL == pucSrc) )
    {
        return -1;
    }
    
    iTmp = pstEncoder->iPosition + 1;
    if( iTmp > pstEncoder->iLength )
    {
        return -1;
    }

    pstEncoder->pucData[pstEncoder->iPosition] = *pucSrc;
    pstEncoder->iPosition++;
    return 0;
}


int ecdr_EncodeInt8
(
    St_ecdrEncoder   *pstEncoder,
    char             *pcSrc
)
{
    int iTmp = 0;
    
    if( (NULL == pstEncoder) || (NULL == pcSrc) )
    {
        return -1;
    }
    
    iTmp = pstEncoder->iPosition + 1;
    if( iTmp > pstEncoder->iLength )
    {
        return -1;
    }

    pstEncoder->pucData[pstEncoder->iPosition] = (unsigned char)*pcSrc;
    pstEncoder->iPosition++;
    return 0;
}


int ecdr_EncodeUint16
(
    St_ecdrEncoder   *pstEncoder,
    unsigned short           *pusSrc
)
{
    int iTmp = 0;
    
    if( (NULL == pstEncoder) || (NULL == pusSrc) )
    {
        return -1;
    }
    
    iTmp = pstEncoder->iPosition + 2;
    if( iTmp > pstEncoder->iLength )
    {
        return -1;
    }
        
    pstEncoder->pucData[pstEncoder->iPosition] = (unsigned char)((*pusSrc>>8) & 0xFF);
    pstEncoder->pucData[pstEncoder->iPosition + 1] = (unsigned char)(*pusSrc & 0xFF);
    pstEncoder->iPosition += 2;
    return 0;
}


int ecdr_EncodeInt16
(
    St_ecdrEncoder   *pstEncoder,
    short            *psSrc
)
{
    int iTmp = 0;
    if( (NULL == pstEncoder) || (NULL == psSrc) )
    {
        return -1;
    }

    iTmp = pstEncoder->iPosition + 2;
    if( iTmp > pstEncoder->iLength )
    {
        return -1;
    }
       
    pstEncoder->pucData[pstEncoder->iPosition]   = (unsigned char)((*psSrc>>8) & 0xFF);
    pstEncoder->pucData[pstEncoder->iPosition + 1] = (unsigned char)(*psSrc & 0xFF);
    pstEncoder->iPosition += 2;
    return 0;
}


int ecdr_EncodeUint32
(
    St_ecdrEncoder   *pstEncoder,
    unsigned int           *puiSrc
)
{
    int iTmp = 0;
    if( (NULL == pstEncoder) || (NULL == puiSrc) )
    {
        return -1;
    }

    iTmp = pstEncoder->iPosition + 4;
    if( iTmp > pstEncoder->iLength )
    {
        return -1;
    }

    pstEncoder->pucData[pstEncoder->iPosition]   = (unsigned char)((*puiSrc>>24) & 0xFF);
    pstEncoder->pucData[pstEncoder->iPosition + 1] = (unsigned char)((*puiSrc>>16) & 0xFF);
    pstEncoder->pucData[pstEncoder->iPosition + 2] = (unsigned char)((*puiSrc>>8 ) & 0xFF);
    pstEncoder->pucData[pstEncoder->iPosition + 3] = (unsigned char)(*puiSrc & 0xFF);
    pstEncoder->iPosition += 4;
    return 0;
}


int ecdr_EncodeInt32
(
    St_ecdrEncoder   *pstEncoder,
    int            *piSrc
)
{
    int iTmp = 0;
    if( (NULL == pstEncoder) || (NULL == piSrc) )
    {
        return -1;
    }

    iTmp = pstEncoder->iPosition + 4;
    if( iTmp > pstEncoder->iLength )
    {
        return -1;
    }
    pstEncoder->pucData[pstEncoder->iPosition]   = (unsigned char)((*piSrc>>24) & 0xFF);
    pstEncoder->pucData[pstEncoder->iPosition + 1] = (unsigned char)((*piSrc>>16) & 0xFF);
    pstEncoder->pucData[pstEncoder->iPosition + 2] = (unsigned char)((*piSrc>>8 ) & 0xFF);
    pstEncoder->pucData[pstEncoder->iPosition + 3] = (unsigned char)(*piSrc & 0xFF);
    pstEncoder->iPosition += 4;
    return 0;
}


int ecdr_EncodeUintVar
(
    St_ecdrEncoder   *pstEncoder,
    unsigned int           *puiSrc
)
{
    int iEncodeVarSize = 0;
    int iCount = 0;
    int iTmp = 0;
    
    if( (NULL == pstEncoder) || (NULL == puiSrc) )
    {
        return -1;
    }
    
    iEncodeVarSize = ecdr_GetEncodeUintVarSize(puiSrc);
    iTmp = pstEncoder->iPosition + iEncodeVarSize;
    if( iTmp > pstEncoder->iLength )
    {
        return -1;
    }

    for( iCount = iEncodeVarSize; iCount > 0; iCount-- )
    {
        pstEncoder->pucData[pstEncoder->iPosition] 
                        = (unsigned char)((*puiSrc >> ((iCount - 1)*7)) & 0x7F);
        if( iCount < iEncodeVarSize )
        {
            pstEncoder->pucData[pstEncoder->iPosition - 1] |= 0x80;
        }
        pstEncoder->iPosition++ ;
    }
    return 0;
}


int ecdr_EncodeOctets
(
    St_ecdrEncoder   *pstEncoder,
    char              *pcSrc,
    int             iSrcLength
)
{
    int iTmp = 0;
    if( NULL == pstEncoder || (iSrcLength > 0 && NULL == pcSrc) )
    {
        return -1;
    }
    iTmp = pstEncoder->iPosition + iSrcLength;
    
    if( iTmp > pstEncoder->iLength )
    {
        return -1;
    }
    memcpy( (void *)(pstEncoder->pucData + pstEncoder->iPosition),(void *)pcSrc, (unsigned int)iSrcLength );
    pstEncoder->iPosition += iSrcLength;
    return 0;
}


int ecdr_EncodeString (St_ecdrEncoder   *pstEncoder, char  *pcSrc)
{
    int iLength = 0;
    int iTmp = 0;
    iLength = ((pcSrc != NULL) ? strlen (pcSrc) : 0);
    iTmp = pstEncoder->iPosition + iLength +1;
    if( iTmp > pstEncoder->iLength )
    {
        return -1;
    }
    memcpy (pstEncoder->pucData+ pstEncoder->iPosition,  pcSrc, iLength);
    (pstEncoder->pucData+ pstEncoder->iPosition)[iLength] = '\0';
    pstEncoder->iPosition += iLength + 1;

    return 0;
}

St_ecdrDecoder *ecdr_CreateDecoder
(
    unsigned char    *pucData,
    unsigned int    uiDataLength
)
{
    St_ecdrDecoder   *pstDecoder = NULL;

    if( NULL == pucData || uiDataLength < 1 )
    {
        return NULL;
    }
    pstDecoder = (St_ecdrDecoder*)malloc(sizeof(St_ecdrDecoder) );
    if( NULL == pstDecoder )
    {
        return NULL;
    }
    pstDecoder->pucData = (unsigned char*)malloc(uiDataLength);
    if( NULL == pstDecoder->pucData )
    {
        free(pstDecoder);
        return NULL;
    }
    memcpy( (void *)(pstDecoder->pucData),(void *)pucData, uiDataLength );
    pstDecoder->iLength = (int)uiDataLength;
    pstDecoder->iPosition = 0;
    return pstDecoder;
}


int ecdr_DestroyDecoder
(
    St_ecdrDecoder   *pstDecoder
)
{
    if( NULL == pstDecoder )
    {
        return -1;
    }
    if( NULL != pstDecoder->pucData )
    {
        free(pstDecoder->pucData);
    }
    free(pstDecoder);
    return 0;
}


int ecdr_ChangeDecodePosition
(
    St_ecdrDecoder  *pstDecoder,
    int            iOffset
)
{
    int iCurrentPos = 0;
    
    if( NULL == pstDecoder )
    {
        return -1;
    }
    iCurrentPos = pstDecoder->iPosition + iOffset;
    if( (iCurrentPos < 0) || (iCurrentPos > pstDecoder->iLength) )
    {
        return -1;
    }
    pstDecoder->iPosition = iCurrentPos;
    return 0;
}


int ecdr_DecodeUint8
(
    St_ecdrDecoder   *pstDecoder,
    unsigned char            *pucDst
)
{
    int iTmp = 0;
    
    if( NULL == pstDecoder || NULL == pstDecoder->pucData || NULL == pucDst )
    {
        return -1;
    }
    
    iTmp = pstDecoder->iPosition + 1;
    if( iTmp > pstDecoder->iLength)
    {
        return -1;
    }
    *pucDst = pstDecoder->pucData[pstDecoder->iPosition];
    pstDecoder->iPosition++;
    return 0;
}


int ecdr_DecodeInt8
(
    St_ecdrDecoder   *pstDecoder,
    char             *pcDst
)
{
    int iTmp = 0;
    
    if( NULL == pstDecoder || NULL == pstDecoder->pucData || NULL == pcDst )
    {
        return -1;
    }
    
    iTmp = pstDecoder->iPosition + 1;
    if( iTmp > pstDecoder->iLength)
    {
        return -1;
    }

    *pcDst = (char)pstDecoder->pucData[pstDecoder->iPosition];
    pstDecoder->iPosition++;
    return 0;
}


int ecdr_DecodeUint16
(
    St_ecdrDecoder   *pstDecoder,
    unsigned short           *pusDst
)
{
    unsigned char ucTmp1 = '\0';
    unsigned char ucTmp2 = '\0';
    int iTmp = 0;
    
    if( NULL == pstDecoder || NULL == pstDecoder->pucData || NULL == pusDst )
    {
        return -1;
    }
    
    iTmp = pstDecoder->iPosition + 2;
    if( iTmp > pstDecoder->iLength)
    {
        return -1;
    }

    ucTmp1 = pstDecoder->pucData[pstDecoder->iPosition];
    ucTmp2 = pstDecoder->pucData[pstDecoder->iPosition + 1];
    pstDecoder->iPosition += 2;

    *pusDst = (unsigned short)(((unsigned short)(ucTmp1 << 8)) | ((unsigned short)ucTmp2));
    return 0;
}


int ecdr_DecodeInt16
(
    St_ecdrDecoder   *pstDecoder,
    short            *psDst
)
{
    unsigned char ucTmp1 = '\0';
    unsigned char ucTmp2 = '\0';
    int iTmp = 0;
    
    if( NULL == pstDecoder || NULL == pstDecoder->pucData || NULL == psDst )
    {
        return -1;
    }
    
    iTmp = pstDecoder->iPosition + 2;
    if( iTmp > pstDecoder->iLength)
    {
        return -1;
    }
    ucTmp1 = pstDecoder->pucData[pstDecoder->iPosition];
    ucTmp2 = pstDecoder->pucData[pstDecoder->iPosition + 1];
    pstDecoder->iPosition += 2;

    *psDst = (short)(((short)(ucTmp1 << 8)) | ((short)ucTmp2));
    return 0;
}


int ecdr_DecodeUint32
(
    St_ecdrDecoder   *pstDecoder,
    unsigned int           *puiDst
)
{
    unsigned char ucTmp1 = '\0';
    unsigned char ucTmp2 = '\0';
    unsigned char ucTmp3 = '\0';
    unsigned char ucTmp4 = '\0';
    int iTmp = 0;
    
    if( NULL == pstDecoder || NULL == pstDecoder->pucData || NULL == puiDst )
    {
        return -1;
    }
    
    iTmp = pstDecoder->iPosition + 4;
    if( iTmp > pstDecoder->iLength)
    {
        return -1;
    }
    ucTmp1 = pstDecoder->pucData[pstDecoder->iPosition];
    ucTmp2 = pstDecoder->pucData[pstDecoder->iPosition + 1];
    ucTmp3 = pstDecoder->pucData[pstDecoder->iPosition + 2];
    ucTmp4 = pstDecoder->pucData[pstDecoder->iPosition + 3];
    pstDecoder->iPosition += 4;

    *puiDst = (unsigned int)( ((unsigned int)(ucTmp1 << 24)) | ((unsigned int)(ucTmp2 << 16)) | \
                           ((unsigned int)(ucTmp3 << 8 )) | ((unsigned int)ucTmp4 ) );
    return 0;
}


int ecdr_DecodeInt32
(
    St_ecdrDecoder   *pstDecoder,
    int            *piDst
)
{
    unsigned char ucTmp1 = '\0';
    unsigned char ucTmp2 = '\0';
    unsigned char ucTmp3 = '\0';
    unsigned char ucTmp4 = '\0';
    int iTmp = 0;
    
    if( NULL == pstDecoder || NULL == pstDecoder->pucData || NULL == piDst )
    {
        return -1;
    }
    
    iTmp = pstDecoder->iPosition + 4;
    if( iTmp > pstDecoder->iLength)
    {
        return -1;
    }
    
    ucTmp1 = pstDecoder->pucData[pstDecoder->iPosition];
    ucTmp2 = pstDecoder->pucData[pstDecoder->iPosition + 1];
    ucTmp3 = pstDecoder->pucData[pstDecoder->iPosition + 2];
    ucTmp4 = pstDecoder->pucData[pstDecoder->iPosition + 3];
    pstDecoder->iPosition += 4;

    *piDst = (int)(((int)(ucTmp1 << 24)) | ((int)(ucTmp2 << 16)) | \
                        ((int)(ucTmp3 << 8 )) | ((int)ucTmp4) );
    return 0;
}


int ecdr_DecodeUintVar
(
    St_ecdrDecoder   *pstDecoder,
    unsigned int           *puiDst
)
{
    unsigned char iCount = 0;
    unsigned char ucTmp = '\0';
    
    if( NULL == pstDecoder || NULL == pstDecoder->pucData || NULL == puiDst )
    {
        return -1;
    }

    *puiDst = 0;
    for( iCount = 0; iCount < 5; iCount++ )
    {
        if( pstDecoder->iPosition + 1 > pstDecoder->iLength )
        {
            *puiDst = 0;
            return -1;
        }
        ucTmp = pstDecoder->pucData[pstDecoder->iPosition++];

        if( 0 != *puiDst )
        {
            *puiDst <<= 7;
        }
        *puiDst |= (unsigned int)(ucTmp & 0x7f);
        if( ucTmp < 0x80 )   /* The last 7bits */
        {
            return 0;
        }
        if( 4 == iCount )        /* The decoder exceed 5 byte, the data is error */
        {
            *puiDst = 0;
            return -1;
        }
    }
    
    return 0;
}


int ecdr_DecodeOctets
(
    St_ecdrDecoder   *pstDecoder,
    char              *pcDst,
    int            iDstLength
)
{
    int iTmp;
    if( NULL == pstDecoder || NULL == pcDst || iDstLength < 0 )
    {
        return -1;
    }
    iTmp = pstDecoder->iPosition + iDstLength;
    if (iTmp > pstDecoder->iLength )
    {
    return -1;
    }

    memcpy( pcDst, (pstDecoder->pucData) + (pstDecoder->iPosition), (unsigned int)iDstLength );
    pstDecoder->iPosition += iDstLength;
    
    return 0;
}


int
ecdr_DecodeString (St_ecdrDecoder   *pstEncoder, char  **ppcDst)
{
    char  *pcBuf = NULL;
    int        iLength = 0;;
    int        iIndex = 0;
    iLength = pstEncoder->iLength - pstEncoder->iPosition;
    pcBuf = (char *)(pstEncoder->pucData + pstEncoder->iPosition);
    
    for (iIndex = 0; iIndex < iLength; iIndex++) 
    {
      if (pcBuf[iIndex] == '\0')
      {
        break;
      }
    }
    if (iIndex == iLength) 
    {
      *ppcDst = NULL;
      return -1;
    }
    iLength = iIndex;
    if (iLength == 0) 
    {
      *ppcDst = NULL;
    }
    else 
    {
      *ppcDst = (char*)malloc (iLength + 1);
      memcpy ((void *)*ppcDst, pstEncoder->pucData+ pstEncoder->iPosition, iLength);
      ((char  *)*ppcDst)[iLength] = '\0';
    }
    pstEncoder->iPosition+= iLength + 1;

    return 0;
}


int ecdr_GetEncodeUint8Size( void )
{
    return 1;
}


int ecdr_GetEncodeInt8Size( void )
{
    return 1;
}


int ecdr_GetEncodeUint16Size( void )
{
    return 2;
}

int ecdr_GetEncodeInt16Size( void )
{
    return 2;
}


int ecdr_GetEncodeUint32Size( void )
{
    return 4;
}


int ecdr_GetEncodeInt32Size( void )
{
    return 4;
}


int ecdr_GetEncodeUintVarSize
(
    unsigned int *puiSrc
)
{
    if( NULL == puiSrc )
    {
        return -1;
    }
    
    if( *puiSrc < (unsigned int)0x80 )              /* 1 0000000 */
    {
        return 1;
    }
    else if( *puiSrc < (unsigned int)0x4000 )      /* 1 0000000 0000000 */
    {
        return 2;
    }
    else if( *puiSrc < (unsigned int)0x200000 )     /* 1 0000000 0000000 0000000 */
    {
        return 3;
    }
    else if( *puiSrc < (unsigned int)0x10000000 )   /* 1 0000000 0000000 0000000 0000000 */
    {
        return 4;
    }
    else
    {
        return 5;
    }
}


int ecdr_GetEncodeOctetsSize
(
    int    iSrcLength
)
{
    if( iSrcLength < 0 )
    {
        return -1;
    }
    
    return iSrcLength;
}


int ecdr_GetRemainLength
(
    St_ecdrEncoder *pstEncoder
)
{
    int iRemLength = 0;
    if( NULL == pstEncoder || pstEncoder->iLength < pstEncoder->iPosition )
    {
        return -1;
    }
    iRemLength = pstEncoder->iLength - pstEncoder->iPosition;
    return iRemLength;
}

