# CBPlusUI

This demo is a simplified edition of a real product called [CBPlus],
which is developed by [Joobot Technologies].

## Features

1. Use the framework derived from mGUXDemo (`common/`) to support
animated activities and list view.
1. Using GNU gettext for localization.
1. Different input methods with touch panel:
   * Screen locker with digital password.
   * A soft keyboard to input Latin letters, digits, and symbol characters.
1. Interacting with system services by using an asynchronous task thread,
for example:
   * Getting WiFi hotspots list and connecting to a specific WiFi hotspot.
   * Getting system events to updating the icons on status bar.
1. Message screen and toast window.
1. Others:
   * Using BITMAP font to show time.

Note that the system services are fake ones in the demo. You may need
to replace them with a real mechanism to interact with the underlying
services. In the real product, we use `ubus` of OpenWRT as the
mechanism to interact with the system services.

## Copying

    Copyright (C) 2019 Beijing Joobot Technologies Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

NOTE that [Joobot Technologies] reserves the copyright of
all images used by this demo. You can not use, copy, modify,
and/or distribute the images included in the directory
`res/images` for any purpose other than demonstration.

[CBPlus]: https://www.joobot.com/cn/joopic/cbplus/
[Joobot Technologies]: https://www.joobot.com
