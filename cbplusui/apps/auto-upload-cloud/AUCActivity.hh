/*!============================================================================
 * @file AUCActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 03/06/2018
 *
 * Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _AUC_ACTIVITY_HH_
#define _AUC_ACTIVITY_HH_

class AUCActivity : public NCSActivity {

protected:
    virtual int onCancel();
    virtual int onStart();
    virtual int onStop();

public:
    AUCActivity ();
    ~AUCActivity() {};
};

#endif /* _AUC_ACTIVITY_HH_ */
