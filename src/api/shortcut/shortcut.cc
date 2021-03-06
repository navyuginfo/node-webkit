// Copyright (c) 2014 Intel Corp
// Copyright (c) 2014 The Chromium Authors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell co
// pies of the Software, and to permit persons to whom the Software is furnished
// to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IM
// PLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNES
// S FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WH
// ETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "content/nw/src/api/shortcut/shortcut.h"

#include <vector>

#include "base/compiler_specific.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "content/nw/src/api/dispatcher_host.h"
#include "content/nw/src/api/shortcut/shortcut_constants.h"

namespace nwapi {

ui::Accelerator Parse(const std::string& shortcut) {
  // Convert to lower case, see
  // https://github.com/rogerwang/node-webkit/pull/1735.
  std::string lower_shortcut = StringToLowerASCII(shortcut);

  std::vector<std::string> tokens;
  base::SplitString(lower_shortcut, '+', &tokens);
  if (tokens.size() == 0)
    return ui::Accelerator();

  int modifiers = ui::EF_NONE;
  ui::KeyboardCode key = ui::VKEY_UNKNOWN;
  for (size_t i = 0; i < tokens.size(); i++) {
    if (tokens[i] == kKeyCtrl) {
#if defined(OS_MACOSX)
      modifiers |= ui::EF_COMMAND_DOWN;
#else
      modifiers |= ui::EF_CONTROL_DOWN;
#endif
    } else if (tokens[i] == kKeyAlt) {
      modifiers |= ui::EF_ALT_DOWN;
    } else if (tokens[i] == kKeyShift) {
      modifiers |= ui::EF_SHIFT_DOWN;
    } else if (tokens[i].size() == 1 ||  // A-Z, 0-9.
               tokens[i] == kKeyComma ||
               tokens[i] == kKeyPeriod ||
               tokens[i] == kKeyUp ||
               tokens[i] == kKeyDown ||
               tokens[i] == kKeyLeft ||
               tokens[i] == kKeyRight ||
               tokens[i] == kKeyIns ||
               tokens[i] == kKeyDel ||
               tokens[i] == kKeyHome ||
               tokens[i] == kKeyEnd ||
               tokens[i] == kKeyPgUp ||
               tokens[i] == kKeyPgDwn ||
               tokens[i] == kKeyTab ||
               tokens[i] == kKeyMediaNextTrack ||
               tokens[i] == kKeyMediaPlayPause ||
               tokens[i] == kKeyMediaPrevTrack ||
               tokens[i] == kKeyMediaStop) {
      if (key != ui::VKEY_UNKNOWN) {
        // Multiple key assignments.
        key = ui::VKEY_UNKNOWN;
        break;
      }

      if (tokens[i] == kKeyComma) {
        key = ui::VKEY_OEM_COMMA;
      } else if (tokens[i] == kKeyPeriod) {
        key = ui::VKEY_OEM_PERIOD;
      } else if (tokens[i] == kKeyUp) {
        key = ui::VKEY_UP;
      } else if (tokens[i] == kKeyDown) {
        key = ui::VKEY_DOWN;
      } else if (tokens[i] == kKeyLeft) {
        key = ui::VKEY_LEFT;
      } else if (tokens[i] == kKeyRight) {
        key = ui::VKEY_RIGHT;
      } else if (tokens[i] == kKeyIns) {
        key = ui::VKEY_INSERT;
      } else if (tokens[i] == kKeyDel) {
        key = ui::VKEY_DELETE;
      } else if (tokens[i] == kKeyHome) {
        key = ui::VKEY_HOME;
      } else if (tokens[i] == kKeyEnd) {
        key = ui::VKEY_END;
      } else if (tokens[i] == kKeyPgUp) {
        key = ui::VKEY_PRIOR;
      } else if (tokens[i] == kKeyPgDwn) {
        key = ui::VKEY_NEXT;
      } else if (tokens[i] == kKeyTab) {
        key = ui::VKEY_TAB;
      } else if (tokens[i] == kKeyMediaNextTrack) {
        key = ui::VKEY_MEDIA_NEXT_TRACK;
      } else if (tokens[i] == kKeyMediaPlayPause) {
        key = ui::VKEY_MEDIA_PLAY_PAUSE;
      } else if (tokens[i] == kKeyMediaPrevTrack) {
        key = ui::VKEY_MEDIA_PREV_TRACK;
      } else if (tokens[i] == kKeyMediaStop) {
        key = ui::VKEY_MEDIA_STOP;
      } else if (tokens[i].size() == 1 &&
                 tokens[i][0] >= 'a' && tokens[i][0] <= 'z') {
        key = static_cast<ui::KeyboardCode>(ui::VKEY_A + (tokens[i][0] - 'a'));
      } else if (tokens[i].size() == 1 &&
                 tokens[i][0] >= '0' && tokens[i][0] <= '9') {
        key = static_cast<ui::KeyboardCode>(ui::VKEY_0 + (tokens[i][0] - '0'));
      } else {
        key = ui::VKEY_UNKNOWN;
        break;
      }
    }
  }

  return ui::Accelerator(key, modifiers);
}

Shortcut::Shortcut(int id,
                   const base::WeakPtr<DispatcherHost>& dispatcher_host,
                   const base::DictionaryValue& option)
    : Base(id, dispatcher_host, option) {
  std::string shortcut;
  option.GetString("key", &shortcut);
  accelerator_ = Parse(shortcut);
  if (accelerator_.key_code() == ui::VKEY_UNKNOWN)
    OnFailed("Can not parse shortcut: " + shortcut + ".");
}

Shortcut::~Shortcut() {
}

void Shortcut::OnActive() {
  base::ListValue args;
  dispatcher_host()->SendEvent(this, "active", args);
}

void Shortcut::OnFailed(const std::string failed_msg) {
  base::ListValue args;
  args.AppendString(failed_msg);
  dispatcher_host()->SendEvent(this, "failed", args);
}

void Shortcut::OnKeyPressed(const ui::Accelerator& accelerator) {
  if (accelerator != accelerator_) {
    // This should never occur, because if it does, GlobalShortcutListener
    // notifes us with wrong accelerator.
    NOTREACHED();
    return;
  }

  OnActive();
}

}  // namespace nwapi
