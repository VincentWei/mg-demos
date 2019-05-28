/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2010 ~ 2019 FMSoft (http://www.fmsoft.cn).
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*! ============================================================================
 * @file utilities.c
 * @Synopsis
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technology Co., Ltd.
 */

#include <math.h>

#include "global.h"
#include "utilities.h"

static HDC _refMemDC32b = HDC_INVALID;

HDC getRefMemDC32b (void)
{
    if (_refMemDC32b == HDC_INVALID) {
        _refMemDC32b = CreateMemDC (1, 1, 32, MEMDC_FLAG_SWSURFACE,
            0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    }

    return _refMemDC32b;
}

void releaseRefMemDC32b (void)
{
    if (_refMemDC32b != HDC_INVALID) {
        DeleteMemDC (_refMemDC32b);
        _refMemDC32b = HDC_INVALID;
    }
}

int loadResByTag (const RES_NODE *node_list, int tag)
{
    int cnt;

    const RES_NODE *res_node = node_list;

    cnt = 0;

    for (;;) {
        if (res_node->res_name == NULL) {
            break;
        }

        if ((tag == 0) || (res_node->tag == tag)) {
            void* res;
            if (res_node->bits == 32) {
                res = LoadResource (res_node->res_name, res_node->type, (DWORD) _refMemDC32b);
            }
            else {
                res = LoadResource (res_node->res_name, res_node->type, (DWORD) HDC_SCREEN);
            }

            if (res == NULL) {
                _ERR_PRINTF ("loadResByTag: failed to load resource: %s\n", res_node->res_name);
            }
            else {
                cnt++;
            }
        }

        res_node++;
    } /* for */

    return cnt;
}

int unloadResByTag (const RES_NODE *node_list, int tag)
{
    int cnt;
    const RES_NODE *res_node = node_list;

    cnt = 0;
    while (TRUE) {
        if (res_node->res_name == NULL) {
            break;
        }

        if ((tag == 0) || (res_node->tag == tag)) {
            ReleaseRes (Str2Key(res_node->res_name));
            cnt++;
        } /* if */

        res_node++;
    } /* for */

    return cnt;
}


HDC forceGetShadowDC (HWND hwnd, BOOL set_foreground)
{
    if (HWND_INVALID != hwnd) {
        HDC sec_dc = GetSecondaryDC (hwnd);
        HDC memdc = HDC_INVALID;
        BOOL has_secondary_dc = (sec_dc != HDC_SCREEN);

        if (!has_secondary_dc) {
            sec_dc = CreateSecondaryDC (hwnd);
        }

        memdc = CreateCompatibleDC (sec_dc);

        // Set memdc as secondary dc, and make the secondary dc update function to do nothing.
        SetSecondaryDC(hwnd, sec_dc, ON_UPDSECDC_DONOTHING);
        if (set_foreground) {
            ShowWindow(hwnd, SW_SHOW);
            UpdateWindow(hwnd, TRUE);
        }
        else {
            if (!IsWindowVisible (hwnd)) {
                IncludeWindowStyle (hwnd, WS_VISIBLE);
                UpdateWindow (hwnd, TRUE);
                ExcludeWindowStyle (hwnd, WS_VISIBLE);
            }
            else {
                UpdateWindow (hwnd, TRUE);
            }
        }

        if (HDC_INVALID != memdc) {
            BitBlt (sec_dc, 0, 0, -1, -1, memdc, 0, 0, -1);
        }

        /* If window haven't secondary dc ago.*/
        if (!has_secondary_dc) {
            DeleteSecondaryDC (hwnd);
        }
        else {
            SetSecondaryDC (hwnd, sec_dc, ON_UPDSECDC_DEFAULT);
        }

        return memdc;
    }

    return HDC_INVALID;
}

void humanSize (char* buff, int len, float size)
{
    size = fabsf (size);

    if (size >= 1073741824.0f) {
        size = size / 1073741824.0f;
        snprintf (buff, len, "%.2f GB", size);
    }
    else if (size >= 1048576.0f) {
        size = size / 1048576.0f;
        snprintf (buff, len, "%.2f MB", size);
    }
    else if (size >= 1024.0f) {
        size = size / 1024.0f;
        snprintf (buff, len, "%.2f KB", size);
    }
    else {
        unsigned int uSize = (unsigned int)size;
        snprintf (buff, len, "%d B", uSize);
    }
}

#if 0
#include <libxml/parser.h>
#include <libxml/xpath.h>

#define XML_ENCODE "UTF-8"

static xmlXPathObjectPtr get_nodeset(xmlDocPtr doc, const xmlChar *szXpath)
{
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    context = xmlXPathNewContext(doc);
    if (context == NULL)
    {
        printf("context is NULL\n");
        return NULL;
    }

    result = xmlXPathEvalExpression(szXpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL)
    {
        printf("xmlXPathEvalExpression return NULL\n");
        return NULL;
    }

    if (xmlXPathNodeSetIsEmpty(result->nodesetval))
    {
        xmlXPathFreeObject(result);
//        printf("nodeset is empty\n");
        return NULL;
    }

    return result;
}

int readXmlConfig (const char* file, const char* path, const char* attr, char* value, size_t size)
{
    if (NULL != file && NULL != path && NULL != value) {
        xmlNodePtr curNode = NULL;
        char* key = NULL;
        int ret = -1;
        xmlChar* szXpath = BAD_CAST(path);
        xmlXPathObjectPtr app_result;
        xmlDocPtr doc = xmlReadFile(file, XML_ENCODE, XML_PARSE_RECOVER);
        if (NULL == doc)
        {
            fprintf(stderr, "Document not parsed successfully.\n");
            return -1;
        }
        if (NULL != attr) {
            key = (char*)calloc(1, strlen (path) + strlen(attr) + strlen("[@]") + 1);
            if (NULL != key) {
                sprintf(key, "%s[@%s]", path, attr);
                szXpath = BAD_CAST(key);
            }
            else {
                fprintf(stderr, "Not enough memory!\n");
                return -1;
            }
        }
        app_result = get_nodeset(doc, szXpath);
        free(key);

        if(NULL != app_result) {
            xmlChar *szValue = NULL;
            xmlNodeSetPtr nodeset = app_result->nodesetval;
            if (nodeset->nodeNr > 0) {
                /* Take the first match node. */
                curNode = nodeset->nodeTab[0];
                if(curNode != NULL) {
                    if (NULL != attr) {
                        szValue = xmlGetProp(curNode, BAD_CAST(attr));
                        if (NULL != szValue) {
                            if (strlen((const char*)szValue) < size) {
                                strcpy(value, (char*)szValue);
                                ret = 0;
                            }
                            xmlFree(szValue);
                        }
                    }
                    else {
                        szValue = xmlNodeGetContent(curNode);
                        if (NULL != szValue) {
                            if (strlen((const char*)szValue) < size) {
                                strcpy(value, (char*)szValue);
                                ret = 0;
                            }
                            xmlFree(szValue);
                        }
                    }
                }
            }
            xmlXPathFreeObject (app_result);
        }
        else {
//            fprintf(stderr, "Node set is empty.\n");
        }
        xmlFreeDoc(doc);
        return ret;
    }
    fprintf(stderr, "Invalid arguments.\n");
    return -1;
}
#endif

