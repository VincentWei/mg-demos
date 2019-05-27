/*!============================================================================
 * @file LauncherActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 03/06/2018
 *
 * Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _LAUNCHER_ACTIVITY_HH_
#define _LAUNCHER_ACTIVITY_HH_

class LauncherActivity : public NCSActivity {

public:
    bool m_firstBoot;

    LauncherActivity ();
    ~LauncherActivity() {};
};

#endif /* _LAUNCHER_ACTIVITY_HH_ */
