/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtCore module of the Katie Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QCONFIG_H
#define QCONFIG_H

// Build specs
#define QT_VERSION_MAJOR ${KATIE_MAJOR}
#define QT_VERSION_MINOR ${KATIE_MINOR}
#define QT_VERSION_MICRO ${KATIE_MICRO}
#define QT_VERSION_STR "${KATIE_VERSION}"
#define QT_VERSION_HEX_STR "${KATIE_HEX}"
#define QT_VERSION ${KATIE_HEX}

#define QT_KATIE
#cmakedefine QT_POINTER_SIZE ${QT_POINTER_SIZE}

// The operating system, must be one of Q_OS_x
#define Q_OS_${KATIE_OS}

// The architechture, must be one of QT_ARCH_x
#define QT_ARCH_${KATIE_ARCH}

// Host endianness
#define Q_BIG_ENDIAN 4321
#define Q_LITTLE_ENDIAN 1234
#define Q_BYTE_ORDER ${KATIE_BYTE_ORDER}

// Not supported, used to bootstrap
#cmakedefine QT_NO_QOBJECT
#cmakedefine QT_NO_COMPRESS
#cmakedefine QT_NO_THREAD
#cmakedefine QT_NO_PROCESS
#cmakedefine QT_NO_DATASTREAM
#cmakedefine QT_NO_TEXTSTREAM
#cmakedefine QT_NO_REGEXP
#cmakedefine QT_NO_REGEXP_ANCHOR_ALT
#cmakedefine QT_NO_REGEXP_BACKREF
#cmakedefine QT_NO_REGEXP_CAPTURE
#cmakedefine QT_NO_REGEXP_CCLASS
#cmakedefine QT_NO_REGEXP_ESCAPE
#cmakedefine QT_NO_REGEXP_INTERVAL
#cmakedefine QT_NO_REGEXP_LOOKAHEAD
#cmakedefine QT_NO_REGEXP_OPTIM
#cmakedefine QT_NO_REGEXP_WILDCARD
#cmakedefine QT_NO_STANDARDPATHS
#cmakedefine QT_NO_SETTINGS
#cmakedefine QT_NO_TEXTCODEC

// Compiler and host dependant
#cmakedefine QT_NO_EXCEPTIONS
#cmakedefine QT_NO_FPU

// External packages dependant
#cmakedefine QT_NO_CUPS
#cmakedefine QT_NO_EXECINFO
#cmakedefine QT_NO_FONTCONFIG
#cmakedefine QT_NO_SESSIONMANAGER
#cmakedefine QT_NO_TRANSLATION
#cmakedefine QT_NO_XCURSOR
#cmakedefine QT_NO_XFIXES
#cmakedefine QT_NO_XINERAMA
#cmakedefine QT_NO_XRANDR
#cmakedefine QT_NO_XRENDER
#cmakedefine QT_NO_XSHAPE
#cmakedefine QT_NO_XSYNC

