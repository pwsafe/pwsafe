#ifndef __PWS_TOOLBAR_BUTTONS_H
#define __PWS_TOOLBAR_BUTTONS_H

#include "./graphics/toolbar/new/new.xpm"
#include "./graphics/toolbar/new/new_disabled.xpm"
#include "./graphics/toolbar/new/open.xpm"
#include "./graphics/toolbar/new/open_disabled.xpm"
#include "./graphics/toolbar/new/close.xpm"
#include "./graphics/toolbar/new/close_disabled.xpm"
#include "./graphics/toolbar/new/save.xpm"
#include "./graphics/toolbar/new/save_disabled.xpm"
#include "./graphics/toolbar/new/copypassword.xpm"
#include "./graphics/toolbar/new/copypassword_disabled.xpm"
#include "./graphics/toolbar/new/copyuser.xpm"
#include "./graphics/toolbar/new/copyuser_disabled.xpm"
#include "./graphics/toolbar/new/copynotes.xpm"
#include "./graphics/toolbar/new/copynotes_disabled.xpm"
#include "./graphics/toolbar/new/clearclipboard.xpm"
#include "./graphics/toolbar/new/clearclipboard_disabled.xpm"
#include "./graphics/toolbar/new/autotype.xpm"
#include "./graphics/toolbar/new/autotype_disabled.xpm"
#include "./graphics/toolbar/new/browseurl.xpm"
#include "./graphics/toolbar/new/browseurl_disabled.xpm"
#include "./graphics/toolbar/new/sendemail.xpm"
#include "./graphics/toolbar/new/sendemail_disabled.xpm"
#include "./graphics/toolbar/new/add.xpm"
#include "./graphics/toolbar/new/add_disabled.xpm"
#include "./graphics/toolbar/new/viewedit.xpm"
#include "./graphics/toolbar/new/viewedit_disabled.xpm"
#include "./graphics/toolbar/new/delete.xpm"
#include "./graphics/toolbar/new/delete_disabled.xpm"
#include "./graphics/toolbar/new/expandall.xpm"
#include "./graphics/toolbar/new/expandall_disabled.xpm"
#include "./graphics/toolbar/new/collapseall.xpm"
#include "./graphics/toolbar/new/collapseall_disabled.xpm"
#include "./graphics/toolbar/new/options.xpm"
#include "./graphics/toolbar/new/options_disabled.xpm"
#include "./graphics/toolbar/new/help.xpm"
#include "./graphics/toolbar/new/help_disabled.xpm"

#include "./graphics/toolbar/classic/new.xpm"
#include "./graphics/toolbar/classic/open.xpm"
#include "./graphics/toolbar/classic/close.xpm"
#include "./graphics/toolbar/classic/save.xpm"
#include "./graphics/toolbar/classic/copypassword.xpm"
#include "./graphics/toolbar/classic/copyuser.xpm"
#include "./graphics/toolbar/classic/copynotes.xpm"
#include "./graphics/toolbar/classic/clearclipboard.xpm"
#include "./graphics/toolbar/classic/autotype.xpm"
#include "./graphics/toolbar/classic/browseurl.xpm"
#include "./graphics/toolbar/classic/sendemail.xpm"
#include "./graphics/toolbar/classic/add.xpm"
#include "./graphics/toolbar/classic/viewedit.xpm"
#include "./graphics/toolbar/classic/delete.xpm"
#include "./graphics/toolbar/classic/expandall.xpm"
#include "./graphics/toolbar/classic/collapseall.xpm"
#include "./graphics/toolbar/classic/options.xpm"
#include "./graphics/toolbar/classic/help.xpm"


enum {ID_SEPARATOR = -1};

#define PWS_TOOLBAR_BITMAPS(n) wxCONCAT(n, _xpm), wxCONCAT(wxCONCAT(n, _disabled), _xpm), wxCONCAT(classic_, n)
#define SEPARATOR {ID_SEPARATOR, NULL, NULL, NULL, NULL}

struct _PwsToolbarInfo{
  int id;
  const wxChar* tooltip;
  const char** bitmap_normal;
  const char** bitmap_disabled;
  const char** bitmap_classic;
  
} PwsToolbarButtons[] = 

{
          { wxID_NEW,         _("Make New Database"),               PWS_TOOLBAR_BITMAPS(new)            },
          { wxID_OPEN,        _("Open Another Database"),           PWS_TOOLBAR_BITMAPS(open)           },
          { wxID_CLOSE,       _("Close Database"),                  PWS_TOOLBAR_BITMAPS(close)          },
          { wxID_SAVE,        _("Save Database"),                   PWS_TOOLBAR_BITMAPS(save)           },
          SEPARATOR,
          { ID_COPYPASSWORD,   _("Copy Password to Clipboard"),     PWS_TOOLBAR_BITMAPS(copypassword)   },
          { ID_COPYUSERNAME,   _("Copy Username to Clipboard"),     PWS_TOOLBAR_BITMAPS(copyuser)       },
          { ID_COPYNOTESFLD,   _("Copy Notes to Clipboard"),        PWS_TOOLBAR_BITMAPS(copynotes)      },
          { ID_CLEARCLIPBOARD, _("Clear the clipboard contents"),   PWS_TOOLBAR_BITMAPS(clearclipboard) },
          SEPARATOR,
          { ID_AUTOTYPE,       _("Perform Autotype"),               PWS_TOOLBAR_BITMAPS(autotype)       },
          { ID_BROWSEURL,      _("Browse to URL"),                  PWS_TOOLBAR_BITMAPS(browseurl)      },
          { ID_SENDEMAIL,      _("Send Email"),                     PWS_TOOLBAR_BITMAPS(sendemail)      },
          SEPARATOR,
          { wxID_ADD,          _("Add New Entry"),                   PWS_TOOLBAR_BITMAPS(add)            },
          { ID_EDIT,           _("Edit an Entry"),                   PWS_TOOLBAR_BITMAPS(viewedit)       },
          SEPARATOR,
          { wxID_DELETE,       _("Delete an Entry"),                 PWS_TOOLBAR_BITMAPS(delete)         },
          SEPARATOR,
          { ID_EXPANDALL,      _("Expand All"),                      PWS_TOOLBAR_BITMAPS(expandall)      },
          { ID_COLLAPSEALL,    _("Collapse All"),                    PWS_TOOLBAR_BITMAPS(collapseall)    },
          SEPARATOR,
          { wxID_PREFERENCES,  _("Options"),                         PWS_TOOLBAR_BITMAPS(options)        },
          SEPARATOR,
          { wxID_HELP,         _("Help"),                            PWS_TOOLBAR_BITMAPS(help)           }
};

#undef SEPARATOR

#endif
