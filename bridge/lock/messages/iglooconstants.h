#ifndef H_IGLOOCONSTANTS_
#define H_IGLOOCONSTANTS_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum IgSerializerError {
    IgSerializerNoError = 0  ,
    IgSerializerUnknownError = 1  ,
    IgSerializerErrorUnknownLength = 2  ,
    IgSerializerErrorAdvancePastEOF = 3  ,
    IgSerializerErrorIO = 4  ,
    IgSerializerErrorGarbageAtEnd = 256  ,
    IgSerializerErrorUnexpectedEOF = 257  ,
    IgSerializerErrorUnexpectedBreak = 258  ,
    IgSerializerErrorUnknownType = 259  ,
    IgSerializerErrorIllegalType = 260  ,
    IgSerializerErrorIllegalNumber = 261  ,
    IgSerializerErrorIllegalSimpleType = 262  ,
    IgSerializerErrorUnknownSimpleType = 512  ,
    IgSerializerErrorUnknownTag = 513  ,
    IgSerializerErrorInappropriateTagForType = 514  ,
    IgSerializerErrorDuplicateObjectKeys = 515  ,
    IgSerializerErrorInvalidUtf8TextString = 516  ,
    IgSerializerErrorTooManyItems = 768  ,
    IgSerializerErrorTooFewItems = 769  ,
    IgSerializerErrorDataTooLarge = 1024  ,
    IgSerializerErrorNestingTooDeep = 1025  ,
    IgSerializerErrorUnsupportedType = 1026  ,
    IgSerializerErrorJsonObjectKeyIsAggregate = 1027  ,
    IgSerializerErrorJsonObjectKeyNotString = 1028  ,
    IgSerializerErrorJsonNotImplemented = 1029  ,
    IgSerializerErrorInvalidTypeMsgId = 2001  ,
    IgSerializerErrorInvalidTypeTagId = 2002  ,
    IgSerializerErrorInvalidMsgIdValue = 2003  ,
    IgSerializerErrorInvalidPayloadType = 2004  ,
    IgSerializerErrorInvalidMsgIdKey = 2005  ,
    IgSerializerErrorUnknownTagId = 2006  ,
    IgSerializerErrorInvalidPayload = 2007
  } IgSerializerError;


  #ifdef __cplusplus
  }
  #endif

  #endif