// Broken, do not set until further notice
// #cmakedefine QT_NO_ACTION
// #cmakedefine QT_NO_ANIMATION
// #cmakedefine QT_NO_BUTTONGROUP
// #cmakedefine QT_NO_CLIPBOARD
// #cmakedefine QT_NO_DRAGANDDROP
// #cmakedefine QT_NO_COLORDIALOG
// #cmakedefine QT_NO_COMBOBOX
// #cmakedefine QT_NO_COMPLETER
// #cmakedefine QT_NO_CONTEXTMENU
// #cmakedefine QT_NO_CSSPARSER
// #cmakedefine QT_NO_DATESTRING
// #cmakedefine QT_NO_DEBUG_STREAM
// #cmakedefine QT_NO_DIRMODEL
// #cmakedefine QT_NO_DOCKWIDGET
// #cmakedefine QT_NO_DOM
// #cmakedefine QT_NO_FILEICONPROVIDER
// #cmakedefine QT_NO_FILESYSTEMMODEL
// #cmakedefine QT_NO_FONTCOMBOBOX
// #cmakedefine QT_NO_GRAPHICSVIEW
// #cmakedefine QT_NO_GROUPBOX
// #cmakedefine QT_NO_ICON
// #cmakedefine QT_NO_INPUTDIALOG
// #cmakedefine QT_NO_ITEMVIEWS
// #cmakedefine QT_NO_LINEEDIT
// #cmakedefine QT_NO_LISTVIEW
// #cmakedefine QT_NO_LISTWIDGET
// #cmakedefine QT_NO_MAINWINDOW
// #cmakedefine QT_NO_MATRIX4X4
// #cmakedefine QT_NO_VECTOR3D
// #cmakedefine QT_NO_MENU
// #cmakedefine QT_NO_MENUBAR
// #cmakedefine QT_NO_MESSAGEBOX
// #cmakedefine QT_NO_PROGRESSBAR
// #cmakedefine QT_NO_PROPERTIES
// #cmakedefine QT_NO_PROXYMODEL
// #cmakedefine QT_NO_RESIZEHANDLER
// #cmakedefine QT_NO_RUBBERBAND
// #cmakedefine QT_NO_SCROLLAREA
// #cmakedefine QT_NO_SCROLLBAR
// #cmakedefine QT_NO_SHORTCUT
// #cmakedefine QT_NO_SLIDER
// #cmakedefine QT_NO_SORTFILTERPROXYMODEL
// #cmakedefine QT_NO_SPINBOX
// #cmakedefine QT_NO_SPLITTER
// #cmakedefine QT_NO_STACKEDWIDGET
// #cmakedefine QT_NO_STANDARDITEMMODEL
// #cmakedefine QT_NO_STATUSBAR
// #cmakedefine QT_NO_STATUSTIP
// #cmakedefine QT_NO_TABBAR
// #cmakedefine QT_NO_TABWIDGET
// #cmakedefine QT_NO_TABLEVIEW
// #cmakedefine QT_NO_TABLEWIDGET
// #cmakedefine QT_NO_TEXTCONTROL
// #cmakedefine QT_NO_TEXTEDIT
// #cmakedefine QT_NO_TEXTHTMLPARSER
// #cmakedefine QT_NO_TEXTDATE
// #cmakedefine QT_NO_TOOLBAR
// #cmakedefine QT_NO_TOOLBOX
// #cmakedefine QT_NO_TOOLBUTTON
// #cmakedefine QT_NO_TOOLTIP
// #cmakedefine QT_NO_TREEVIEW
// #cmakedefine QT_NO_TREEWIDGET
// #cmakedefine QT_NO_UNDOCOMMAND
// #cmakedefine QT_NO_UNDOSTACK
// #cmakedefine QT_NO_VALIDATOR
// #cmakedefine QT_NO_VECTOR4D
// #cmakedefine QT_NO_XMLSTREAM
// #cmakedefine QT_NO_XMLSTREAMREADER
// #cmakedefine QT_NO_XMLSTREAMWRITER

