/*!============================================================================
 * @file SettingFtpServerActivity.cc 
 * @Synopsis The Time activity of Setting app. 
 * @author Vincent Wei
 * @version 1.0
 * @date 12/06/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "SysConfService.hh"
#include "SettingFtpServerActivity.hh"
#include "MenuListPiece.hh"

REGISTER_ACTIVITY(SettingFtpServerActivity);

#define RES_IMG_OK  "images/soft-keyboard/done.png"

static RES_NODE _myres_list [] = {
    /* images on title bar */
    {RES_IMG_OK,            RES_TYPE_IMAGE, 16, 0},

    /* terminator */
    {NULL}
};

#define IDE_FTPSERVER_HOST          (IDE_MIN + 0)
#define IDE_FTPSERVER_PORT          (IDE_MIN + 1)
#define IDE_FTPSERVER_USER          (IDE_MIN + 2)
#define IDE_FTPSERVER_PASSWORD      (IDE_MIN + 3)

static MENU_ENTRY _menu_entries [] = {
    { IDE_FTPSERVER_HOST,       TYPE_MENU_ENTRY_USER, },
    { IDE_FTPSERVER_PORT,       TYPE_MENU_ENTRY_USER, },
    { IDE_FTPSERVER_USER,       TYPE_MENU_ENTRY_USER, },
    { IDE_FTPSERVER_PASSWORD,   TYPE_MENU_ENTRY_USER, },
};

#define MAX_FTP_NAME_LEN        255

static struct _LocalFtpInfo {
    char host[MAX_FTP_NAME_LEN + 1];
    char port[MAX_FTP_NAME_LEN + 1];
    char user[MAX_FTP_NAME_LEN + 1];
    char password[MAX_FTP_NAME_LEN + 1];
} local_ftp_info;

#define local_ftp_host local_ftp_info.host
#define local_ftp_port local_ftp_info.port
#define local_ftp_user local_ftp_info.user
#define local_ftp_password local_ftp_info.password

static void init_local_ftp_server_info(void)
{
    const FTP_SERVER_INFO* ftp_info = SYSCFGSERVICE->getFtpServerInfo();

    memset(local_ftp_host, 0, sizeof(local_ftp_host));
    memset(local_ftp_port, 0, sizeof(local_ftp_port));
    memset(local_ftp_user, 0, sizeof(local_ftp_user));
    memset(local_ftp_password, 0, sizeof(local_ftp_password));

    if (ftp_info->host) {
        strncpy(local_ftp_host, ftp_info->host, MAX_FTP_NAME_LEN);
        strncpy(local_ftp_port, ftp_info->port, MAX_FTP_NAME_LEN);
        strncpy(local_ftp_user, ftp_info->user, MAX_FTP_NAME_LEN);
        strncpy(local_ftp_password, ftp_info->password, MAX_FTP_NAME_LEN);
    }
}

static void init_menu_entries (void)
{
    _menu_entries [IDE_FTPSERVER_HOST - IDE_MIN].name = _("Host Name");
    if (local_ftp_host[0])
        _menu_entries [IDE_FTPSERVER_HOST - IDE_MIN].value = local_ftp_host;
    else
        _menu_entries [IDE_FTPSERVER_HOST - IDE_MIN].value = _("Unset");

    _menu_entries [IDE_FTPSERVER_PORT - IDE_MIN].name = _("Port");
    if (local_ftp_port[0])
        _menu_entries [IDE_FTPSERVER_PORT - IDE_MIN].value = local_ftp_port;
    else
        _menu_entries [IDE_FTPSERVER_PORT - IDE_MIN].value = _("Unset");

    _menu_entries [IDE_FTPSERVER_USER - IDE_MIN].name = _("User Name");
    if (local_ftp_user[0])
        _menu_entries [IDE_FTPSERVER_USER - IDE_MIN].value = local_ftp_user;
    else
        _menu_entries [IDE_FTPSERVER_USER - IDE_MIN].value = _("Unset");

    _menu_entries [IDE_FTPSERVER_PASSWORD - IDE_MIN].name = _("Password");
    if (local_ftp_password[0])
        _menu_entries [IDE_FTPSERVER_PASSWORD - IDE_MIN].value = local_ftp_password;
    else
        _menu_entries [IDE_FTPSERVER_PASSWORD - IDE_MIN].value = _("Unset");
}

static mMenuListPiece* _listpiece;

