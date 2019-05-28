# MiniGUI Demos

This is a set of comprehensive demonstrations of MiniGUI and
its key components, including mGEff, mGPlus, mGNCS, and mGNCS4Touch.

The demos are supplements to [mGUXDemo]. They illustrate the following
features which are not implemented in mGUXDemo:

1. A smaller framework derived from mGUXDemo (`common/`).
1. Using GNU gettext for localization.
1. Different input methods with touch panel:
   * Screen locker with digital password.
   * A soft keyboard to input Latin letters, digits, and symbol characters.
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

## 

## Prerequisites

  * MiniGUI: v3.2.2 or later
  * MiniGUI Resource: v3.0.12 or later
  * mGPlus: v1.4.0 or later
  * mGEff: v1.2.0 or later
  * mGNCS: V1.2.2 or later
  * mGNCS4Touch: V1.2.2 or later
  * 3rd-party libraries:
    * chipmunk-5.3.1

## Building

We strongly recommend you to build this demo by using the fetching and building
scripts of build-minigui-4.0:

https://github.com/VincenWei/build-minigui-4.0

Otherwise, you can use the following commands to configure, make, install,
and run the demos, once you have installed the dependent libraries listed
above.

To build the demos:

    $ ./autogen.sh; # if you got the source from GitHub repo
    $ ./configure;
    $ make

To run a demo:

    $ cd cbplusui
    $ ./cbplusui

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

Note that the copyright of the resource assets included in the demos
might be reserved by the copyright owner. For more information,
please see the README files in the sub directories.

[mGUXDemo]: https://github.com/VincentWei/cell-phone-ux-demo