// Misc
#cmakedefine QT_NO_BACKINGSTORE
#cmakedefine QT_NO_CALENDARWIDGET
#cmakedefine QT_NO_COLORNAMES
#cmakedefine QT_NO_COLUMNVIEW
#cmakedefine QT_NO_CURSOR
#cmakedefine QT_NO_DATETIMEEDIT
#cmakedefine QT_NO_DEBUG
#cmakedefine QT_NO_DIAL
#cmakedefine QT_NO_DIALOGBUTTONBOX
#cmakedefine QT_NO_EFFECTS
#cmakedefine QT_NO_FILEDIALOG
#cmakedefine QT_NO_FILESYSTEMITERATOR
#cmakedefine QT_NO_FILESYSTEMWATCHER
#cmakedefine QT_NO_FONTDIALOG
#cmakedefine QT_NO_FORMLAYOUT
#cmakedefine QT_NO_FRAME
#cmakedefine QT_NO_FSCOMPLETER
#cmakedefine QT_NO_IDENTITYPROXYMODEL
#cmakedefine QT_NO_IMAGEFORMAT_KAT
#cmakedefine QT_NO_IMAGEFORMAT_PPM
#cmakedefine QT_NO_IMAGEFORMAT_XPM
#cmakedefine QT_NO_IMAGE_HEURISTIC_MASK
#cmakedefine QT_NO_IPV6
#cmakedefine QT_NO_IPV6IFNAME
#cmakedefine QT_NO_LCDNUMBER
#cmakedefine QT_NO_LIBRARY
#cmakedefine QT_NO_LOCALSERVER
#cmakedefine QT_NO_LOCALSOCKET
#cmakedefine QT_NO_MOVIE
#cmakedefine QT_NO_NETWORKINTERFACE
#cmakedefine QT_NO_PLUGIN_CHECK
#cmakedefine QT_NO_PRINTDIALOG
#cmakedefine QT_NO_PRINTER
#cmakedefine QT_NO_PRINTPREVIEWDIALOG
#cmakedefine QT_NO_PRINTPREVIEWWIDGET
#cmakedefine QT_NO_PROGRESSDIALOG
#cmakedefine QT_NO_SIGNALMAPPER
#cmakedefine QT_NO_SIZEGRIP
#cmakedefine QT_NO_SPLASHSCREEN
#cmakedefine QT_NO_STRINGLISTMODEL
#cmakedefine QT_NO_STYLE_CLEANLOOKS
#cmakedefine QT_NO_STYLE_PROXY
#cmakedefine QT_NO_STYLE_STYLESHEET
#cmakedefine QT_NO_STYLE_WINDOWS
#cmakedefine QT_NO_SYNTAXHIGHLIGHTER
#cmakedefine QT_NO_TEMPORARYFILE
#cmakedefine QT_NO_TEXTBROWSER
#cmakedefine QT_NO_UDPSOCKET
#cmakedefine QT_NO_UNDOGROUP
#cmakedefine QT_NO_UNDOVIEW
#cmakedefine QT_NO_USING_NAMESPACE
#cmakedefine QT_NO_VECTOR2D
#cmakedefine QT_NO_WHATSTHIS
#cmakedefine QT_NO_WHEELEVENT
#cmakedefine QT_NO_WIZARD

// Debugging
#if defined(QT_NO_TEXTSTREAM) && !defined(QT_NO_DEBUG_STREAM)
#  define QT_NO_DEBUG_STREAM
#endif

// Animation
#if !defined(QT_NO_ANIMATION) && defined(QT_NO_PROPERTIES)
#  define QT_NO_ANIMATION
#endif

// QButtonGroup
#if !defined(QT_NO_BUTTONGROUP) && defined(QT_NO_GROUPBOX)
#  define QT_NO_BUTTONGROUP
#endif

// QDate/QTime/QDateTime
#if !defined(QT_NO_DATESTRING) && defined(QT_NO_TEXTDATE)
#  define QT_NO_DATESTRING
#endif

// QDial
#if !defined(QT_NO_DIAL) && defined(QT_NO_SLIDER)
#  define QT_NO_DIAL
#endif

// QMenu
#if !defined(QT_NO_MENU) && defined(QT_NO_ACTION)
#  define QT_NO_MENU
#endif

// QProgressDialog
#if !defined(QT_NO_PROGRESSDIALOG) && defined(QT_NO_PROGRESSBAR)
#  define QT_NO_PROGRESSDIALOG
#endif

// QScrollBar
#if !defined(QT_NO_SCROLLBAR) && defined(QT_NO_SLIDER)
#  define QT_NO_SCROLLBAR
#endif

// QSplitter
#if !defined(QT_NO_SPLITTER) && defined(QT_NO_RUBBERBAND)
#  define QT_NO_SPLITTER
#endif

// QToolButton
#if !defined(QT_NO_TOOLBUTTON) && defined(QT_NO_ACTION)
#  define QT_NO_TOOLBUTTON
#endif

// QUndoStack
#if !defined(QT_NO_UNDOSTACK) && defined(QT_NO_UNDOCOMMAND)
#  define QT_NO_UNDOSTACK
#endif

// QWizard
#if !defined(QT_NO_WIZARD) && defined(QT_NO_PROPERTIES)
#  define QT_NO_WIZARD
#endif

