/**
 * @file   ppttypes.h
 * @author Alex Ott <alexott@gmail.com>
 * @date   26 ‰≈À 2004
 * Version: $Id: ppttypes.h,v 1.1.1.1 2007/12/04 18:43:09 boitho Exp $
 * Copyright: Alex Ott
 * 
 * @brief  Enumerations for .ppt records
 * 
 * 
 */

#ifndef _PPTTYPES_H
#define _PPTTYPES_H 1

#define UNKNOWN         0
#define DOCUMENT        1000
#define DOCUMENT_ATOM   1001
#define DOCUMENT_END    1002
#define SLIDE_PERSIST   1003
#define SLIDE_BASE      1004
#define SLIDE_BASE_ATOM 1005
#define SLIDE           1006
#define SLIDE_ATOM      1007
#define NOTES           1008 
#define NOTES_ATOM      1009
#define ENVIRONMENT     1010
#define SLIDE_PERSIST_ATOM 1011
#define MAIN_MASTER     1016
#define SSSLIDE_INFO_ATOM 1017
#define SSDOC_INFO_ATOM 1025
#define EX_OBJ_LIST     1033
#define PPDRAWING_GROUP 1035
#define PPDRAWING       1036
#define LIST            2000
#define COLOR_SCHEME_ATOM 2032
#define TEXT_HEADER_ATOM 3999
#define TEXT_CHARS_ATOM 4000
#define STYLE_TEXT_PROP_ATOM 4001
#define TX_MASTER_STYLE_ATOM 4003
#define TEXT_BYTES_ATOM 4008
#define TEXT_CISTYLE_ATOM 4008
#define TEXT_SPEC_INFO  4010
#define EX_OLE_OBJ_STG  4113
#define CSTRING         4026
#define HANDOUT         4041
#define HEADERS_FOOTERS 4057
#define HEADERS_FOOTERS_ATOM 4058
#define SLIDE_LIST_WITH_TEXT 4080
#define SLIDE_LIST      4084
#define USER_EDIT_ATOM  4085
#define PROG_TAGS       5000
#define PROG_STRING_TAG 5001
#define PROG_BINARY_TAG 5002
#define PERSIST_PTR_INCREMENTAL_BLOCK 6002
/* #define  */
/* #define  */
/* #define  */
/* #define  */
/* #define  */

#endif /* _PPTTYPES_H */

