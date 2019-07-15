///////////////////////////////////////////////////////////////////////////////
//
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
/*!============================================================================
 * @file LauncherActivity.cc 
 * @Synopsis The activity for WiFi password.
 * @author Vincent Wei
 * @version 1.0
 * @date 23/05/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "resource.h"

#include "SysConfService.hh"
#include "MiscService.hh"
#include "LauncherActivity.hh"

REGISTER_ACTIVITY(LauncherActivity);

typedef struct _APP_INFO {
    int id;
    const char* activity_name;
    const char* res_name;
} APP_INFO;

#define IDC_APP_MIN             100

static APP_INFO _myapp_info [] = {
    {IDC_APP_MIN + 0,  "AUCActivity",           "images/launcher/icon-app-auto-upload.png"},
    {IDC_APP_MIN + 1,  "MUCActivity",           "images/launcher/icon-app-manual-upload.png"},
    {IDC_APP_MIN + 2,  "SettingWifiActivity",   "images/launcher/icon-app-settings-wifi.png"},
    {IDC_APP_MIN + 3,  "SettingMainActivity",   "images/launcher/icon-app-settings.png"},
#if 0
    {IDC_APP_MIN + 2,  "RmtCtlWLANActivity",    "images/launcher/icon-app-remote-control-wlan.png"},
    {IDC_APP_MIN + 3,  "RmtCtlHotspotActivity", "images/launcher/icon-app-remote-control-hotspot.png"},
    {IDC_APP_MIN + 4,  "RmtCtlCloudActivity",   "images/launcher/icon-app-remote-control-cloud.png"},
    {IDC_APP_MIN + 5,  "LivePushCloudActivity", "images/launcher/icon-app-live-push.png"},
    {IDC_APP_MIN + 6,  "SettingMainActivity",   "images/launcher/icon-app-settings.png"},
#endif
};

#define RES_ICON_CAMERA     "images/launcher/icon-camera.png"
#define RES_ICON_CLOUD      "images/launcher/icon-cloud.png"
#define RES_ICON_FTP        "images/launcher/icon-ftp.png"

static RES_NODE _myres_list [] = {
    {RES_ICON_CAMERA, RES_TYPE_IMAGE, 16, 0},
    {RES_ICON_CLOUD,  RES_TYPE_IMAGE, 16, 0},
    {RES_ICON_FTP,    RES_TYPE_IMAGE, 16, 0},
    {NULL},
};

static BOOL load_res (void)
{
    unsigned u;
    RES_NODE res_list [] = {
        { NULL, RES_TYPE_IMAGE, 16, 0},
        { NULL},
    };

    for (u = 0; u < TABLESIZE (_myapp_info); u++) {
        res_list [0].res_name = _myapp_info[u].res_name;
        if (loadResByTag (res_list, 0) == 0) {
            _ERR_PRINTF ("LauncherActivity: failed to load resource: %s.\n", res_list [0].res_name);
            return FALSE;
        }
    }

    u = loadResByTag (_myres_list, 0);
    if (u < (TABLESIZE (_myres_list) - 1)) {
        _ERR_PRINTF ("LauncherActivity: failed to load resource.\n");
        return FALSE;
    }

    return TRUE;
}

static void unload_res (void)
{
    unsigned u;
    RES_NODE res_list [] = {
        { NULL, RES_TYPE_IMAGE, 16, 0},
        { NULL },
    };

    for (u = 0; u < TABLESIZE (_myapp_info); u++) {
        res_list [0].res_name = _myapp_info[u].res_name;
        unloadResByTag (res_list, 0);
    }

    unloadResByTag (_myres_list, 0);
}

#define IDC_CAMERA_ICON     100
#define IDC_CAMERA_NAME     101

#define IDC_CLOUD_ICON      201

#define IDC_CLOUD_SPACE_TXT 202
#define IDC_CLOUD_SPACE_BAR 203

/* for OEM controls */
#define IDC_ORDER_STATUS    212
#define IDC_ORDER_REFRESH   213
#define IDC_ORDER_NUMBER    214

#define ICON_SIZE_H         38
#define ICON_SIZE_W         50

#define BUTTON_SIZE_W       50
#define BUTTON_SIZE_H       30

#define MARGIN_H            10
#define MARGIN_V            20

#define APP_ICON_SIZE       56

static void set_camera_name (mWidget* ctrl, const char* vendor, const char* model)
{
    if (vendor && model) {
        char camera_name [32];
        snprintf (camera_name, 30, "%s %s", vendor, model);
        if (strlen (camera_name) > 4) {
            SetWindowText (ctrl->hwnd, camera_name);
        }
        else {
            SetWindowText (ctrl->hwnd, _("Disconnected"));
        }
    }
    else {
        SetWindowText (ctrl->hwnd, _("Disconnected"));
    }
}