// QXmlStreamReader
#if !defined(QT_NO_XMLSTREAMREADER) && defined(QT_NO_XMLSTREAM)
#  define QT_NO_XMLSTREAMREADER
#endif

// QXmlStreamWriter
#if !defined(QT_NO_XMLSTREAMWRITER) && defined(QT_NO_XMLSTREAM)
#  define QT_NO_XMLSTREAMWRITER
#endif

// Context menu
#if !defined(QT_NO_CONTEXTMENU) && defined(QT_NO_MENU)
#  define QT_NO_CONTEXTMENU
#endif

// QScrollArea
#if !defined(QT_NO_SCROLLAREA) && defined(QT_NO_SCROLLBAR)
#  define QT_NO_SCROLLAREA
#endif

// QTabBar
#if !defined(QT_NO_TABBAR) && defined(QT_NO_TOOLBUTTON)
#  define QT_NO_TABBAR
#endif

// QUndoGroup
#if !defined(QT_NO_UNDOGROUP) && defined(QT_NO_UNDOSTACK)
#  define QT_NO_UNDOGROUP
#endif

// QWhatsThis
#if !defined(QT_NO_WHATSTHIS) && defined(QT_NO_TOOLBUTTON)
#  define QT_NO_WHATSTHIS
#endif

// Drag and drop
#if !defined(QT_NO_DRAGANDDROP) && (defined(QT_NO_CLIPBOARD) || defined(QT_NO_IMAGEFORMAT_PPM))
#  define QT_NO_DRAGANDDROP
#endif

// QGraphicsView
#if !defined(QT_NO_GRAPHICSVIEW) && (defined(QT_NO_SCROLLAREA) || defined(QT_NO_MATRIX4X4) || defined(QT_NO_VECTOR3D))
#  define QT_NO_GRAPHICSVIEW
#endif

// QPrinter
#if !defined(QT_NO_PRINTER) && defined(QT_NO_TEMPORARYFILE)
#  define QT_NO_PRINTER
#endif

// QSpinBox
#if !defined(QT_NO_SPINBOX) && (defined(QT_NO_SPINWIDGET) || defined(QT_NO_LINEEDIT) || defined(QT_NO_VALIDATOR))
#  define QT_NO_SPINBOX
#endif

// QCleanLooksStyle
#if !defined(QT_NO_STYLE_CLEANLOOKS) && defined(QT_NO_STYLE_WINDOWS)
#  define QT_NO_STYLE_CLEANLOOKS
#endif

// QStyleSheetStyle
#if !defined(QT_NO_STYLE_STYLESHEET) && (defined(QT_NO_STYLE_WINDOWS) || defined(QT_NO_PROPERTIES) || defined(QT_NO_CSSPARSER))
#  define QT_NO_STYLE_STYLESHEET
#endif

// QColorDialog
#if !defined(QT_NO_COLORDIALOG) && defined(QT_NO_SPINBOX)
#  define QT_NO_COLORDIALOG
#endif

// The Model/View Framework
#if !defined(QT_NO_ITEMVIEWS) && (defined(QT_NO_RUBBERBAND) || defined(QT_NO_SCROLLAREA))
#  define QT_NO_ITEMVIEWS
#endif

// QMenuBar
#if !defined(QT_NO_MENUBAR) && (defined(QT_NO_MENU) || defined(QT_NO_TOOLBUTTON))
#  define QT_NO_MENUBAR
#endif

// QTabWidget
#if !defined(QT_NO_TABWIDGET) && (defined(QT_NO_TABBAR) || defined(QT_NO_STACKEDWIDGET))
#  define QT_NO_TABWIDGET
#endif

// QTextEdit
#if !defined(QT_NO_TEXTEDIT) && (defined(QT_NO_SCROLLAREA) || defined(QT_NO_PROPERTIES))
#  define QT_NO_TEXTEDIT
#endif

// QListView
#if !defined(QT_NO_LISTVIEW) && defined(QT_NO_ITEMVIEWS)
#  define QT_NO_LISTVIEW
#endif

