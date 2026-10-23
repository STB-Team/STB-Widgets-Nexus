"""Structural guardrails for paths not executable without the Skyrim runtime."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
src = root / 'src'
read = lambda name: (src / name).read_text(encoding='utf-8-sig')
for name in ['main.cpp', 'MenuHandler.cpp', 'InputHandler.cpp', 'ImGuiMenu.cpp']:
    text = read(name)
    assert not re.search(r'->(?:Invoke|SetVariable|SetVisible)\(', text), name
assert 'AddTask(' not in read('MenuHandler.cpp')
assert 'PollHudConfig' not in read('MenuHandler.cpp')
hooks = read('Hooks.cpp')
producers = hooks[:hooks.index('class PlayerUpdate')]
assert 'GetMenu' not in producers and 'GFx' not in producers
assert not re.search(r'->(?:Invoke|SetVariable|SetVisible)\(', hooks)
assert 'Settings::Update' not in hooks
manager = read('WidgetManager.cpp')
request = manager[manager.index('void CheckInI()'):manager.index('bool STB::ApplyWidgetConfiguration')]
assert 'GetMenu' not in request and 'GFx' not in request
assert manager.count('target == Widget::') == 8
for name in ['WidgetEquip', 'WidgetGameTime', 'WidgetGold', 'WidgetLvlWidget',
             'WidgetPlayTime', 'WidgetResist', 'WidgetShout', 'WidgetWeight']:
    text = read(name + '.cpp')
    advance = text[text.index('::AdvanceMovie('):text.index('::toggle_visibility(')]
    assert 'STB::AdvanceWidget(' in advance, name
    assert advance.index('STB::AdvanceWidget(') < advance.index('update();'), name
    assert 'IMenu::AdvanceMovie(' not in advance, name
    assert 'Settings::Update' not in advance, name
    assert 'STB::WidgetFrameState frameState_' in read(name + '.h'), name
print('PASS: UI ownership guardrails for messaging, menu, input, settings, hooks and all eight widget consumers')
