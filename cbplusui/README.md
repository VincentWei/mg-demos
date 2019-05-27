# CBPlusUI

This demo is a simplified edition of a real product called `CBPlus`,
which is developed by [Joobot Technologies].

NOTE that [Joobot Technologies] reserves the copyright of
all images used by this demo. You can not use, copy, modify,
and/or distribute the images included in the directory
`res/images`.

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

[Joobot Technologies]: https://www.joobot.com