// QMainWindow
#if !defined(QT_NO_MAINWINDOW) && (defined(QT_NO_MENU) || defined(QT_NO_RESIZEHANDLER) || defined(QT_NO_TOOLBUTTON))
#  define QT_NO_MAINWINDOW
#endif

// QAbstractProxyModel
#if !defined(QT_NO_PROXYMODEL) && defined(QT_NO_ITEMVIEWS)
#  define QT_NO_PROXYMODEL
#endif

// QIdentityProxyModel
#if !defined(QT_NO_IDENTITYPROXYMODEL) && defined(QT_NO_PROXYMODEL)
#  define QT_NO_IDENTITYPROXYMODEL
#endif

// QStandardItemModel
#if !defined(QT_NO_STANDARDITEMMODEL) && defined(QT_NO_ITEMVIEWS)
#  define QT_NO_STANDARDITEMMODEL
#endif

// QStringListModel
#if !defined(QT_NO_STRINGLISTMODEL) && defined(QT_NO_ITEMVIEWS)
#  define QT_NO_STRINGLISTMODEL
#endif

// QSyntaxHighlighter
#if !defined(QT_NO_SYNTAXHIGHLIGHTER) && defined(QT_NO_TEXTEDIT)
#  define QT_NO_SYNTAXHIGHLIGHTER
#endif

// QTableView
#if !defined(QT_NO_TABLEVIEW) && defined(QT_NO_ITEMVIEWS)
#  define QT_NO_TABLEVIEW
#endif

// QTextBrowser
#if !defined(QT_NO_TEXTBROWSER) && defined(QT_NO_TEXTEDIT)
#  define QT_NO_TEXTBROWSER
#endif

// QToolBox
#if !defined(QT_NO_TOOLBOX) && (defined(QT_NO_TOOLBUTTON) || defined(QT_NO_SCROLLAREA))
#  define QT_NO_TOOLBOX
#endif

// QTreeView
#if !defined(QT_NO_TREEVIEW) && defined(QT_NO_ITEMVIEWS)
#  define QT_NO_TREEVIEW
#endif

// QColumnView
#if !defined(QT_NO_COLUMNVIEW) && defined(QT_NO_LISTVIEW)
#  define QT_NO_COLUMNVIEW
#endif

// QCompleter
#if !defined(QT_NO_COMPLETER) && defined(QT_NO_PROXYMODEL)
#  define QT_NO_COMPLETER
#endif

// QListWidget
#if !defined(QT_NO_LISTWIDGET) && defined(QT_NO_LISTVIEW)
#  define QT_NO_LISTWIDGET
#endif

// QSortFilterProxyModel
#if !defined(QT_NO_SORTFILTERPROXYMODEL) && defined(QT_NO_PROXYMODEL)
#  define QT_NO_SORTFILTERPROXYMODEL
#endif

// QTableWidget
#if !defined(QT_NO_TABLEWIDGET) && defined(QT_NO_TABLEVIEW)
#  define QT_NO_TABLEWIDGET
#endif

// QToolBar
#if !defined(QT_NO_TOOLBAR) && defined(QT_NO_MAINWINDOW)
#  define QT_NO_TOOLBAR
#endif

// QTreeWidget
#if !defined(QT_NO_TREEWIDGET) && defined(QT_NO_TREEVIEW)
#  define QT_NO_TREEWIDGET
#endif

// QDirModel
#if !defined(QT_NO_DIRMODEL) && (defined(QT_NO_ITEMVIEWS) || defined(QT_NO_FILESYSTEMMODEL))
#  define QT_NO_DIRMODEL
#endif

// QDockwidget
#if !defined(QT_NO_DOCKWIDGET) && (defined(QT_NO_RUBBERBAND) || defined(QT_NO_MAINWINDOW))
#  define QT_NO_DOCKWIDGET
#endif

// QUndoView
#if !defined(QT_NO_UNDOVIEW) && (defined(QT_NO_UNDOSTACK) || defined(QT_NO_LISTVIEW))
#  define QT_NO_UNDOVIEW
#endif

// QCompleter
#if !defined(QT_NO_FSCOMPLETER) && (defined(QT_NO_FILESYSTEMMODEL) || defined(QT_NO_COMPLETER))
#  define QT_NO_FSCOMPLETER
#endif

