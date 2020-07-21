# MiniGUI Demos

This is a set of comprehensive demonstrations of MiniGUI and
its key components, including mGEff, mGPlus, mGNCS, and mGNCS4Touch.

The demos are supplements to [mGUXDemo]. They illustrate the following
features which are not implemented in mGUXDemo:

1. The activity-driven framework derived from mGUXDemo (`common/`).
1. Vairous input methods:
   * Screen locker with digital password.
   * A soft keyboard to input Latin words, digits, and Chinese characters.
1. Using GNU gettext `.mo` files for localization.
1. Interacting with system services by using an asynchronous task thread,
for example:
   * Getting WiFi hotspots list and connecting to a specific WiFi hotspot.
   * Getting system events to updating the icons on status bar.
1. Message screen and toast window.

Currently, there are two demos included in this project.

## CBPlusUI

This demo is a simplified edition of a real product called `CBPlus`.
So it can be a very good start of one application which is based on MiniGUI.

Note that the system services are fake ones in the demo. You may need
to replace them with a real mechanism to interact with the underlying
services. In the real product, we use `ubus` of OpenWRT as the
mechanism to interact with the system services.

For more information, please visit:

https://github.com/VincentWei/mg-demos/tree/master/cbplusui

## SoftKBD

This demo creates a soft keyboard to input characters with touch panel.

For more information, please visit:

https://github.com/VincentWei/mg-demos/tree/master/softkbd

## Prerequisites

  * A C/C++ compiler compatible with C++ 11 (gcc 4.5+ or clang).
  * MiniGUI: v3.2.2 or later
  * MiniGUI Resource: v3.0.12 or later
  * mGPlus: v1.4.0 or later
  * mGEff: v1.2.0 or later
  * mGNCS: V1.2.2 or later
  * mGNCS4Touch: V1.2.2 or later
  * 3rd-party libraries:
    * chipmunk-5.3.1
    * librime-1.5.3(softkbd2)

## Building

If you are using MiniGUI 4.0, we strongly recommend you to build
the demos by using the fetching and building scripts of
`build-minigui-4.0`:

https://github.com/VincenWei/build-minigui-4.0

Or, if you are using MiniGUI 3.2, you can use the following commands
to configure, make, install, and run the demos, once you have installed
the dependent libraries listed above:

    $ ./autogen.sh; # if you got the source from GitHub repo
    $ ./configure;
    $ make

To run a demo:

    $ cd cbplusui
    $ ./cbplusui

## Configure Options

The configure script provides the following options:

    # Whether display tooltip window for soft keyboard:
    --enable-tooltip
    
    # with or without animation for soft keyboard:
    --enable-animate
    
    # specify the size of soft keyboard:
    --with-softkbdsize=480x272/320x240/240x320

## Directory

* `common/`: The implementation of the framework.
* `res/`: Common resource files used by all demos.
* `cbplusui/`: The demo of CBPlusUI.
* `softkbd/`: The demo of SoftKBD.
* `softkbd2/`: Another demo of SoftKBD that use librime as backend.

## Copying

Copyright (C) 2019 Beijing FMSoft Technologies Co., Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

### Special Statement

The above open source or free software license does
not apply to any entity in the Exception List published by
Beijing FMSoft Technologies Co., Ltd.

If you are or the entity you represent is listed in the Exception List,
the above open source or free software license does not apply to you
or the entity you represent. Regardless of the purpose, you should not
use the software in any way whatsoever, including but not limited to
downloading, viewing, copying, distributing, compiling, and running.
If you have already downloaded it, you MUST destroy all of its copies.

The Exception List is published by FMSoft and may be updated
from time to time. For more information, please see
<https://www.fmsoft.cn/exception-list>.

Also note that the copyright of the resource assets included in the
demos might be reserved by the copyright owner. For more information,
please see the README files in the sub directories.

[mGUXDemo]: https://github.com/VincentWei/cell-phone-ux-demo

