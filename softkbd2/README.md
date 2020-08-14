# SoftKBD

This demo creates a soft keyboard for touch panel.
You can use it to input English words (Predictive Text Input)
and or Chinese words (Rime).

Note the most source files are imported from mGi, which is
a legacy MiniGUI component.

## Features

1. Create and register an IME window.
1. Show or hide the IME window automatically.
1. Show tooltip window when the user pressed a key.
1. Switch different input methods.
1. It is customizable (follow https://github.com/rime/plum)

## Custom

1. This project use `librime` as backend,so everyone can config the schema and dictionary just like `ibus-rime`.

2. the config files of rime input engine is under `data/` directory.There are several schema:

   1. `luna_pinyin` (the preset schema)
   2. `luna_pinyin_simp`
   3. `bopomofo`
   4. `stroke`
   5. `cangjie5`

   You can delete useless file(schema and dictionary) and remove it from `default.yaml`

3. Also,you can write schema and config dictionary by yourself.

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