static bool set_cloud_icon (mMainWnd* self)
{
    mWidget* ctrl;
    if (SYSCFGSERVICE->getTransferProtocol() == SYS_TRANSFER_PROTOCOL_FTP) {
        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_ICON));
        _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_ICON_FTP)));
        return false;
    }
    else {
        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_ICON));
        _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_ICON_CLOUD)));

        return true;
    }
}

static void set_cloud_info (mMainWnd* self, float space_free, float space_total)
{
    mWidget* ctrl;
    float space_used = space_total - space_free;

    if (space_total < 1.0f || space_free > space_total) {
        _ERR_PRINTF ("AUCActivity: bad cloud space data (%f/%f).\n", space_free, space_total);
        return;
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_SPACE_TXT));
    if (ctrl) {
        char size_used [16], size_total [16], size_all [32];
        humanSize (size_used, 15, space_used);
        humanSize (size_total, 15, space_total);

        strcpy (size_all, size_used);
        strcat (size_all, "/");
        strcat (size_all, size_total);

        SetWindowText (ctrl->hwnd, size_all);
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_SPACE_BAR));
    if (ctrl) {
        int percent = (int)(space_used*100.0f/space_total);
        _c(ctrl)->setProperty (ctrl, NCSP_PROG_CURPOS, (DWORD)percent);
    }
}

static const char* upload_status[] = {
    N_("Idle"),
    N_("Reading: %s"),
    N_("Uploading: %s"),
    N_("Failed: %s"),
    N_("Unknown"),
};

static void set_upload_info (mMainWnd* self, int status, char* name)
{
    mWidget* ctrl;

    if (status < UPLOAD_STATUS_IDLE && status > UPLOAD_STATUS_UNKNOWN)
        status = UPLOAD_STATUS_UNKNOWN;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_SPACE_TXT));
    if (ctrl) {
        char buff[256];

        memset(buff, 0, 256);
        switch (status) {
        case UPLOAD_STATUS_IDLE:
        case UPLOAD_STATUS_UNKNOWN:
            strncpy(buff, _(upload_status[status]), 255);
            break;

        default:
            if (name)
                snprintf(buff, 255, _(upload_status[status]), name);
            else
                snprintf(buff, 255, _(upload_status[status]), "");
            break;
        }

        SetWindowText (ctrl->hwnd, buff);
    }

    if (name)
        free (name);

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_SPACE_BAR));
    if (ctrl) {
        int percent;

        switch (status) {
        case UPLOAD_STATUS_READING:
            percent = 30;
            break;
        case UPLOAD_STATUS_UPLOADING:
            percent = 80;
            break;

        default:
            percent = 0;
            break;
        }

        _c(ctrl)->setProperty (ctrl, NCSP_PROG_CURPOS, (DWORD)percent);
    }
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    if (!load_res ()) {
        return FALSE;
    }

    mWidget* ctrl;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CAMERA_ICON));
    if (ctrl)
        _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_ICON_CAMERA)));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CAMERA_NAME));
    if (ctrl) {
        set_camera_name (ctrl, statusBar_getDevInfo (SBINFO_DEV_CAMERA_VENDOR), statusBar_getDevInfo (SBINFO_DEV_CAMERA_MODEL));
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_ICON));
    if (ctrl) {
        _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_ICON_CLOUD)));
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_ORDER_REFRESH));
    if (ctrl)
        ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TITLE);

    NCS_PROP_ENTRY props [] = {
        { NCSP_IMAGE_IMAGE, 0 },
        { 0, 0 }
    };

    NCS_WND_TEMPLATE ctrl_templ = {};
    ctrl_templ.class_name = NCSCTRL_IMAGE;
    ctrl_templ.style = WS_VISIBLE | NCSS_NOTIFY;
    ctrl_templ.ex_style = WS_EX_NONE;
    ctrl_templ.w = APP_ICON_SIZE;
    ctrl_templ.h = APP_ICON_SIZE;
    ctrl_templ.props = props;

    unsigned u;
    for (u = 0; u < TABLESIZE (_myapp_info); u++) {
        props [0].value = (DWORD)GetResource (Str2Key (_myapp_info[u].res_name));

        ctrl_templ.id = _myapp_info[u].id;
        ctrl_templ.caption = _myapp_info[u].activity_name;
        ctrl_templ.x = (ACTIVITY_W - APP_ICON_SIZE * 4)/2 + APP_ICON_SIZE * (u % 4);
        ctrl_templ.y = ICON_SIZE_H*2 + 60 + APP_ICON_SIZE * (u / 4);

        ncsCreateWindowIndirect (&ctrl_templ, ((mWidget*)self)->hwnd);
    }

    if (set_cloud_icon ((mMainWnd*)self)) {
        set_cloud_info ((mMainWnd*)self, statusBar_getSpaceInfo (SBINFO_SPACE_FREE),
                statusBar_getSpaceInfo (SBINFO_SPACE_TOTAL));
    }
    else {
        set_upload_info ((mMainWnd*)self, UPLOAD_STATUS_IDLE, NULL);
    }

    return TRUE;
}

