#ifndef __PWS_TOOLBAR_BUTTONS_H
#define __PWS_TOOLBAR_BUTTONS_H

#include "../graphics/toolbar/wxWidgets/new.xpm"
#include "../graphics/toolbar/wxWidgets/new_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/open.xpm"
#include "../graphics/toolbar/wxWidgets/open_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/close.xpm"
#include "../graphics/toolbar/wxWidgets/close_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/save.xpm"
#include "../graphics/toolbar/wxWidgets/save_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/copypassword.xpm"
#include "../graphics/toolbar/wxWidgets/copypassword_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/copyuser.xpm"
#include "../graphics/toolbar/wxWidgets/copyuser_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/copynotes.xpm"
#include "../graphics/toolbar/wxWidgets/copynotes_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/clearclipboard.xpm"
#include "../graphics/toolbar/wxWidgets/clearclipboard_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/autotype.xpm"
#include "../graphics/toolbar/wxWidgets/autotype_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/browseurl.xpm"
#include "../graphics/toolbar/wxWidgets/browseurl_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/sendemail.xpm"
#include "../graphics/toolbar/wxWidgets/sendemail_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/add.xpm"
#include "../graphics/toolbar/wxWidgets/add_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/viewedit.xpm"
#include "../graphics/toolbar/wxWidgets/viewedit_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/delete.xpm"
#include "../graphics/toolbar/wxWidgets/delete_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/expandall.xpm"
#include "../graphics/toolbar/wxWidgets/expandall_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/collapseall.xpm"
#include "../graphics/toolbar/wxWidgets/collapseall_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/options.xpm"
#include "../graphics/toolbar/wxWidgets/options_disabled.xpm"
#include "../graphics/toolbar/wxWidgets/help.xpm"
#include "../graphics/toolbar/wxWidgets/help_disabled.xpm"

#include "../graphics/toolbar/wxWidgets/classic/new.xpm"
#include "../graphics/toolbar/wxWidgets/classic/open.xpm"
#include "../graphics/toolbar/wxWidgets/classic/close.xpm"
#include "../graphics/toolbar/wxWidgets/classic/save.xpm"
#include "../graphics/toolbar/wxWidgets/classic/copypassword.xpm"
#include "../graphics/toolbar/wxWidgets/classic/copyuser.xpm"
#include "../graphics/toolbar/wxWidgets/classic/copynotes.xpm"
#include "../graphics/toolbar/wxWidgets/classic/clearclipboard.xpm"
#include "../graphics/toolbar/wxWidgets/classic/autotype.xpm"
#include "../graphics/toolbar/wxWidgets/classic/browseurl.xpm"
#include "../graphics/toolbar/wxWidgets/classic/sendemail.xpm"
#include "../graphics/toolbar/wxWidgets/classic/add.xpm"
#include "../graphics/toolbar/wxWidgets/classic/viewedit.xpm"
#include "../graphics/toolbar/wxWidgets/classic/delete.xpm"
#include "../graphics/toolbar/wxWidgets/classic/expandall.xpm"
#include "../graphics/toolbar/wxWidgets/classic/collapseall.xpm"
#include "../graphics/toolbar/wxWidgets/classic/options.xpm"
#include "../graphics/toolbar/wxWidgets/classic/help.xpm"


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
