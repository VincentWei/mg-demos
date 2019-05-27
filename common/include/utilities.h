/*!============================================================================
 * @file utility.h 
 * @Synopsis  
 * @author DongKai
 * @version 1.0
 *
 *  Copyright:
 *
 *      Beijing FMSoft Technologies Co., Ltd.
 *      Beijing Joobot Inc.
 *          
 */

#ifndef  __UTILITY_H
#define  __UTILITY_H

#ifdef __cplusplus
extern "C" {
#endif

HDC getRefMemDC32b (void);
void releaseRefMemDC32b (void);

/* \brief struct of resource node */
typedef struct _RES_NODE {
    const char *res_name;
    int         type;
    int         bits;
    int         tag;
} RES_NODE;

int loadResByTag (const RES_NODE *node_list, int tag);
int unloadResByTag (const RES_NODE *node_list, int tag);

#define loadAllRes(node_list) stepLoadRes (node_list, 0)

HDC forceGetShadowDC (HWND hwnd, BOOL set_foreground);

void humanSize (char* buff, int len, float size);

#ifdef __cplusplus
}
#endif

#endif   /* __UTILITY_H */