static LRESULT mymain_onIntent (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    Intent* intent = (Intent*)wparam;
    if (intent)
        Intent::deleteIntent (intent);
    return 0;
}

static LRESULT mymain_onReady (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    if (wparam == ACTIVITY_POPPED) {
        mWidget* ctrl;
        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CAMERA_NAME));
        if (ctrl) {
            set_camera_name (ctrl, statusBar_getDevInfo (SBINFO_DEV_CAMERA_VENDOR),
                    statusBar_getDevInfo (SBINFO_DEV_CAMERA_MODEL));
        }

        if (set_cloud_icon ((mMainWnd*)self)) {
            set_cloud_info (self, statusBar_getSpaceInfo (SBINFO_SPACE_FREE),
                    statusBar_getSpaceInfo (SBINFO_SPACE_TOTAL));
        }
        else {
            set_upload_info ((mMainWnd*)self, UPLOAD_STATUS_IDLE, NULL);
        }
    }

    return 0;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("LauncherActivity: mymain_onClose called.\n");

    unload_res ();
    return TRUE;
}

static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    if (wparam == TASKID_MISC_UPDATE_STATUS) {
        mWidget* ctrl = (mWidget*)(ncsGetChildObj(self->hwnd, IDC_ORDER_REFRESH));
        EnableWindow (ctrl->hwnd, TRUE);
    }
    else {
        _DBG_PRINTF ("LauncherActivity: Unknown task.\n");
    }

    return 0L;
}

static BOOL mymain_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    if (nc == NCSN_WIDGET_CLICKED) {
        if (id >= IDC_APP_MIN && id < (IDC_APP_MIN + (int)TABLESIZE (_myapp_info))) {
            ACTIVITYSTACK->push (_myapp_info [id - IDC_APP_MIN].activity_name);
        }
        else if (id == IDC_ORDER_REFRESH) {
            EnableWindow (hCtrl, FALSE);
            MISCSERVICE->updateStatus (self->hwnd);
        }
    }

    return TRUE;
}

static LRESULT mymain_onDevChanged (mMainWnd* self, UINT msg, WPARAM wparam, LPARAM lparam)
{
    mWidget* ctrl;
    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CAMERA_NAME));
    if (ctrl) {
        set_camera_name (ctrl, (const char*)wparam, (const char*)lparam);
    }
    return 0;
}

static LRESULT mymain_onStateChanged (mMainWnd* self, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
    case MSG_CLOUD_CHANGED: {
        if (SYSCFGSERVICE->getTransferProtocol() != SYS_TRANSFER_PROTOCOL_FTP) {
            set_cloud_info (self, statusBar_getSpaceInfo (SBINFO_SPACE_FREE),
                    statusBar_getSpaceInfo (SBINFO_SPACE_TOTAL));
        }
        break;
    }

    case MSG_UPLOAD_CHANGED: {
        if (SYSCFGSERVICE->getTransferProtocol() == SYS_TRANSFER_PROTOCOL_FTP) {
            set_upload_info (self, (int)wparam, (char*)lparam);
        }
        break;
    }

    default:
        break;
    }

    return 0;
}

