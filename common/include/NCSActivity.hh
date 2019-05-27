/*!============================================================================
 * @file NCSActivity.hh 
 * @Synopsis  
 * @author DongKai
 * @version 1.0
 *  Company: Beijing Feynman Software Technology Co., Ltd.
 */

#ifndef  NCSACTIVITY_INC
#define  NCSACTIVITY_INC

/** 
 * @synopsis NCSActivity 
 */
class NCSActivity : public Activity 
{
public:
    virtual ~NCSActivity();

    bool isAutoEscape () {
        return m_autoEscape;
    }

    void setAutoEscape (bool autoEscape) {
        m_autoEscape = autoEscape;
    }

    void onDestroyOrigin(mWidget* widget, int msg) {
        if (m_onDestroyOrigin) {
            m_onDestroyOrigin(widget, msg);
        }
    }

    virtual int onResume();
    virtual int onPause();
    virtual int onCancel() {
        ActivityStack::singleton()->pop();
        return 0;
    }

protected:
    // tmpl or package just include content of body.
    NCSActivity(NCS_MNWND_TEMPLATE* _template, bool autoEscape = true);

    NCSActivity(HPACKAGE hPack, HICON hIcon, HMENU hMenu, bool autoEscape = true) : 
        m_tmpl(NULL), m_hPack(hPack), m_hIcon(hIcon), m_hMenu(hMenu), m_autoEscape(autoEscape)
    {
    }

    NCSActivity(const NCSActivity&);
    virtual HWND createHWND();

    NCS_MNWND_TEMPLATE* m_tmpl;
    HPACKAGE m_hPack;
    HICON m_hIcon;
    HMENU m_hMenu;
    void (*m_onDestroyOrigin)(mWidget*, int);

    bool m_autoEscape;
};

#endif   /* ----- #ifndef NCSACTIVITY_INC  ----- */

