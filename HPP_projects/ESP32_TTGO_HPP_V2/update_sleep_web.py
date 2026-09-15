import os

files = ['web_server.h', '../ESP32_T_Display_S3_HPP_V3/web_server.h']

for file_path in files:
    if not os.path.exists(file_path): continue
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Extern variable
    if 'extern uint32_t deepSleepTimeoutMs;' not in content:
        content = content.replace('extern uint32_t screenSwitchTime;', 'extern uint32_t screenSwitchTime;\nextern uint32_t deepSleepTimeoutMs;')

    # CONFIG_HTML updates
    screen_cfg_find = """        <!-- Screen Config Submenu -->
        <div id="screenConfig" class="submenu-view">
            <button class="back-btn" onclick="goBack()">&larr; Back to Settings</button>
            <div class="card">
                <div class="label">Screen Rotation (s)</div>
                <input type="number" id="st" min="1" max="60" style="margin-right: 10px;">
                <button class="btn" style="background: #7c3aed; width: auto;" onclick="set('scrTime', document.getElementById('st').value)">SET</button>
            </div>
        </div>"""
    
    screen_cfg_repl = """        <!-- Screen & Power Config Submenu -->
        <div id="screenConfig" class="submenu-view">
            <button class="back-btn" onclick="goBack()">&larr; Back to Settings</button>
            <div class="card">
                <div class="label">Screen Rotation (s)</div>
                <input type="number" id="st" min="1" max="60" style="margin-right: 10px;">
                <button class="btn" style="background: #7c3aed; width: auto;" onclick="set('scrTime', document.getElementById('st').value)">SET</button>
            </div>
            <div class="card">
                <div class="label">Auto-Sleep Timeout (mins)</div>
                <input type="number" id="slp" min="10" max="15" style="margin-right: 10px;">
                <button class="btn" style="background: #7c3aed; width: auto;" onclick="set('sleepTime', document.getElementById('slp').value)">SET</button>
                <div style="font-size: 0.75em; color: #94a3b8; margin-top: 5px;">Range: 10 to 15 minutes.</div>
            </div>
        </div>"""
    content = content.replace(screen_cfg_find, screen_cfg_repl)
    
    # Update Menu text
    content = content.replace("'Screen Config')\">\n                <span class=\"menu-icon\">🖥️</span> Screen Config", "'Screen Config')\">\n                <span class=\"menu-icon\">🖥️</span> Power & Screen")
    
    # Telemetry JS
    if "document.getElementById('slp').value = data.slp;" not in content:
        content = content.replace("document.getElementById('st').value = data.st;", "document.getElementById('st').value = data.st;\n                    document.getElementById('slp').value = data.slp;")
    
    # Telemetry C++
    if ',"slp":' not in content:
        content = content.replace('json += ",\\"st\\":" + String(screenSwitchTime / 1000);', 'json += ",\\"st\\":" + String(screenSwitchTime / 1000);\n    json += ",\\"slp\\":" + String(deepSleepTimeoutMs / 60000);')
        
    # Control C++
    if 'cmd == "sleepTime"' not in content:
        ctrl_logic = """    if (cmd == "sleepTime") {
        uint32_t mins = val.toInt();
        if (mins >= 10 && mins <= 15) {
            deepSleepTimeoutMs = mins * 60000;
            preferences.putUInt("sleepTime", deepSleepTimeoutMs);
            displayWebConfirmation("Sleep Time Saved");
        }
    }
"""
        content = content.replace('if (cmd == "scrTime") {', ctrl_logic + '    if (cmd == "scrTime") {')

    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f'Updated {file_path}')