static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (id == IDOK) {
        if (local_ftp_host[0] == 0 || local_ftp_port[0] == 0
                || local_ftp_user[0] == 0 || local_ftp_password[0] == 0) {
            messageScreen (self->hwnd, _("Incomplete FTP settings"),
                    _("Please set a complete FTP server and account information."),
                    MB_OK);
        }
        else {
            Intent* my_intent = Intent::newIntent ();
            my_intent->putExtra ("host", std::string (local_ftp_host));
            my_intent->putExtra ("port", std::string (local_ftp_port));
            my_intent->putExtra ("user", std::string (local_ftp_user));
            my_intent->putExtra ("password", std::string (local_ftp_password));
            ACTIVITYSTACK->push("SettingCheckFtpActivity", my_intent);
        }
    }
    else if (id == IDC_CONTENT) {
        switch (nc + IDE_MIN) {
        case IDE_FTPSERVER_HOST: {
            statusBar_enableScreenLock(FALSE);
            softKeyboard (self->hwnd, local_ftp_host, 1, MAX_FTP_NAME_LEN);
            statusBar_enableScreenLock(TRUE);
            if (local_ftp_host[0]) {
                if (local_ftp_port[0] == 0) {

                    // set default value for other fields
                    strcpy(local_ftp_port, "21");

                    // use user id as the default ftp user name
                    const char* def_user = SYSCFGSERVICE->getBoundJoobotId();
                    if (def_user)
                        strncpy(local_ftp_user, def_user, MAX_FTP_NAME_LEN);

                    // use device name as the default ftp user password
                    const char* def_passwd = SYSCFGSERVICE->getDeviceName();
                    if (def_passwd)
                    strcpy(local_ftp_password, def_passwd);
                }
            }
            break;
        }

        case IDE_FTPSERVER_PORT:
            statusBar_enableScreenLock(FALSE);
            softKeyboard (self->hwnd, local_ftp_port, 1, MAX_FTP_NAME_LEN);
            statusBar_enableScreenLock(TRUE);
            break;

        case IDE_FTPSERVER_USER:
            statusBar_enableScreenLock(FALSE);
            softKeyboard (self->hwnd, local_ftp_user, 1, MAX_FTP_NAME_LEN);
            statusBar_enableScreenLock(TRUE);
            break;

        case IDE_FTPSERVER_PASSWORD:
            statusBar_enableScreenLock(FALSE);
            softKeyboard (self->hwnd, local_ftp_password, 3, MAX_FTP_NAME_LEN);
            statusBar_enableScreenLock(TRUE);
            break;

        default:
            return TRUE;
        }
    }

    init_menu_entries();
    _c(_listpiece)->reloadData (_listpiece);
    return TRUE;
}

static LRESULT mymain_onReady (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    if (wparam == ACTIVITY_POPPED) {
        if (lparam) {
            SYSCFGSERVICE->clearFtpServerInfo();

            init_menu_entries();
            _c(_listpiece)->reloadData (_listpiece);
        }
        else {
            // check ftp ok
            SYSCFGSERVICE->getFtpAccount(false);
            ACTIVITYSTACK->pop();
        }
    }

    return 0;
}

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
{
    NCS_WND_TEMPLATE ctrl_templ = {
        class_name: NCSCTRL_CONTAINERCTRL,
        id: IDC_CONTENT,
        x: ACTIVITY_CONTENT_X,
        y: ACTIVITY_CONTENT_Y,
        w: ACTIVITY_CONTENT_W,
        h: ACTIVITY_CONTENT_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    mWidget* ctrl_ctnr = ncsCreateWindowIndirect (&ctrl_templ, self->hwnd);
    if (ctrl_ctnr) {
        RECT rc = {0, 0, ACTIVITY_CONTENT_W, ACTIVITY_CONTENT_H};

        init_menu_entries();
        _listpiece = createMenuListView ((mContainerCtrl*)ctrl_ctnr, &rc, _menu_entries, TABLESIZE (_menu_entries));
    }

    return TRUE;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("SettingFtpServerActivity: mymain_onClose called.\n");
    unloadResByTag (_myres_list, 0);

    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    unsigned cnt;
    cnt = loadResByTag (_myres_list, 0);

    if (cnt < (TABLESIZE (_myres_list) - 1)) {
        _ERR_PRINTF ("SettingFtpServerActivity: failed to load resource.\n");
        return FALSE;
    }

    mWidget* ctrl_title = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    ncsSetElement (ctrl_title, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl_title->hwnd, FONT_TITLE);
    SetWindowText (ctrl_title->hwnd, _("FTP Server"));

    mWidget* ctrl_ok = (mWidget*)(_c(self)->getChild (self, IDOK));
    _c(ctrl_ok)->setProperty (ctrl_ok, NCSP_IMAGE_IMAGE,
            (DWORD)GetResource (Str2Key ("images/soft-keyboard/done.png")));

    init_local_ftp_server_info();
    return TRUE;
}

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_USER_APP_READY, reinterpret_cast<void*>(mymain_onReady)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {0, NULL}
};

static NCS_WND_TEMPLATE _ctrl_templ[] =
{
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_TITLE,
        x: ACTIVITY_TITLE_X + 40,
        y: ACTIVITY_TITLE_Y,
        w: ACTIVITY_TITLE_W - 40,
        h: ACTIVITY_TITLE_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_IMAGE,
        id: IDOK,
        x: ACTIVITY_TITLE_X,
        y: ACTIVITY_TITLE_Y,
        w: 40,
        h: ACTIVITY_TITLE_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_SEPARATOR,
        id: IDC_STATIC,
        x: ACTIVITY_SEPARATOR_X,
        y: ACTIVITY_SEPARATOR_Y,
        w: ACTIVITY_SEPARATOR_W,
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
    N_("FTP Server"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingFtpServerActivity::SettingFtpServerActivity() : NCSActivity (&mymain_templ)
{ 
    m_style = STYLE_PUSH;
}

SettingFtpServerActivity::~SettingFtpServerActivity()
{
}