static LRESULT mymain_onOrderChanged (mMainWnd* self, UINT msg, WPARAM wparam, LPARAM lparam)
{
    mWidget* ctrl;

    switch (msg) {
    case MSG_ORDER_CHANGED: {
        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_ORDER_STATUS));
        if (ctrl) {
            const char* status;
            switch (wparam) {
            case ORDER_STATUS_OK:
                status = "订单：正常";
                break;
            case ORDER_STATUS_NONE:
                status = "订单：无";
                break;
            default:
                status = "订单：未知";
                break;
            }

            SetWindowText (ctrl->hwnd, status);
        }

        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_ORDER_NUMBER));
        if (ctrl) {
            if (lparam) {
                char* order_no = (char*)lparam;
                SetWindowText (ctrl->hwnd, order_no);
                free (order_no);
            }
            else {
                SetWindowText (ctrl->hwnd, "N/A");
            }
        }
        break;
    }

    default:
        break;
    }

    return 0;
}

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_USER_APP_READY, reinterpret_cast<void*>(mymain_onReady)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_USBDEV_CHANGED, reinterpret_cast<void*>(mymain_onDevChanged)},
    {MSG_CLOUD_CHANGED, reinterpret_cast<void*>(mymain_onStateChanged)},
    {MSG_UPLOAD_CHANGED, reinterpret_cast<void*>(mymain_onStateChanged)},
    {MSG_ORDER_CHANGED, reinterpret_cast<void*>(mymain_onOrderChanged)},
    {MSG_ASYNC_TASK_FINISHED, reinterpret_cast<void*>(mymain_onAsyncTaskFinished)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {0, NULL}
};

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_PROP_ENTRY _props_static [] = {
    {NCSP_STATIC_ALIGN, NCS_ALIGN_LEFT},
    {-1, 0},
};

static NCS_PROP_ENTRY _props_space [] = {
    {NCSP_PROG_MAXPOS, 100},
    {NCSP_PROG_MINPOS, 0},
    {NCSP_PROG_LINESTEP, 10},
    {NCSP_PROG_CURPOS, 0},
    {-1, 0},
};

static NCS_WND_TEMPLATE _ctrl_templ[] =
{
    {
        class_name: NCSCTRL_IMAGE,
        id: IDC_CAMERA_ICON,
        x: MARGIN_H,
        y: MARGIN_V,
        w: ICON_SIZE_W,
        h: ICON_SIZE_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_CAMERA_NAME,
        x: ICON_SIZE_W + MARGIN_H,
        y: MARGIN_V,
        w: ACTIVITY_W - ICON_SIZE_W - 5,
        h: ICON_SIZE_W,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
        props: _props_static,
    },
    {
        class_name: NCSCTRL_IMAGE,
        id: IDC_CLOUD_ICON,
        x: MARGIN_H,
        y: MARGIN_V + ICON_SIZE_H,
        w: ICON_SIZE_W,
        h: ICON_SIZE_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_CLOUD_SPACE_TXT,
        x: ICON_SIZE_W + MARGIN_H,
        y: MARGIN_V + ICON_SIZE_H,
        w: ACTIVITY_W - ICON_SIZE_W - MARGIN_H*2,
        h: ICON_SIZE_H - 5,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "0 GB/0 GB",
        props: _props_static,
    },
    {
        class_name: NCSCTRL_PROGRESSBAR,
        id: IDC_CLOUD_SPACE_BAR,
        x: MARGIN_H + ICON_SIZE_W,
        y: MARGIN_V + ICON_SIZE_H + ICON_SIZE_H - 5,
        w: ACTIVITY_W - ICON_SIZE_W - MARGIN_H*2,
        h: 5,
        style: WS_VISIBLE | WS_BORDER,
        ex_style: WS_EX_NONE,
        caption: "",
        props: _props_space,
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_ORDER_STATUS,
        x: ICON_SIZE_W + MARGIN_H,
        y: MARGIN_V + ICON_SIZE_H,
        w: ACTIVITY_W - ICON_SIZE_W - MARGIN_H*2 - BUTTON_SIZE_W,
        h: ICON_SIZE_H/2,
        style: WS_NONE,
        ex_style: WS_EX_NONE,
        caption: "订单：未知",
        props: _props_static,
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_ORDER_NUMBER,
        x: ICON_SIZE_W + MARGIN_H,
        y: MARGIN_V + ICON_SIZE_H + ICON_SIZE_H/2,
        w: ACTIVITY_W - ICON_SIZE_W - MARGIN_H*2,
        h: ICON_SIZE_H/2,
        style: WS_NONE,
        ex_style: WS_EX_NONE,
        caption: "N/A",
        props: _props_static,
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_ORDER_REFRESH,
        x: ACTIVITY_W - BUTTON_SIZE_W - MARGIN_H,
        y: MARGIN_V + ICON_SIZE_H,
        w: BUTTON_SIZE_W,
        h: ICON_SIZE_H / 2,
        style: WS_NONE,
        ex_style: WS_EX_NONE,
        caption: "刷新",
    },
    {
        class_name: NCSCTRL_SEPARATOR,
        id: IDC_STATIC,
        x: 0,
        y: MARGIN_V + ICON_SIZE_H*2 + 20,
        w: ACTIVITY_W,
        h: ACTIVITY_SEPARATOR_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX, 
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Launcher"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

LauncherActivity::LauncherActivity () :
    NCSActivity (&mymain_templ, false)
{ 
    m_style = STYLE_ZOOM;
    m_firstBoot = false;
}

