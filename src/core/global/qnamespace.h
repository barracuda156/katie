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

#ifndef QNAMESPACE_H
#define QNAMESPACE_H

#include <QtCore/qglobal.h>


QT_BEGIN_NAMESPACE


#ifndef Q_MOC_RUN
namespace
#else
class Q_CORE_EXPORT
#endif
Qt {

#if defined(Q_MOC_RUN)
    Q_OBJECT

    // NOTE: Generally, do not add Q_ENUMS if a corresponding Q_FLAGS exists.
    Q_ENUMS(ScrollBarPolicy FocusPolicy ContextMenuPolicy)
    Q_ENUMS(ArrowType ToolButtonStyle PenStyle PenCapStyle PenJoinStyle BrushStyle)
    Q_ENUMS(FillRule MaskMode BGMode ClipOperation SizeMode)
    Q_ENUMS(Axis Corner LayoutDirection SizeHint Orientation DropAction)
    Q_FLAGS(Alignment Orientations DropActions)
    Q_FLAGS(DockWidgetAreas ToolBarAreas)
    Q_ENUMS(DockWidgetArea ToolBarArea)
    Q_ENUMS(TextFormat)
    Q_ENUMS(TextElideMode)
    Q_ENUMS(DateFormat TimeSpec DayOfWeek)
    Q_ENUMS(CursorShape GlobalColor)
    Q_ENUMS(AspectRatioMode TransformationMode)
    Q_FLAGS(ImageConversionFlags)
    Q_ENUMS(Key ShortcutContext)
    Q_ENUMS(TextInteractionFlag)
    Q_FLAGS(TextInteractionFlags)
    Q_ENUMS(ItemSelectionMode)
    Q_FLAGS(ItemFlags)
    Q_ENUMS(CheckState)
    Q_ENUMS(SortOrder CaseSensitivity)
    Q_FLAGS(MatchFlags)
    Q_FLAGS(KeyboardModifiers MouseButtons)
    Q_ENUMS(WindowType WindowState WindowModality WidgetAttribute ApplicationAttribute)
    Q_FLAGS(WindowFlags WindowStates)
    Q_ENUMS(ConnectionType)
    Q_ENUMS(CursorMoveStyle)

public:
#endif // defined(Q_MOC_RUN)

    enum GlobalColor {
        color0,
        color1,
        black,
        white,
        darkGray,
        gray,
        lightGray,
        red,
        green,
        blue,
        cyan,
        magenta,
        yellow,
        darkRed,
        darkGreen,
        darkBlue,
        darkCyan,
        darkMagenta,
        darkYellow,
        transparent
    };

    enum Key {
        // misc
        Key_Backspace                     = 0xff08,
        Key_Tab                           = 0xff09,
        Key_Linefeed                      = 0xff0a,
        Key_Clear                         = 0xff0b,
        Key_Return                        = 0xff0d,
        Key_Pause                         = 0xff13,
        Key_Scroll_Lock                   = 0xff14,
        Key_Sys_Req                       = 0xff15,
        Key_Escape                        = 0xff1b,
        Key_Delete                        = 0xffff,
        Key_Multi_key                     = 0xff20,
        Key_Codeinput                     = 0xff37,
        Key_SingleCandidate               = 0xff3c,
        Key_MultipleCandidate             = 0xff3d,
        Key_PreviousCandidate             = 0xff3e,
        Key_Kanji                         = 0xff21,
        Key_Muhenkan                      = 0xff22,
        Key_Henkan                        = 0xff23,
        Key_Romaji                        = 0xff24,
        Key_Hiragana                      = 0xff25,
        Key_Katakana                      = 0xff26,
        Key_Hiragana_Katakana             = 0xff27,
        Key_Zenkaku                       = 0xff28 ,
        Key_Hankaku                       = 0xff29,
        Key_Zenkaku_Hankaku               = 0xff2a,
        Key_Touroku                       = 0xff2b,
        Key_Massyo                        = 0xff2c,
        Key_Kana_Lock                     = 0xff2d,
        Key_Kana_Shift                    = 0xff2e,
        Key_Eisu_Shift                    = 0xff2f,
        Key_Eisu_toggle                   = 0xff30,
        Key_Home                          = 0xff50,
        Key_Left                          = 0xff51,
        Key_Up                            = 0xff52,
        Key_Right                         = 0xff53,
        Key_Down                          = 0xff54,
        Key_Page_Up                       = 0xff55,
        Key_Page_Down                     = 0xff56,
        Key_End                           = 0xff57,
        Key_Begin                         = 0xff58,
        Key_Select                        = 0xff60,
        Key_Print                         = 0xff61,
        Key_Execute                       = 0xff62,
        Key_Insert                        = 0xff63,
        Key_Undo                          = 0xff65,
        Key_Redo                          = 0xff66,
        Key_Menu                          = 0xff67,
        Key_Find                          = 0xff68,
        Key_Cancel                        = 0xff69,
        Key_Help                          = 0xff6a,
        Key_Break                         = 0xff6b,
        Key_Mode_switch                   = 0xff7e,
        Key_Num_Lock                      = 0xff7f,
        Key_F1                            = 0xffbe,
        Key_F2                            = 0xffbf,
        Key_F3                            = 0xffc0,
        Key_F4                            = 0xffc1,
        Key_F5                            = 0xffc2,
        Key_F6                            = 0xffc3,
        Key_F7                            = 0xffc4,
        Key_F8                            = 0xffc5,
        Key_F9                            = 0xffc6,
        Key_F10                           = 0xffc7,
        Key_F11                           = 0xffc8,
        Key_F12                           = 0xffc9,
        Key_F13                           = 0xffca,
        Key_F14                           = 0xffcb,
        Key_F15                           = 0xffcc,
        Key_F16                           = 0xffcd,
        Key_F17                           = 0xffce,
        Key_F18                           = 0xffcf,
        Key_F19                           = 0xffd0,
        Key_F20                           = 0xffd1,
        Key_F21                           = 0xffd2,
        Key_F22                           = 0xffd3,
        Key_F23                           = 0xffd4,
        Key_F24                           = 0xffd5,
        Key_F25                           = 0xffd6,
        Key_F26                           = 0xffd7,
        Key_F27                           = 0xffd8,
        Key_F28                           = 0xffd9,
        Key_F29                           = 0xffda,
        Key_F30                           = 0xffdb,
        Key_F31                           = 0xffdc,
        Key_F32                           = 0xffdd,
        Key_F33                           = 0xffde,
        Key_F34                           = 0xffdf,
        Key_F35                           = 0xffe0,
        Key_Shift_L                       = 0xffe1,
        Key_Shift_R                       = 0xffe2,
        Key_Control_L                     = 0xffe3,
        Key_Control_R                     = 0xffe4,
        Key_Caps_Lock                     = 0xffe5,
        Key_Shift_Lock                    = 0xffe6,
        Key_Meta_L                        = 0xffe7,
        Key_Meta_R                        = 0xffe8,
        Key_Alt_L                         = 0xffe9,
        Key_Alt_R                         = 0xffea,
        Key_Super_L                       = 0xffeb,
        Key_Super_R                       = 0xffec,
        Key_Hyper_L                       = 0xffed,
        Key_Hyper_R                       = 0xffee,

        // latin
        Key_Space                         = 0x0020,
        Key_Exclam                        = 0x0021,
        Key_QuoteDbl                      = 0x0022,
        Key_NumberSign                    = 0x0023,
        Key_Dollar                        = 0x0024,
        Key_Percent                       = 0x0025,
        Key_Ampersand                     = 0x0026,
        Key_Apostrophe                    = 0x0027,
        Key_ParenLeft                     = 0x0028,
        Key_ParenRight                    = 0x0029,
        Key_Asterisk                      = 0x002a,
        Key_Plus                          = 0x002b,
        Key_Comma                         = 0x002c,
        Key_Minus                         = 0x002d,
        Key_Period                        = 0x002e,
        Key_Slash                         = 0x002f,
        Key_0                             = 0x0030,
        Key_1                             = 0x0031,
        Key_2                             = 0x0032,
        Key_3                             = 0x0033,
        Key_4                             = 0x0034,
        Key_5                             = 0x0035,
        Key_6                             = 0x0036,
        Key_7                             = 0x0037,
        Key_8                             = 0x0038,
        Key_9                             = 0x0039,
        Key_Colon                         = 0x003a,
        Key_Semicolon                     = 0x003b,
        Key_Less                          = 0x003c,
        Key_Equal                         = 0x003d,
        Key_Greater                       = 0x003e,
        Key_Question                      = 0x003f,
        Key_At                            = 0x0040,
        Key_A                             = 0x0041,
        Key_B                             = 0x0042,
        Key_C                             = 0x0043,
        Key_D                             = 0x0044,
        Key_E                             = 0x0045,
        Key_F                             = 0x0046,
        Key_G                             = 0x0047,
        Key_H                             = 0x0048,
        Key_I                             = 0x0049,
        Key_J                             = 0x004a,
        Key_K                             = 0x004b,
        Key_L                             = 0x004c,
        Key_M                             = 0x004d,
        Key_N                             = 0x004e,
        Key_O                             = 0x004f,
        Key_P                             = 0x0050,
        Key_Q                             = 0x0051,
        Key_R                             = 0x0052,
        Key_S                             = 0x0053,
        Key_T                             = 0x0054,
        Key_U                             = 0x0055,
        Key_V                             = 0x0056,
        Key_W                             = 0x0057,
        Key_X                             = 0x0058,
        Key_Y                             = 0x0059,
        Key_Z                             = 0x005a,
        Key_BracketLeft                   = 0x005b,
        Key_Backslash                     = 0x005c,
        Key_BracketRight                  = 0x005d,
        Key_AsciiCircum                   = 0x005e,
        Key_Underscore                    = 0x005f,
        Key_Grave                         = 0x0060,
        Key_BraceLeft                     = 0x007b,
        Key_Bar                           = 0x007c,
        Key_BraceRight                    = 0x007d,
        Key_AsciiTilde                    = 0x007e,
        Key_nobreakspace                  = 0x00a0,
        Key_exclamdown                    = 0x00a1,
        Key_cent                          = 0x00a2,
        Key_sterling                      = 0x00a3,
        Key_currency                      = 0x00a4,
        Key_yen                           = 0x00a5,
        Key_brokenbar                     = 0x00a6,
        Key_section                       = 0x00a7,
        Key_diaeresis                     = 0x00a8,
        Key_copyright                     = 0x00a9,
        Key_ordfeminine                   = 0x00aa,
        Key_guillemotleft                 = 0x00ab,
        Key_notsign                       = 0x00ac,
        Key_hyphen                        = 0x00ad,
        Key_registered                    = 0x00ae,
        Key_macron                        = 0x00af,
        Key_degree                        = 0x00b0,
        Key_plusminus                     = 0x00b1,
        Key_twosuperior                   = 0x00b2,
        Key_threesuperior                 = 0x00b3,
        Key_acute                         = 0x00b4,
        Key_mu                            = 0x00b5,
        Key_paragraph                     = 0x00b6,
        Key_periodcentered                = 0x00b7,
        Key_cedilla                       = 0x00b8,
        Key_onesuperior                   = 0x00b9,
        Key_masculine                     = 0x00ba,
        Key_guillemotright                = 0x00bb,
        Key_onequarter                    = 0x00bc,
        Key_onehalf                       = 0x00bd,
        Key_threequarters                 = 0x00be,
        Key_questiondown                  = 0x00bf,
        Key_Agrave                        = 0x00c0,
        Key_Aacute                        = 0x00c1,
        Key_Acircumflex                   = 0x00c2,
        Key_Atilde                        = 0x00c3,
        Key_Adiaeresis                    = 0x00c4,
        Key_Aring                         = 0x00c5,
        Key_AE                            = 0x00c6,
        Key_Ccedilla                      = 0x00c7,
        Key_Egrave                        = 0x00c8,
        Key_Eacute                        = 0x00c9,
        Key_Ecircumflex                   = 0x00ca,
        Key_Ediaeresis                    = 0x00cb,
        Key_Igrave                        = 0x00cc,
        Key_Iacute                        = 0x00cd,
        Key_Icircumflex                   = 0x00ce,
        Key_Idiaeresis                    = 0x00cf,
        Key_ETH                           = 0x00d0,
        Key_Ntilde                        = 0x00d1,
        Key_Ograve                        = 0x00d2,
        Key_Oacute                        = 0x00d3,
        Key_Ocircumflex                   = 0x00d4,
        Key_Otilde                        = 0x00d5,
        Key_Odiaeresis                    = 0x00d6,
        Key_multiply                      = 0x00d7,
        Key_Oslash                        = 0x00d8,
        Key_Ugrave                        = 0x00d9,
        Key_Uacute                        = 0x00da,
        Key_Ucircumflex                   = 0x00db,
        Key_Udiaeresis                    = 0x00dc,
        Key_Yacute                        = 0x00dd,
        Key_THORN                         = 0x00de,
        Key_ssharp                        = 0x00df,
        Key_agrave                        = 0x00e0,
        Key_aacute                        = 0x00e1,
        Key_acircumflex                   = 0x00e2,
        Key_atilde                        = 0x00e3,
        Key_adiaeresis                    = 0x00e4,
        Key_aring                         = 0x00e5,
        Key_ae                            = 0x00e6,
        Key_ccedilla                      = 0x00e7,
        Key_egrave                        = 0x00e8,
        Key_eacute                        = 0x00e9,
        Key_ecircumflex                   = 0x00ea,
        Key_ediaeresis                    = 0x00eb,
        Key_igrave                        = 0x00ec,
        Key_iacute                        = 0x00ed,
        Key_icircumflex                   = 0x00ee,
        Key_idiaeresis                    = 0x00ef,
        Key_eth                           = 0x00f0,
        Key_ntilde                        = 0x00f1,
        Key_ograve                        = 0x00f2,
        Key_oacute                        = 0x00f3,
        Key_ocircumflex                   = 0x00f4,
        Key_otilde                        = 0x00f5,
        Key_odiaeresis                    = 0x00f6,
        Key_division                      = 0x00f7,
        Key_oslash                        = 0x00f8,
        Key_ugrave                        = 0x00f9,
        Key_uacute                        = 0x00fa,
        Key_ucircumflex                   = 0x00fb,
        Key_udiaeresis                    = 0x00fc,
        Key_yacute                        = 0x00fd,
        Key_thorn                         = 0x00fe,
        Key_ydiaeresis                    = 0x00ff,

        // pseudo
        Key_Shift                         = 0x1008FE30,
        Key_Control                       = 0x1008FE31,
        Key_Meta                          = 0x1008FE32,
        Key_Alt                           = 0x1008FE33,

        // compatibility
        Key_PageUp = Key_Page_Up,
        Key_PageDown = Key_Page_Down,
        Key_NumLock = Key_Num_Lock,
        Key_CapsLock = Key_Caps_Lock,
        Key_ScrollLock = Key_Scroll_Lock,
        Key_SysReq = Key_Sys_Req,

        // deprecated and aliases
        Key_Next = Key_Page_Down,
        Key_Prior = Key_Page_Up,
        Key_ooblique = Key_oslash,
        Key_Ooblique = Key_Oslash,
        Key_Thorn = Key_THORN,
        Key_QuoteLeft = Key_Grave,
        Key_QuoteRight = Key_Apostrophe,
        Key_Eth = Key_ETH,
        Key_Henkan_Mode = Key_Henkan,
        Key_Kanji_Bangou = Key_Codeinput,
        Key_Zen_Koho = Key_MultipleCandidate,
        Key_Mae_Koho = Key_PreviousCandidate,
        Key_script_switch = Key_Mode_switch,

        Key_Any = Key_Space,
        Key_Enter = 0xff8d,
        Key_AltGr = 0xfe03,
        Key_Backtab = 0xfe20,
        Key_unknown = 0xffffff
    };

    enum KeyboardModifier {
        NoModifier           = 0x00000000,
        ShiftModifier        = 0x02000000,
        ControlModifier      = 0x04000000,
        AltModifier          = 0x08000000,
        MetaModifier         = 0x10000000,
        KeypadModifier       = 0x20000000,
        GroupSwitchModifier  = 0x40000000,
        // Do not extend the mask to include 0x01000000
        KeyboardModifierMask = 0xfe000000
    };
    Q_DECLARE_FLAGS(KeyboardModifiers, KeyboardModifier)

    // shorter names for shortcuts
    enum Modifier {
        META          = Qt::MetaModifier,
        SHIFT         = Qt::ShiftModifier,
        CTRL          = Qt::ControlModifier,
        ALT           = Qt::AltModifier,
        MODIFIER_MASK = KeyboardModifierMask
    };

    enum MouseButton {
        NoButton         = 0,
        LeftButton       = 0x1,
        RightButton      = 0x2,
        MiddleButton     = 0x4,
    };
    Q_DECLARE_FLAGS(MouseButtons, MouseButton)


    enum Orientation {
        Horizontal = 0x1,
        Vertical = 0x2
    };
    Q_DECLARE_FLAGS(Orientations, Orientation)

    enum FocusPolicy {
        NoFocus = 0,
        TabFocus = 0x1,
        ClickFocus = 0x2,
        StrongFocus = TabFocus | ClickFocus | 0x8,
        WheelFocus = StrongFocus | 0x4
    };

    enum SortOrder {
        AscendingOrder,
        DescendingOrder
    };

    enum TileRule {
        StretchTile,
        RepeatTile,
        RoundTile
    };

    // Text formatting flags for QPainter::drawText and QLabel.
    // The following two enums can be combined to one integer which
    // is passed as 'flags' to drawText and qt_format_text.
    enum AlignmentFlag {
        AlignLeft = 0x0001,
        AlignLeading = AlignLeft,
        AlignRight = 0x0002,
        AlignTrailing = AlignRight,
        AlignHCenter = 0x0004,
        AlignJustify = 0x0008,
        AlignAbsolute = 0x0010,
        AlignHorizontal_Mask = AlignLeft | AlignRight | AlignHCenter | AlignJustify | AlignAbsolute,

        AlignTop = 0x0020,
        AlignBottom = 0x0040,
        AlignVCenter = 0x0080,
        AlignVertical_Mask = AlignTop | AlignBottom | AlignVCenter,

        AlignCenter = AlignVCenter | AlignHCenter
    };
    Q_DECLARE_FLAGS(Alignment, AlignmentFlag)

    enum TextFlag {
        TextSingleLine = 0x0100,
        TextDontClip = 0x0200,
        TextShowMnemonic = 0x0400,
        TextWordWrap = 0x0800,
        TextWrapAnywhere = 0x1000,
        TextDontPrint = 0x2000,
        TextHideMnemonic = 0x4000,
        TextForceLeftToRight = 0x8000,
        TextForceRightToLeft = 0x10000,
        TextLongestVariant = 0x20000,
        TextIncludeTrailingSpaces = 0x08000000
    };

    enum TextElideMode {
        ElideLeft,
        ElideRight,
        ElideMiddle,
        ElideNone
    };

    enum WindowType {
        Widget = 0x00000000,
        Window = 0x00000001,
        Dialog = 0x00000002 | Window,
        Popup = 0x00000008 | Window,
        Tool = 0x0000000a | Window,
        ToolTip = 0x0000000c | Window,
        SplashScreen = 0x0000000e | Window,
        Desktop = 0x00000010 | Window,
        SubWindow =  0x00000012,

        WindowType_Mask = 0x000000ff,
        X11BypassWindowManagerHint = 0x00000100,
        FramelessWindowHint = 0x00000200,
        WindowTitleHint = 0x00000400,
        WindowSystemMenuHint = 0x00000800,
        WindowMinimizeButtonHint = 0x00001000,
        WindowMaximizeButtonHint = 0x00002000,
        WindowMinMaxButtonsHint = WindowMinimizeButtonHint | WindowMaximizeButtonHint,
        WindowContextHelpButtonHint = 0x00004000,
        WindowShadeButtonHint = 0x00008000,
        WindowStaysOnTopHint = 0x00010000,
        CustomizeWindowHint = 0x00020000,
        WindowStaysOnBottomHint = 0x00040000,
        WindowCloseButtonHint = 0x00080000,
        BypassGraphicsProxyWidget = 0x00100000
    };
    Q_DECLARE_FLAGS(WindowFlags, WindowType)

    enum WindowState {
        WindowNoState    = 0x00000000,
        WindowMinimized  = 0x00000001,
        WindowMaximized  = 0x00000002,
        WindowFullScreen = 0x00000004,
        WindowActive     = 0x00000008
    };
    Q_DECLARE_FLAGS(WindowStates, WindowState)

    enum WidgetAttribute {
        WA_Disabled = 0,
        WA_UnderMouse = 1,
        WA_MouseTracking = 2,
        WA_OpaquePaintEvent = 3,
        WA_NoBackground = WA_OpaquePaintEvent, // ## deprecated
        WA_PaintOnScreen = 4,
        WA_NoSystemBackground = 5,
        WA_UpdatesDisabled = 6,
        WA_Mapped = 7,
        WA_ForceDisabled = 8,
        WA_PendingMoveEvent = 9,
        WA_PendingResizeEvent = 10,
        WA_SetPalette = 11,
        WA_SetFont = 12,
        WA_SetCursor = 13,
        WA_NoChildEventsFromChildren = 14,
        WA_WindowModified = 15,
        WA_Resized = 16,
        WA_Moved = 17,
        WA_CustomWhatsThis = 18,
        WA_LayoutOnEntireRect = 19,
        WA_OutsideWSRange = 20,
        WA_TransparentForMouseEvents = 21,
        WA_NoMouseReplay = 22,
        WA_DeleteOnClose = 23,
        WA_RightToLeft = 24,
        WA_NoChildEventsForParent = 25,
        WA_ShowModal = 26, // ## deprecated
        WA_MouseNoMask = 27,
        WA_GroupLeader = 28, // ## deprecated
        WA_NoMousePropagation = 29, // ## for now, might go away.
        WA_Hover = 30,
        WA_QuitOnClose = 31,
        WA_KeyboardFocusChange = 32,
        WA_AcceptDrops = 33,
        WA_WindowPropagation = 34,
        WA_AlwaysShowToolTips = 35,
        WA_SetStyle = 36,
        WA_SetLocale = 37,
        WA_LayoutUsesWidgetRect = 38,
        WA_ShowWithoutActivating = 39,
        WA_NativeWindow = 40,
        WA_DontCreateNativeAncestors = 41,
        WA_TranslucentBackground = 42,
        WA_X11DoNotAcceptFocus = 43,
        WA_X11BypassTransientForHint = 44,

        // window types from http://standards.freedesktop.org/wm-spec/
        WA_X11NetWmWindowTypeDesktop = 45,
        WA_X11NetWmWindowTypeDock = 46,
        WA_X11NetWmWindowTypeToolBar = 47,
        WA_X11NetWmWindowTypeMenu = 48,
        WA_X11NetWmWindowTypeUtility = 49,
        WA_X11NetWmWindowTypeSplash = 50,
        WA_X11NetWmWindowTypeDialog = 51,
        WA_X11NetWmWindowTypeDropDownMenu = 52,
        WA_X11NetWmWindowTypePopupMenu = 53,
        WA_X11NetWmWindowTypeToolTip = 54,
        WA_X11NetWmWindowTypeNotification = 55,
        WA_X11NetWmWindowTypeCombo = 56,
        WA_X11NetWmWindowTypeDND = 57,

        // internal
        WA_LaidOut = 58,
        WA_GrabbedShortcut = 59,
        WA_DontShowOnScreen = 60,
        WA_ForceUpdatesDisabled = 61,
        WA_StyledBackground = 62,
        WA_StyleSheet = 63,
        WA_DropSiteRegistered = 64,
        WA_WState_Visible = 65,
        WA_WState_Hidden = 66,
        WA_WState_Created = 67,
        WA_WState_InPaintEvent = 68,
        WA_WState_Reparented = 69,
        WA_WState_Polished = 70,
        WA_WState_OwnSizePolicy = 71,
        WA_WState_ExplicitShowHide = 72,
        WA_WState_ConfigPending = 73,
        WA_SetWindowIcon = 74,
        WA_SetLayoutDirection = 75,
        WA_SetWindowModality = 76,
        WA_NoX11EventCompression = 77,

        // Add new attributes before this line
        WA_AttributeCount
    };

    enum ApplicationAttribute {
        AA_ImmediateWidgetCreation = 0,
        AA_DontShowIconsInMenus = 1,
        AA_NativeWindows = 2,
        AA_DontCreateNativeWidgetSiblings = 3,
        AA_X11InitThreads = 4,

        // Add new attributes before this line
        AA_AttributeCount
    };

    // Image conversion flags.  The unusual ordering is caused by
    // compatibility and default requirements.
    enum ImageConversionFlag {
        ColorMode_Mask          = 0x00000003,
        AutoColor               = 0x00000000,
        ColorOnly               = 0x00000003,
        MonoOnly                = 0x00000002,
        // Reserved             = 0x00000001,

        AlphaDither_Mask        = 0x0000000c,
        ThresholdAlphaDither    = 0x00000000,
        OrderedAlphaDither      = 0x00000004,
        DiffuseAlphaDither      = 0x00000008,

        Dither_Mask             = 0x00000030,
        DiffuseDither           = 0x00000000,
        OrderedDither           = 0x00000010,
        ThresholdDither         = 0x00000020,
        // ReservedDither       = 0x00000030,

        DitherMode_Mask         = 0x000000c0,
        AutoDither              = 0x00000000,
        PreferDither            = 0x00000040,
        AvoidDither             = 0x00000080,

        NoOpaqueDetection       = 0x00000100
    };
    Q_DECLARE_FLAGS(ImageConversionFlags, ImageConversionFlag)

    enum BGMode {
        TransparentMode,
        OpaqueMode
    };

    enum ArrowType {
        NoArrow,
        UpArrow,
        DownArrow,
        LeftArrow,
        RightArrow
    };

    enum PenStyle {
        NoPen,
        SolidLine,
        DashLine,
        DotLine,
        DashDotLine,
        DashDotDotLine,
        CustomDashLine
    };

    // line endcap style
    enum PenCapStyle {
        FlatCap = 0x00,
        SquareCap = 0x10,
        RoundCap = 0x20,
    };

    // line join style
    enum PenJoinStyle {
        MiterJoin = 0x00,
        BevelJoin = 0x40,
        RoundJoin = 0x80,
        SvgMiterJoin = 0x100,
    };

    enum BrushStyle {
        NoBrush,
        SolidPattern,
        Dense1Pattern,
        Dense2Pattern,
        Dense3Pattern,
        Dense4Pattern,
        Dense5Pattern,
        Dense6Pattern,
        Dense7Pattern,
        HorPattern,
        VerPattern,
        CrossPattern,
        BDiagPattern,
        FDiagPattern,
        DiagCrossPattern,
        LinearGradientPattern,
        RadialGradientPattern,
        TexturePattern
    };

    enum SizeMode {
        AbsoluteSize,
        RelativeSize
    };

    enum UIEffect {
        UI_General,
        UI_FadeMenu,
        UI_FadeTooltip
    };

    enum CursorShape {
        ArrowCursor,
        UpArrowCursor,
        CrossCursor,
        WaitCursor,
        IBeamCursor,
        SizeVerCursor,
        SizeHorCursor,
        SizeBDiagCursor,
        SizeFDiagCursor,
        SizeAllCursor,
        BlankCursor,
        SplitVCursor,
        SplitHCursor,
        PointingHandCursor,
        ForbiddenCursor,
        WhatsThisCursor,
        BusyCursor,
        OpenHandCursor,
        ClosedHandCursor,
        DragCopyCursor,
        DragMoveCursor,
        DragLinkCursor,
        LastCursor = DragLinkCursor,
        BitmapCursor = 24,
        CustomCursor = 25
    };

    enum TextFormat {
        PlainText,
        RichText,
        AutoText
    };

    enum AspectRatioMode {
        IgnoreAspectRatio,
        KeepAspectRatio,
        KeepAspectRatioByExpanding
    };

    enum DockWidgetArea {
        LeftDockWidgetArea = 0x1,
        RightDockWidgetArea = 0x2,
        TopDockWidgetArea = 0x4,
        BottomDockWidgetArea = 0x8,

        DockWidgetArea_Mask = 0xf,
        AllDockWidgetAreas = DockWidgetArea_Mask,
        NoDockWidgetArea = 0
    };
    enum DockWidgetAreaSizes {
        NDockWidgetAreas = 4
    };
    Q_DECLARE_FLAGS(DockWidgetAreas, DockWidgetArea)

    enum ToolBarArea {
        LeftToolBarArea = 0x1,
        RightToolBarArea = 0x2,
        TopToolBarArea = 0x4,
        BottomToolBarArea = 0x8,

        ToolBarArea_Mask = 0xf,
        AllToolBarAreas = ToolBarArea_Mask,
        NoToolBarArea = 0
    };

    enum ToolBarAreaSizes {
        NToolBarAreas = 4
    };
    Q_DECLARE_FLAGS(ToolBarAreas, ToolBarArea)

    enum DateFormat {
        TextDate,      // default Qt
        ISODate,       // ISO 8601
        SystemLocaleShortDate,
        SystemLocaleLongDate
    };

    enum TimeSpec {
        LocalTime,
        UTC,
        OffsetFromUTC
    };

    enum DayOfWeek {
        Monday = 1,
        Tuesday = 2,
        Wednesday = 3,
        Thursday = 4,
        Friday = 5,
        Saturday = 6,
        Sunday = 7
    };

    enum ScrollBarPolicy {
        ScrollBarAsNeeded,
        ScrollBarAlwaysOff,
        ScrollBarAlwaysOn
    };

    enum CaseSensitivity {
        CaseInsensitive,
        CaseSensitive
    };

    enum Corner {
        TopLeftCorner = 0x00000,
        TopRightCorner = 0x00001,
        BottomLeftCorner = 0x00002,
        BottomRightCorner = 0x00003
    };

    enum ConnectionType {
        AutoConnection,
        DirectConnection,
        QueuedConnection,
        BlockingQueuedConnection,
        UniqueConnection =  0x80
    };

    enum ShortcutContext {
        WidgetShortcut,
        WindowShortcut,
        ApplicationShortcut,
        WidgetWithChildrenShortcut
    };

    enum FillRule {
        OddEvenFill,
        WindingFill
    };

    enum MaskMode {
        MaskInColor,
        MaskOutColor
    };

    enum ClipOperation {
        NoClip,
        ReplaceClip,
        IntersectClip,
        UniteClip
    };

    // Shape = 0x1, BoundingRect = 0x2
    enum ItemSelectionMode {
        ContainsItemShape = 0x0,
        IntersectsItemShape = 0x1,
        ContainsItemBoundingRect = 0x2,
        IntersectsItemBoundingRect = 0x3
    };

    enum TransformationMode {
        FastTransformation,
        SmoothTransformation
    };

    enum Axis {
        XAxis,
        YAxis,
        ZAxis
    };

    enum FocusReason {
        MouseFocusReason,
        TabFocusReason,
        BacktabFocusReason,
        ActiveWindowFocusReason,
        PopupFocusReason,
        ShortcutFocusReason,
        MenuBarFocusReason,
        OtherFocusReason,
        NoFocusReason
    };

    enum ContextMenuPolicy {
        NoContextMenu,
        DefaultContextMenu,
        ActionsContextMenu,
        CustomContextMenu,
        PreventContextMenu
    };

    enum ToolButtonStyle {
        ToolButtonIconOnly,
        ToolButtonTextOnly,
        ToolButtonTextBesideIcon,
        ToolButtonTextUnderIcon,
        ToolButtonFollowStyle
    };

    enum LayoutDirection {
        LayoutDirectionAuto = 0,
        LeftToRight = 1,
        RightToLeft = 2
    };

    enum AnchorPoint {
        AnchorLeft = 0,
        AnchorHorizontalCenter,
        AnchorRight,
        AnchorTop,
        AnchorVerticalCenter,
        AnchorBottom
    };

    enum DropAction {
        CopyAction = 0x1,
        MoveAction = 0x2,
        LinkAction = 0x4,
        ActionMask = 0xff,
        IgnoreAction = 0x0
    };
    Q_DECLARE_FLAGS(DropActions, DropAction)

    enum CheckState {
        Unchecked,
        PartiallyChecked,
        Checked
    };

    enum ItemDataRole {
        DisplayRole = 0,
        DecorationRole = 1,
        EditRole = 2,
        ToolTipRole = 3,
        StatusTipRole = 4,
        WhatsThisRole = 5,
        // Metadata
        FontRole = 6,
        TextAlignmentRole = 7,
        BackgroundRole = 8,
        ForegroundRole = 9,
        CheckStateRole = 10,
        // More general purpose
        SizeHintRole = 11,
        InitialSortOrderRole = 12,
        // Internal UiLib roles
        DisplayPropertyRole = 13,
        DecorationPropertyRole = 14,
        ToolTipPropertyRole = 15,
        StatusTipPropertyRole = 16,
        WhatsThisPropertyRole = 17,
        // Reserved
        UserRole = 32
    };

    enum ItemFlag {
        NoItemFlags = 0,
        ItemIsSelectable = 1,
        ItemIsEditable = 2,
        ItemIsDragEnabled = 4,
        ItemIsDropEnabled = 8,
        ItemIsUserCheckable = 16,
        ItemIsEnabled = 32,
        ItemIsTristate = 64
    };
    Q_DECLARE_FLAGS(ItemFlags, ItemFlag)

    enum MatchFlag {
        MatchExactly = 0,
        MatchContains = 1,
        MatchStartsWith = 2,
        MatchEndsWith = 3,
        MatchRegExp = 4,
        MatchWildcard = 5,
        MatchFixedString = 8,
        MatchCaseSensitive = 16,
        MatchWrap = 32,
        MatchRecursive = 64
    };
    Q_DECLARE_FLAGS(MatchFlags, MatchFlag)

#if defined(Q_WS_X11)
    typedef unsigned long HANDLE;
#endif
    typedef WindowFlags WFlags;

    enum WindowModality {
        NonModal,
        WindowModal,
        ApplicationModal
    };

    enum TextInteractionFlag {
        NoTextInteraction         = 0,
        TextSelectableByMouse     = 1,
        TextSelectableByKeyboard  = 2,
        LinksAccessibleByMouse    = 4,
        LinksAccessibleByKeyboard = 8,
        TextEditable              = 16,

        TextEditorInteraction     = TextSelectableByMouse | TextSelectableByKeyboard | TextEditable,
        TextBrowserInteraction    = TextSelectableByMouse | LinksAccessibleByMouse | LinksAccessibleByKeyboard
    };
    Q_DECLARE_FLAGS(TextInteractionFlags, TextInteractionFlag)

    enum EventPriority {
        HighEventPriority = 1,
        NormalEventPriority = 0,
        LowEventPriority = -1
    };

    enum SizeHint {
        MinimumSize,
        PreferredSize,
        MaximumSize,
        MinimumDescent,
        NSizeHints
    };

    enum WindowFrameSection {
        NoSection,
        LeftSection,           // For resize
        TopLeftSection,
        TopSection,
        TopRightSection,
        RightSection,
        BottomRightSection,
        BottomSection,
        BottomLeftSection,
        TitleBarArea    // For move
    };

    enum Initialization {
        Uninitialized
    };

    enum CoordinateSystem {
        DeviceCoordinates,
        LogicalCoordinates
    };

    enum HitTestAccuracy {
        ExactHit,
        FuzzyHit
    };

    enum WhiteSpaceMode {
        WhiteSpaceNormal,
        WhiteSpacePre,
        WhiteSpaceNoWrap,
        WhiteSpaceModeUndefined = -1
    };

    Q_CORE_EXPORT bool mightBeRichText(const QString&);
    Q_CORE_EXPORT QString escape(const QString& plain);
    Q_CORE_EXPORT QString convertFromPlainText(const QString &plain, WhiteSpaceMode mode = WhiteSpacePre);
}
#ifdef Q_MOC_RUN
 ;
#endif

Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::MouseButtons)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::Orientations)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::KeyboardModifiers)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::WindowFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::Alignment)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::ImageConversionFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::DockWidgetAreas)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::ToolBarAreas)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::WindowStates)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::DropActions)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::ItemFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::MatchFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qt::TextInteractionFlags)

class Q_CORE_EXPORT QInternal {
public:
    enum PaintDeviceFlags {
        UnknownDevice     = 0,
        Widget            = 1,
        Pixmap            = 2,
        Image             = 3,
        Printer           = 4
    };

    enum DockPosition {
        LeftDock,
        RightDock,
        TopDock,
        BottomDock,
        DockCount
    };
};

QT_END_NAMESPACE


#endif // QNAMESPACE_H
