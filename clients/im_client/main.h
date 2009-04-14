#ifndef MAIN_H
#define MAIN_H

#include <libim/Manager.h>

#include <Application.h>
#include <Window.h>
#include <Messenger.h>
#include <Entry.h>
#include <TextControl.h>
#include <TextView.h>
#include <Rect.h>
#include <String.h>
#include <BeBuild.h>
#include <Window.h>
#include <Beep.h>

#include <libim/Constants.h>
#include <libim/Contact.h>
#include <libim/Helpers.h>

#include <iterator>
#include <map>

#include "ObjectList.h"

void
setAttributeIfNotPresent( entry_ref ref, const char * attr, const char * value);

#include "ChatApp.h"
#include "ChatWindow.h"
#include "InputFilter.h"
#include "ResizeView.h"
#include "RunView.h"
#include "Theme.h"

const uint32 kResizeMessage = 'irsz';
const uint32 M_LOOKUP_WEBSTER = 'rvlw';
const uint32 M_LOOKUP_GOOGLE = 'rvlg';
const uint32 M_LOOKUP_ACRONYM = 'rvla';
const uint32 M_CLEAR = 'rvcl';

const uint32 M_OFFVIEW_SELECTION = 'rvos';
const uint32 M_THEME_FOREGROUND_CHANGE = 'rvtf';
const uint32 M_THEME_BACKGROUND_CHANGE = 'rvtb';
const uint32 M_THEME_FONT_CHANGE = 'rvto';

#endif
