#!/usr/bin/python

import os, re

# generated via find /usr/include/katie/ -name 'Q*' -printf '    "%f",\n' | sort -u
classlist = [
    "QAbstractAnimation",
    "QAbstractButton",
    "QAbstractEventDispatcher",
    "QAbstractGraphicsShapeItem",
    "QAbstractItemDelegate",
    "QAbstractItemModel",
    "QAbstractItemView",
    "QAbstractListModel",
    "QAbstractPageSetupDialog",
    "QAbstractPrintDialog",
    "QAbstractProxyModel",
    "QAbstractScrollArea",
    "QAbstractSlider",
    "QAbstractSocket",
    "QAbstractSpinBox",
    "QAbstractTableModel",
    "QAbstractTextDocumentLayout",
    "QAbstractUndoItem",
    "QAction",
    "QActionEvent",
    "QActionGroup",
    "QAnimationGroup",
    "QApplication",
    "QArgument",
    "QAtomicInt",
    "QAtomicPointer",
    "QBasicTimer",
    "QBitArray",
    "QBitRef",
    "QBitmap",
    "QBoxLayout",
    "QBrush",
    "QBrushData",
    "QBuffer",
    "QButtonGroup",
    "QByteArray",
    "QByteArrayMatcher",
    "QByteRef",
    "QCache",
    "QCalendarWidget",
    "QChar",
    "QCharRef",
    "QCheckBox",
    "QChildEvent",
    "QCleanlooksStyle",
    "QClipboard",
    "QClipboardEvent",
    "QCloseEvent",
    "QColor",
    "QColorDialog",
    "QColumnView",
    "QComboBox",
    "QCommandLinkButton",
    "QCommonStyle",
    "QCompleter",
    "QContextMenuEvent",
    "QCoreApplication",
    "QCryptographicHash",
    "QCursor",
    "QCustomWidget",
    "QCustomWidgetPlugin",
    "QDBusAbstractAdaptor",
    "QDBusAbstractInterface",
    "QDBusAbstractInterfaceBase",
    "QDBusArgument",
    "QDBusConnection",
    "QDBusConnectionInterface",
    "QDBusContext",
    "QDBusError",
    "QDBusInterface",
    "QDBusMessage",
    "QDBusMetaType",
    "QDBusObjectPath",
    "QDBusPendingCall",
    "QDBusPendingCallWatcher",
    "QDBusPendingReply",
    "QDBusPendingReplyData",
    "QDBusReply",
    "QDBusServer",
    "QDBusServiceWatcher",
    "QDBusSignature",
    "QDBusUnixFileDescriptor",
    "QDBusVariant",
    "QDataStream",
    "QDataWidgetMapper",
    "QDate",
    "QDateEdit",
    "QDateTime",
    "QDateTimeEdit",
    "QDebug",
    "QDeclarativeAttachedPropertiesFunc",
    "QDeclarativeComponent",
    "QDeclarativeContext",
    "QDeclarativeEngine",
    "QDeclarativeError",
    "QDeclarativeExpression",
    "QDeclarativeExtensionInterface",
    "QDeclarativeExtensionPlugin",
    "QDeclarativeImageProvider",
    "QDeclarativeInfo",
    "QDeclarativeItem",
    "QDeclarativeListProperty",
    "QDeclarativeListReference",
    "QDeclarativeParserStatus",
    "QDeclarativeProperties",
    "QDeclarativeProperty",
    "QDeclarativePropertyMap",
    "QDeclarativePropertyValueInterceptor",
    "QDeclarativePropertyValueSource",
    "QDeclarativeScriptString",
    "QDeclarativeTypeInfo",
    "QDeclarativeView",
    "QDesktopWidget",
    "QDial",
    "QDialog",
    "QDialogButtonBox",
    "QDir",
    "QDirIterator",
    "QDirModel",
    "QDockWidget",
    "QDomAttr",
    "QDomCDATASection",
    "QDomCharacterData",
    "QDomComment",
    "QDomDocument",
    "QDomDocumentFragment",
    "QDomDocumentType",
    "QDomElement",
    "QDomEntity",
    "QDomEntityReference",
    "QDomImplementation",
    "QDomNamedNodeMap",
    "QDomNode",
    "QDomNodeList",
    "QDomNotation",
    "QDomProcessingInstruction",
    "QDomText",
    "QDoubleSpinBox",
    "QDoubleValidator",
    "QDrag",
    "QDragEnterEvent",
    "QDragLeaveEvent",
    "QDragMoveEvent",
    "QDropEvent",
    "QDynamicPropertyChangeEvent",
    "QEasingCurve",
    "QElapsedTimer",
    "QEvent",
    "QEventLoop",
    "QEventSizeOfChecker",
    "QExplicitlySharedDataPointer",
    "QFile",
    "QFileDialog",
    "QFileIconProvider",
    "QFileInfo",
    "QFileInfoList",
    "QFileSystemModel",
    "QFileSystemWatcher",
    "QFlag",
    "QFlags",
    "QFocusEvent",
    "QFocusFrame",
    "QFont",
    "QFontComboBox",
    "QFontDatabase",
    "QFontDialog",
    "QFontMetrics",
    "QFontMetricsF",
    "QFormLayout",
    "QFrame",
    "QGenericArgument",
    "QGenericMatrix",
    "QGenericReturnArgument",
    "QGradient",
    "QGradientStop",
    "QGradientStops",
    "QGraphicsAnchor",
    "QGraphicsAnchorLayout",
    "QGraphicsEllipseItem",
    "QGraphicsGridLayout",
    "QGraphicsItem",
    "QGraphicsItemAnimation",
    "QGraphicsItemGroup",
    "QGraphicsLayout",
    "QGraphicsLayoutItem",
    "QGraphicsLineItem",
    "QGraphicsLinearLayout",
    "QGraphicsObject",
    "QGraphicsPathItem",
    "QGraphicsPixmapItem",
    "QGraphicsPolygonItem",
    "QGraphicsProxyWidget",
    "QGraphicsRectItem",
    "QGraphicsRotation",
    "QGraphicsScale",
    "QGraphicsScene",
    "QGraphicsSceneContextMenuEvent",
    "QGraphicsSceneDragDropEvent",
    "QGraphicsSceneEvent",
    "QGraphicsSceneHelpEvent",
    "QGraphicsSceneHoverEvent",
    "QGraphicsSceneMouseEvent",
    "QGraphicsSceneMoveEvent",
    "QGraphicsSceneResizeEvent",
    "QGraphicsSceneWheelEvent",
    "QGraphicsSimpleTextItem",
    "QGraphicsTextItem",
    "QGraphicsView",
    "QGraphicsWidget",
    "QGridLayout",
    "QGroupBox",
    "QGuiPlatformPlugin",
    "QHBoxLayout",
    "QHash",
    "QHashData",
    "QHashIterator",
    "QHashNode",
    "QHeaderView",
    "QHelpEvent",
    "QHideEvent",
    "QHostAddress",
    "QHostInfo",
    "QHoverEvent",
    "QIODevice",
    "QIPv6Address",
    "QIcon",
    "QIconEngine",
    "QIconEngineV2",
    "QIdentityProxyModel",
    "QImage",
    "QImageIOHandler",
    "QImageIOPlugin",
    "QImageReader",
    "QImageWriter",
    "QIncompatibleFlag",
    "QInputDialog",
    "QInputEvent",
    "QIntValidator",
    "QInternal",
    "QItemDelegate",
    "QItemEditorCreator",
    "QItemEditorCreatorBase",
    "QItemEditorFactory",
    "QItemSelection",
    "QItemSelectionModel",
    "QItemSelectionRange",
    "QJsonDocument",
    "QKeyEvent",
    "QKeySequence",
    "QLCDNumber",
    "QLabel",
    "QLatin1Char",
    "QLatin1String",
    "QLayout",
    "QLayoutItem",
    "QLibrary",
    "QLibraryInfo",
    "QLine",
    "QLineEdit",
    "QLineF",
    "QLinearGradient",
    "QList",
    "QListData",
    "QListIterator",
    "QListView",
    "QListWidget",
    "QListWidgetItem",
    "QLocalServer",
    "QLocalSocket",
    "QLocale",
    "QMainWindow",
    "QMap",
    "QMapData",
    "QMapIterator",
    "QMapNode",
    "QMapPayloadNode",
    "QMargins",
    "QMatrix",
    "QMatrix2x2",
    "QMatrix2x3",
    "QMatrix2x4",
    "QMatrix3x2",
    "QMatrix3x3",
    "QMatrix3x4",
    "QMatrix4x2",
    "QMatrix4x3",
    "QMatrix4x4",
    "QMenu",
    "QMenuBar",
    "QMessageBox",
    "QMetaClassInfo",
    "QMetaEnum",
    "QMetaMethod",
    "QMetaObject",
    "QMetaObjectAccessor",
    "QMetaProperty",
    "QMetaType",
    "QMetaTypeId",
    "QMetaTypeId2",
    "QMimeData",
    "QModelIndex",
    "QModelIndexList",
    "QMouseEvent",
    "QMoveEvent",
    "QMovie",
    "QMultiHash",
    "QMultiMap",
    "QMutableHashIterator",
    "QMutableListIterator",
    "QMutableMapIterator",
    "QMutableSetIterator",
    "QMutableStringListIterator",
    "QMutableVectorIterator",
    "QMutex",
    "QMutexLocker",
    "QNetworkAddressEntry",
    "QNetworkInterface",
    "QObject",
    "QObjectCleanupHandler",
    "QObjectData",
    "QObjectList",
    "QPageSetupDialog",
    "QPaintDevice",
    "QPaintEngine",
    "QPaintEngineState",
    "QPaintEvent",
    "QPainter",
    "QPainterPath",
    "QPainterPathPrivate",
    "QPainterPathStroker",
    "QPair",
    "QPalette",
    "QParallelAnimationGroup",
    "QPauseAnimation",
    "QPen",
    "QPersistentModelIndex",
    "QPixmap",
    "QPixmapCache",
    "QPlainTextDocumentLayout",
    "QPlainTextEdit",
    "QPlugin",
    "QPluginLoader",
    "QPoint",
    "QPointF",
    "QPointer",
    "QPolygon",
    "QPolygonF",
    "QPrintDialog",
    "QPrintEngine",
    "QPrintPreviewDialog",
    "QPrintPreviewWidget",
    "QPrinter",
    "QPrinterInfo",
    "QProcess",
    "QProcessEnvironment",
    "QProgressBar",
    "QProgressDialog",
    "QPropertyAnimation",
    "QProxyModel",
    "QProxyStyle",
    "QPushButton",
    "QQueue",
    "QRadialGradient",
    "QRadioButton",
    "QRect",
    "QRectF",
    "QRegExp",
    "QRegExpValidator",
    "QRegion",
    "QResizeEvent",
    "QReturnArgument",
    "QRgb",
    "QRubberBand",
    "QRunnable",
    "QScopedPointer",
    "QScopedPointerPodDeleter",
    "QScopedValueRollback",
    "QScriptClass",
    "QScriptClassPropertyIterator",
    "QScriptContext",
    "QScriptContextInfo",
    "QScriptContextInfoList",
    "QScriptEngine",
    "QScriptEngineAgent",
    "QScriptExtensionInterface",
    "QScriptExtensionPlugin",
    "QScriptProgram",
    "QScriptString",
    "QScriptSyntaxCheckResult",
    "QScriptValue",
    "QScriptValueIterator",
    "QScriptValueList",
    "QScriptable",
    "QScrollArea",
    "QScrollBar",
    "QSemaphore",
    "QSequentialAnimationGroup",
    "QSessionManager",
    "QSet",
    "QSetIterator",
    "QSettings",
    "QSharedData",
    "QSharedDataPointer",
    "QSharedPointer",
    "QShortcut",
    "QShortcutEvent",
    "QShowEvent",
    "QSignalMapper",
    "QSignalSpy",
    "QSize",
    "QSizeF",
    "QSizeGrip",
    "QSizePolicy",
    "QSlider",
    "QSocketNotifier",
    "QSortFilterProxyModel",
    "QSpacerItem",
    "QSpinBox",
    "QSplashScreen",
    "QSplitter",
    "QSplitterHandle",
    "QSpontaneKeyEvent",
    "QStack",
    "QStackedLayout",
    "QStackedWidget",
    "QStandardItem",
    "QStandardItemEditorCreator",
    "QStandardItemModel",
    "QStandardPaths",
    "QStatusBar",
    "QStatusTipEvent",
    "QString",
    "QStringList",
    "QStringListIterator",
    "QStringListModel",
    "QStringMatcher",
    "QStringRef",
    "QStyle",
    "QStyleFactory",
    "QStyleHintReturn",
    "QStyleHintReturnMask",
    "QStyleHintReturnVariant",
    "QStyleOption",
    "QStyleOptionButton",
    "QStyleOptionComboBox",
    "QStyleOptionComplex",
    "QStyleOptionDockWidget",
    "QStyleOptionDockWidgetV2",
    "QStyleOptionFocusRect",
    "QStyleOptionFrame",
    "QStyleOptionFrameV2",
    "QStyleOptionFrameV3",
    "QStyleOptionGraphicsItem",
    "QStyleOptionGroupBox",
    "QStyleOptionHeader",
    "QStyleOptionMenuItem",
    "QStyleOptionProgressBar",
    "QStyleOptionProgressBarV2",
    "QStyleOptionRubberBand",
    "QStyleOptionSizeGrip",
    "QStyleOptionSlider",
    "QStyleOptionSpinBox",
    "QStyleOptionTab",
    "QStyleOptionTabBarBase",
    "QStyleOptionTabBarBaseV2",
    "QStyleOptionTabV2",
    "QStyleOptionTabV3",
    "QStyleOptionTabWidgetFrame",
    "QStyleOptionTabWidgetFrameV2",
    "QStyleOptionTitleBar",
    "QStyleOptionToolBar",
    "QStyleOptionToolBox",
    "QStyleOptionToolBoxV2",
    "QStyleOptionToolButton",
    "QStyleOptionViewItem",
    "QStyleOptionViewItemV2",
    "QStyleOptionViewItemV3",
    "QStyleOptionViewItemV4",
    "QStylePainter",
    "QStylePlugin",
    "QStyledItemDelegate",
    "QSvgRenderer",
    "QSyntaxHighlighter",
    "QSystemTrayIcon",
    "QTabBar",
    "QTabWidget",
    "QTableView",
    "QTableWidget",
    "QTableWidgetItem",
    "QTableWidgetSelectionRange",
    "QTcpServer",
    "QTcpSocket",
    "QTemporaryFile",
    "QTest",
    "QTestBasicStreamer",
    "QTestCoreElement",
    "QTestCoreList",
    "QTestData",
    "QTestDelayEvent",
    "QTestElement",
    "QTestElementAttribute",
    "QTestEvent",
    "QTestEventList",
    "QTestEventLoop",
    "QTestFileLogger",
    "QTestKeyClicksEvent",
    "QTestKeyEvent",
    "QTestMouseEvent",
    "QTestXmlStreamer",
    "QTextBlock",
    "QTextBlockFormat",
    "QTextBlockGroup",
    "QTextBlockUserData",
    "QTextBoundaryFinder",
    "QTextBrowser",
    "QTextCharFormat",
    "QTextCodec",
    "QTextConverter",
    "QTextCursor",
    "QTextDocument",
    "QTextDocumentFragment",
    "QTextDocumentWriter",
    "QTextEdit",
    "QTextFormat",
    "QTextFragment",
    "QTextFrame",
    "QTextFrameFormat",
    "QTextFrameLayoutData",
    "QTextImageFormat",
    "QTextInlineObject",
    "QTextItem",
    "QTextLayout",
    "QTextLength",
    "QTextLine",
    "QTextList",
    "QTextListFormat",
    "QTextObject",
    "QTextObjectInterface",
    "QTextOption",
    "QTextStream",
    "QTextStreamFunction",
    "QTextTable",
    "QTextTableCell",
    "QTextTableCellFormat",
    "QTextTableFormat",
    "QThread",
    "QThreadPool",
    "QTileRules",
    "QTime",
    "QTimeEdit",
    "QTimeLine",
    "QTimer",
    "QTimerEvent",
    "QToolBar",
    "QToolBox",
    "QToolButton",
    "QToolTip",
    "QTransform",
    "QTranslator",
    "QTreeView",
    "QTreeWidget",
    "QTreeWidgetItem",
    "QTreeWidgetItemIterator",
    "QTypeInfo",
    "QUdpSocket",
    "QUiLoader",
    "QUndoCommand",
    "QUndoGroup",
    "QUndoStack",
    "QUndoView",
    "QUnixPrintWidget",
    "QUpdateLaterEvent",
    "QUrl",
    "QVBoxLayout",
    "QValidator",
    "QVariant",
    "QVariantAnimation",
    "QVariantHash",
    "QVariantList",
    "QVariantMap",
    "QVector",
    "QVector2D",
    "QVector3D",
    "QVector4D",
    "QVectorData",
    "QVectorIterator",
    "QVectorTypedData",
    "QWaitCondition",
    "QWeakPointer",
    "QWhatsThis",
    "QWhatsThisClickedEvent",
    "QWheelEvent",
    "QWidget",
    "QWidgetAction",
    "QWidgetData",
    "QWidgetItem",
    "QWidgetList",
    "QWidgetMapper",
    "QWidgetSet",
    "QWindowStateChangeEvent",
    "QWindowsStyle",
    "QWizard",
    "QWizardPage",
    "QX11EmbedContainer",
    "QX11EmbedWidget",
    "QX11Info",
    "QXmlAttributes",
    "QXmlContentHandler",
    "QXmlDTDHandler",
    "QXmlDeclHandler",
    "QXmlDefaultHandler",
    "QXmlEntityResolver",
    "QXmlErrorHandler",
    "QXmlInputSource",
    "QXmlLexicalHandler",
    "QXmlLocator",
    "QXmlNamespaceSupport",
    "QXmlParseException",
    "QXmlReader",
    "QXmlSimpleReader",
    "QXmlStreamAttribute",
    "QXmlStreamAttributes",
    "QXmlStreamEntityDeclaration",
    "QXmlStreamEntityDeclarations",
    "QXmlStreamEntityResolver",
    "QXmlStreamNamespaceDeclaration",
    "QXmlStreamNamespaceDeclarations",
    "QXmlStreamNotationDeclaration",
    "QXmlStreamNotationDeclarations",
    "QXmlStreamReader",
    "QXmlStreamWriter",
    "Q_IPV6ADDR",
    "Q_PID",
    "Qt",
    "QtAlgorithms",
    "QtCleanUpFunction",
    "QtConfig",
    "QtContainerFwd",
    "QtCore",
    "QtDBus",
    "QtDebug",
    "QtDeclarative",
    "QtEndian",
    "QtEvents",
    "QtGlobal",
    "QtGui",
    "QtMsgHandler",
    "QtNetwork",
    "QtPlugin",
    "QtPluginInstanceFunction",
    "QtScript",
    "QtSvg",
    "QtTest",
    "QtTestGui",
    "QtUiTools",
    "QtXml",
]
regex = re.compile('((?:class|struct|template.*) (%s);)' % '|'.join(classlist))

cppfiles = []
for root, dirs, files in os.walk(os.curdir):
    for f in files:
        if f.endswith(('.cpp', '.cc', '.hpp', '.h')):
            cppfiles.append('%s/%s' % (root, f))

for cpp in cppfiles:
    cpp = os.path.realpath(cpp)
    with open(cpp, 'r') as f:
        cppcontent = f.read()
    for match, sclass in regex.findall(cppcontent):
        with open(cpp, 'w') as f:
            print('replacing folrward declaration of %s with inclusion in: %s' % (sclass, cpp))
            cppcontent = cppcontent.replace(match, '#include <%s>' % sclass)
            f.write(cppcontent)