// QComboBox
#if !defined(QT_NO_COMBOBOX) && (defined(QT_NO_LINEEDIT) || defined(QT_NO_STANDARDITEMMODEL) || defined(QT_NO_LISTVIEW))
#  define QT_NO_COMBOBOX
#endif

// QPrintPreviewWidget
#if !defined(QT_NO_PRINTPREVIEWWIDGET) && (defined(QT_NO_GRAPHICSVIEW) || defined(QT_NO_PRINTER) || defined(QT_NO_MAINWINDOW))
#  define QT_NO_PRINTPREVIEWWIDGET
#endif

// QCalendarWidget
#if !defined(QT_NO_CALENDARWIDGET) && (defined(QT_NO_TABLEVIEW) || defined(QT_NO_MENU) || defined(QT_NO_TEXTDATE) || defined(QT_NO_SPINBOX) || defined(QT_NO_TOOLBUTTON))
#  define QT_NO_CALENDARWIDGET
#endif

// QDateTimeEdit
#if !defined(QT_NO_DATETIMEEDIT) && (defined(QT_NO_CALENDARWIDGET) || defined(QT_NO_DATESTRING))
#  define QT_NO_DATETIMEEDIT
#endif

// QInputDialog
#if !defined(QT_NO_INPUTDIALOG) && (defined(QT_NO_COMBOBOX) || defined(QT_NO_SPINBOX) || defined(QT_NO_STACKEDWIDGET))
#  define QT_NO_INPUTDIALOG
#endif

// QFontComboBox
#if !defined(QT_NO_FONTCOMBOBOX) && (defined(QT_NO_COMBOBOX) || defined(QT_NO_STRINGLISTMODEL))
#  define QT_NO_FONTCOMBOBOX
#endif

// QFontDialog
#if !defined(QT_NO_FONTDIALOG) && (defined(QT_NO_STRINGLISTMODEL) || defined(QT_NO_COMBOBOX) || defined(QT_NO_VALIDATOR) || defined(QT_NO_GROUPBOX))
#  define QT_NO_FONTDIALOG
#endif

// QPrintDialog
#if !defined(QT_NO_PRINTDIALOG) && (defined(QT_NO_PRINTER) || defined(QT_NO_COMBOBOX) || defined(QT_NO_BUTTONGROUP) || defined(QT_NO_SPINBOX) || defined(QT_NO_TREEVIEW) || defined(QT_NO_TABWIDGET))
#  define QT_NO_PRINTDIALOG
#endif

// QFileDialog
#if !defined(QT_NO_FILEDIALOG) && (defined(QT_NO_DIRMODEL) || defined(QT_NO_TREEVIEW) || defined(QT_NO_COMBOBOX) || defined(QT_NO_TOOLBUTTON) || defined(QT_NO_BUTTONGROUP) || defined(QT_NO_TOOLTIP) || defined(QT_NO_SPLITTER) || defined(QT_NO_STACKEDWIDGET) || defined(QT_NO_PROXYMODEL))
#  define QT_NO_FILEDIALOG
#endif

// QPrintPreviewDialog
#if !defined(QT_NO_PRINTPREVIEWDIALOG) && (defined(QT_NO_PRINTPREVIEWWIDGET) || defined(QT_NO_PRINTDIALOG) || defined(QT_NO_TOOLBAR))
#  define QT_NO_PRINTPREVIEWDIALOG
#endif

// QLocalServer
#if !defined(QT_NO_LOCALSERVER) && defined(QT_NO_LOCALSOCKET)
#  define QT_NO_LOCALSERVER
#endif

// Always off when building Katie, may be defined by application
// QT_NO_DYNAMIC_CAST
// QT_NO_URL_CAST_FROM_STRING
// QT_NO_CAST_FROM_ASCII
// QT_NO_CAST_FROM_BYTEARRAY
// QT_NO_CAST_TO_ASCII
// QT_NO_KEYWORDS
// QT_NO_TEMPLATE_TEMPLATE_PARAMETERS
// QT_NO_QOBJECT_CHECK

#endif
