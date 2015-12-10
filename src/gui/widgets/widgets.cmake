set(GUI_HEADERS
    ${GUI_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qbuttongroup.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractbutton_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractslider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractslider_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractspinbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractspinbox_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcalendartextnavigator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcalendarwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcheckbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcombobox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcombobox_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcommandlinkbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdatetimeedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdatetimeedit_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdial.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdialogbuttonbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdockwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdockwidget_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdockarealayout_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qfontcombobox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qframe_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qgroupbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlabel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlabel_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlcdnumber.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlineedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlineedit_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlinecontrol_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmainwindowlayout_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmdiarea.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmdiarea_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmdisubwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmdisubwindow_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenu_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenubar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenubar_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenudata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qprogressbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qpushbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qpushbutton_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qradiobutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qrubberband.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qscrollbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qscrollarea_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qsizegrip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qslider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qspinbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qsplashscreen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qsplitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qsplitter_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qstackedwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qstatusbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtabbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtabbar_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtabwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtextedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtextedit_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtextbrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbar_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbarlayout_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbarextension_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbarseparator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qvalidator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractscrollarea.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractscrollarea_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwidgetresizehandler_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qfocusframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qscrollarea.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qworkspace.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwidgetanimator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbararealayout_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qplaintextedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qplaintextedit_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qprintpreviewwidget.h
    
    # XXX: obsolete?
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdoublespinbox.h
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractslider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractspinbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcalendarwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcheckbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcombobox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcommandlinkbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdatetimeedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdial.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdialogbuttonbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdockwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qdockarealayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qeffects.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qfontcombobox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qgroupbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlabel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlcdnumber.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlineedit_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlineedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qlinecontrol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmainwindowlayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmdiarea.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmdisubwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenubar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenudata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qprogressbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qpushbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qradiobutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qrubberband.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qscrollbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qsizegrip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qslider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qspinbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qsplashscreen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qsplitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qstackedwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qstatusbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtabbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtabwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtextedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtextbrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbarlayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbarextension.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbarseparator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qvalidator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractscrollarea.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwidgetresizehandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qfocusframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qscrollarea.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qworkspace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qwidgetanimator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qtoolbararealayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qplaintextedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qprintpreviewwidget.cpp
)

if(WITH_X11 AND X11_FOUND)
    set(GUI_HEADERS
        ${GUI_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qabstractplatformmenubar_p.h
    )
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenubar_x11.cpp
    )
endif()

if(KATIE_PLATFORM STREQUAL "mac")
    set(GUI_HEADERS
        ${GUI_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmacnativewidget_mac.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmaccocoaviewcontainer_mac.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcocoatoolbardelegate_mac_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcocoamenu_mac_p.h
    )
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenu_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmaccocoaviewcontainer_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcocoatoolbardelegate_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmainwindowlayout_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmacnativewidget_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qcocoamenu_mac.mm
    )
elseif(KATIE_PLATFORM STREQUAL "wince")
    set(GUI_HEADERS
        ${GUI_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenu_wince_resource_p.h
    )
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenu_wince.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qmenu_wince.rc
    )
endif()