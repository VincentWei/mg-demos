/*!============================================================================
 * @file MUCActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 03/06/2018
 *
 * Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _MUC_ACTIVITY_HH_
#define _MUC_ACTIVITY_HH_

class MUCActivity : public NCSActivity {

protected:
    virtual int onStart();
    virtual int onCancel();
    virtual int onStop();

public:
    int m_duration;
    int m_filter;
    bool m_started;
    bool m_stopped;

    MUCActivity ();
    ~MUCActivity() {};
};

#endif /* _MUC_ACTIVITY_HH_ */
