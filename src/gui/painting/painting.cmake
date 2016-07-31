set(GUI_HEADERS
    ${GUI_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qbezier_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qbrush.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcolor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcolor_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcolormap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawutil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qgraphicssystem_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qmatrix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qmemrotate_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qoutlinemapper_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintdevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_alpha_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_preview_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengineex_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpainter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpainter_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpainterpath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpainterpath_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qvectorpath_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpathclipper_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpdf_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpolygon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpolygonclipper_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprintengine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprintengine_pdf_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprintengine_ps_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprinter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprinter_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprinterinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprinterinfo_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qrasterizer_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qregion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qstroker_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qstylepainter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qtessellator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qtextureglyphcache_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qtransform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qwindowsurface_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qwmatrix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_raster_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qblendfunctions_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qrasterdefs_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qgrayraster_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qbackingstore_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qmath_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdatabuffer_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpen_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qfixed_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qgraphicssystem_raster_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qgraphicssystemfactory_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qgraphicssystemplugin_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qwindowsurface_raster_p.h

    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_x86_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_mmx_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_sse_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawingprimitive_sse2_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_neon_p.h

    # XXX: obsolete?
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qrgb.h
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qbezier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qblendfunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qbrush.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcolor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcolor_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcssutil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawutil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qgraphicssystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qmatrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qmemrotate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qoutlinemapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintdevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_alpha.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_preview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengineex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpainter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpainterpath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpathclipper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpolygon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprintengine_pdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprintengine_ps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprinterinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qrasterizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qregion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qstroker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qstylepainter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qtessellator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qtextureglyphcache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qtransform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qwindowsurface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_raster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qgrayraster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qbackingstore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qgraphicssystem_raster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qgraphicssystemfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qgraphicssystemplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qwindowsurface_raster.cpp
)

katie_setup_sources(
    GUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_mmx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_mmx3dnow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_sse3dnow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_sse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_sse2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_ssse3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_iwmmxt.cpp
    # TODO: link to pixman for arm-neon?
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_neon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/painting/qdrawhelper_neon_asm.S
)

if(WITH_X11 AND X11_FOUND)
    set(GUI_HEADERS
        ${GUI_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_x11_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qwindowsurface_x11_p.h
    )
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcolormap_x11.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintdevice_x11.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_x11.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qwindowsurface_x11.cpp
    )
endif()

if(KATIE_PLATFORM STREQUAL "win32")
    set(GUI_HEADERS
        ${GUI_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprintengine_win_p.h
    )
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcolormap_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintdevice_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprintengine_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprinterinfo_win.cpp
    )
    set(EXTRA_GUI_LIBS
        ${EXTRA_GUI_LIBS}
        msimg32
    )
elseif(KATIE_PLATFORM STREQUAL "mac")
    set(GUI_HEADERS
        ${GUI_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qwindowsurface_mac_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qunifiedtoolbarsurface_mac_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_mac_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprintengine_mac_p.h
    )
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qwindowsurface_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qunifiedtoolbarsurface_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcolormap_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintdevice_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qpaintengine_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprinterinfo_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprintengine_mac.mm
    )
endif()

if(UNIX AND NOT KATIE_PLATFORM STREQUAL "mac")
    set(GUI_HEADERS
        ${GUI_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprinterinfo_unix_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcups_p.h
    )
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qprinterinfo_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/painting/qcups.cpp
    )
endif()