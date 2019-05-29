/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2019 FMSoft (http://www.fmsoft.cn).
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
/*!============================================================================
 * @file resource.h
 * @Synopsis header for resource definition 
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technologies Co., Ltd.
 */

#ifndef  __RESOURCE_INC
#define  __RESOURCE_INC

#if defined(SOFTKBD_480_272)
    #define SCREEN_W       480
    #define SCREEN_H       640
#elif defined(SOFTKBD_320_240)
    #define SCREEN_W       320
    #define SCREEN_H       480
#else
    #define SCREEN_W       240
    #define SCREEN_H       320
#endif

#ifdef _FULL_SCREEN
    #define STATUSBAR_X    0
    #define STATUSBAR_Y    0
    #define STATUSBAR_W    SCREEN_W
    #define STATUSBAR_H    0
#else
    #define STATUSBAR_X    0
    #define STATUSBAR_Y    0
    #define STATUSBAR_W    SCREEN_W
    #define STATUSBAR_H    24
#endif

#define ACTIVITY_X     0
#define ACTIVITY_Y     STATUSBAR_H
#define ACTIVITY_W     SCREEN_W
#define ACTIVITY_H     (SCREEN_H - STATUSBAR_H)

#define ACTIVITY_TITLE_H            24
#ifdef _HAVE_SUBTITLE
    #define ACTIVITY_SUBTITLE_H     20
#else
    #define ACTIVITY_SUBTITLE_H     0
#endif

#define ACTIVITY_TITLE_X        0
#define ACTIVITY_TITLE_Y        0
#define ACTIVITY_TITLE_W        ACTIVITY_W

#define ACTIVITY_SUBTITLE_X     0
#define ACTIVITY_SUBTITLE_Y     ACTIVITY_TITLE_H
#define ACTIVITY_SUBTITLE_W     ACTIVITY_W

#define ACTIVITY_SEPARATOR_X        0
#define ACTIVITY_SEPARATOR_Y        (ACTIVITY_TITLE_H + ACTIVITY_SUBTITLE_H)
#define ACTIVITY_SEPARATOR_H        2
#define ACTIVITY_SEPARATOR_W        ACTIVITY_W

#define ACTIVITY_CONTENT_X          0
#define ACTIVITY_CONTENT_Y          (ACTIVITY_TITLE_H + ACTIVITY_SUBTITLE_H + ACTIVITY_SEPARATOR_H)
#define ACTIVITY_CONTENT_W          ACTIVITY_W
#define ACTIVITY_CONTENT_H          (ACTIVITY_H - ACTIVITY_TITLE_H - ACTIVITY_SUBTITLE_H - ACTIVITY_SEPARATOR_H)

#define ACTIVITY_LEFT_BUTTON_X      10
#define ACTIVITY_LEFT_BUTTON_H      30
#define ACTIVITY_LEFT_BUTTON_Y      (ACTIVITY_H - ACTIVITY_LEFT_BUTTON_H - 10)
#define ACTIVITY_LEFT_BUTTON_W      (ACTIVITY_W/2 - 20)

#define ACTIVITY_RIGHT_BUTTON_X     (ACTIVITY_W/2 + 10)
#define ACTIVITY_RIGHT_BUTTON_H     30
#define ACTIVITY_RIGHT_BUTTON_Y     (ACTIVITY_H - ACTIVITY_RIGHT_BUTTON_H - 10)
#define ACTIVITY_RIGHT_BUTTON_W     (ACTIVITY_W/2 - 20)

#define ACTIVITY_CONTENT_MARGIN_H   10
#define ACTIVITY_CONTENT_MARGIN_V   10

#define ACTIVITY_ITEM_WITH_BRIEF_H  50
#define ACTIVITY_ITEM_WITH_BRIEF_W  ACTIVITY_W

#define ACTIVITY_CONTENT_ITEM_H     40
#define ACTIVITY_CONTENT_ITEM_W     ACTIVITY_W

#define TOAST_X                     0
#define TOAST_Y                     (SCREEN_H - 80)
#define TOAST_W                     SCREEN_W
#define TOAST_H                     50

// Ids for common controls
#define IDC_TITLE           701
#define IDC_SUBTITLE        702
#define IDC_LOADING         703
#define IDC_CONTENT         704
#define IDC_LEFT_BUTTON     705
#define IDC_RIGHT_BUTTON    706

// resource name
// global fonts
#if (_MINIGUI_VERSION_CODE >= _VERSION_CODE(4,0,0))
#define GLOBAL_FONT_XS      "ttf-HanSans-rrncns-*-12-UTF-8"
#define GLOBAL_FONT_SM      "ttf-HanSans-rrncns-*-14-UTF-8"
#define GLOBAL_FONT_NM      "ttf-HanSans-rrncns-*-16-UTF-8"
#define GLOBAL_FONT_LG      "ttf-HanSans-rrncns-*-20-UTF-8"
#define GLOBAL_FONT_XL      "ttf-HanSans-rrncns-*-26-UTF-8"
#define GLOBAL_FONT_XXL     "ttf-HanSans-rrncns-*-36-UTF-8"
#else
#define GLOBAL_FONT_XS      "ttf-HanSans-srncnn-*-12-UTF-8"
#define GLOBAL_FONT_SM      "ttf-HanSans-srncnn-*-14-UTF-8"
#define GLOBAL_FONT_NM      "ttf-HanSans-srncnn-*-16-UTF-8"
#define GLOBAL_FONT_LG      "ttf-HanSans-srncnn-*-20-UTF-8"
#define GLOBAL_FONT_XL      "ttf-HanSans-srncnn-*-26-UTF-8"
#define GLOBAL_FONT_XXL     "ttf-HanSans-srncnn-*-36-UTF-8"
#endif

// fonts for title, subtitle, and toast
#define FONT_TITLE      GLOBAL_FONT_NM
#define FONT_SUBTITLE   GLOBAL_FONT_SM
#define FONT_TOAST      GLOBAL_FONT_SM

// colors
#define COLOR_TITLE         0xFF6496C7L
#define COLOR_SUBTITLE      0xFFDDDDDDL
#define COLOR_TEXT_BUTTON   0xFF6496C7L
#define COLOR_SCROLL_ITEM   0xFF000000L
#define COLOR_SCROLLBAR     0xFF6496C7L

#define COLOR_STATE_NORMAL  0xFF16F235L
#define COLOR_STATE_ALARM   0xFF0C0CF5L

#define COLOR_TOAST_BK      0xFFE8E8E8L
#define COLOR_TOAST_DEF     0xFF000000L
#define COLOR_TOAST_INFO    0xFF333333L
#define COLOR_TOAST_WARNING 0xFF6496C7L
#define COLOR_TOAST_ERROR   0xFF0C0CF5L

#endif /* __RESOURCE_INC */

