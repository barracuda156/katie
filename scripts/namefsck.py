#!/usr/bin/python

import os, re

# generated via find /usr/include/katie/ -name 'Q*' -printf '    "%f",\n' | sort -u
classlist = [
    "QAbstractAnimation",
    "QAbstractButton",
    "QAbstractEventDispatcher",
    "QAbstractFileEngine",
    "QAbstractGraphicsShapeItem",
    "QAbstractItemDelegate",
    "QAbstractItemModel",
    "QAbstractItemView",
    "QAbstractListModel",
    "QAbstractNetworkCache",
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
    "QAccessible",
    "QAccessible2Interface",
    "QAccessibleActionInterface",
    "QAccessibleApplication",
    "QAccessibleBridge",
    "QAccessibleBridgeFactoryInterface",
    "QAccessibleBridgePlugin",
    "QAccessibleEditableTextInterface",
    "QAccessibleEvent",
    "QAccessibleFactoryInterface",
    "QAccessibleImageInterface",
    "QAccessibleInterface",
    "QAccessibleInterfaceEx",
    "QAccessibleObject",
    "QAccessibleObjectEx",
    "QAccessiblePlugin",
    "QAccessibleSimpleEditableTextInterface",
    "QAccessibleTable2CellInterface",
    "QAccessibleTable2Interface",
    "QAccessibleTableInterface",
    "QAccessibleTextInterface",
    "QAccessibleValueInterface",
    "QAccessibleWidget",
    "QAccessibleWidgetEx",
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
    "QBitmap",
    "QBitRef",
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
    "QDataStream",
    "QDataWidgetMapper",
    "QDate",
    "QDateEdit",
    "QDateTime",
    "QDateTimeEdit",
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
    "QDesktopServices",
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
    "QErrorMessage",
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
    "QFontInfo",
    "QFontMetrics",
    "QFontMetricsF",
    "QFormLayout",
    "QFrame",
    "QFtp",
    "QFuture",
    "QFutureInterface",
    "QFutureInterfaceBase",
    "QFutureIterator",
    "QFutureSynchronizer",
    "QFutureWatcher",
    "QFutureWatcherBase",
    "QGenericArgument",
    "QGenericMatrix",
    "QGenericReturnArgument",
    "QGradient",
    "QGradientStop",
    "QGradientStops",
    "QGraphicsAnchor",
    "QGraphicsAnchorLayout",
    "QGraphicsBlurEffect",
    "QGraphicsColorizeEffect",
    "QGraphicsDropShadowEffect",
    "QGraphicsEffect",
    "QGraphicsEllipseItem",
    "QGraphicsGridLayout",
    "QGraphicsItem",
    "QGraphicsItemAnimation",
    "QGraphicsItemGroup",
    "QGraphicsLayout",
    "QGraphicsLayoutItem",
    "QGraphicsLinearLayout",
    "QGraphicsLineItem",
    "QGraphicsObject",
    "QGraphicsOpacityEffect",
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
    "QHash",
    "QHashData",
    "QHashIterator",
    "QHashNode",
    "QHBoxLayout",
    "QHeaderView",
    "QHelpEvent",
    "QHideEvent",
    "QHostAddress",
    "QHostInfo",
    "QHoverEvent",
    "QHttp",
    "QHttpHeader",
    "QHttpMultiPart",
    "QHttpPart",
    "QHttpRequestHeader",
    "QHttpResponseHeader",
    "QIcon",
    "QIconEngine",
    "QIconEngineFactoryInterface",
    "QIconEngineFactoryInterfaceV2",
    "QIconEnginePlugin",
    "QIconEnginePluginV2",
    "QIconEngineV2",
    "QIdentityProxyModel",
    "QImage",
    "QImageIOHandler",
    "QImageIOHandlerFactoryInterface",
    "QImageIOPlugin",
    "QImageReader",
    "QImageWriter",
    "QIncompatibleFlag",
    "QInputDialog",
    "QInputEvent",
    "QInternal",
    "QIntValidator",
    "QIODevice",
    "Q_IPV6ADDR",
    "QIPv6Address",
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
    "QLabel",
    "QLatin1Char",
    "QLatin1String",
    "QLayout",
    "QLayoutItem",
    "QLCDNumber",
    "QLibrary",
    "QLibraryInfo",
    "QLine",
    "QLinearGradient",
    "QLineEdit",
    "QLineF",
    "QLinkedList",
    "QLinkedListData",
    "QLinkedListIterator",
    "QLinkedListNode",
    "QList",
    "QListData",
    "QListIterator",
    "QListView",
    "QListWidget",
    "QListWidgetItem",
    "QLocale",
    "QLocalServer",
    "QLocalSocket",
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
    "QMdiArea",
    "QMdiSubWindow",
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
    "QMotifStyle",
    "QMouseEvent",
    "QMoveEvent",
    "QMovie",
    "QMultiHash",
    "QMultiMap",
    "QMutableFutureIterator",
    "QMutableHashIterator",
    "QMutableLinkedListIterator",
    "QMutableListIterator",
    "QMutableMapIterator",
    "QMutableSetIterator",
    "QMutableStringListIterator",
    "QMutableVectorIterator",
    "QMutex",
    "QMutexLocker",
    "QNetworkAccessManager",
    "QNetworkAddressEntry",
    "QNetworkCacheMetaData",
    "QNetworkCookie",
    "QNetworkCookieJar",
    "QNetworkDiskCache",
    "QNetworkInterface",
    "QNetworkReply",
    "QNetworkRequest",
    "QObject",
    "QObjectCleanupHandler",
    "QObjectData",
    "QObjectList",
    "QPageSetupDialog",
    "QPaintDevice",
    "QPaintEngine",
    "QPaintEngineState",
    "QPainter",
    "QPainterPath",
    "QPainterPathPrivate",
    "QPainterPathStroker",
    "QPaintEvent",
    "QPair",
    "QPalette",
    "QParallelAnimationGroup",
    "QPauseAnimation",
    "QPen",
    "QPersistentModelIndex",
    "Q_PID",
    "QPixmap",
    "QPixmapCache",
    "QPlainTextDocumentLayout",
    "QPlainTextEdit",
    "QPlastiqueStyle",
    "QPlugin",
    "QPluginLoader",
    "QPoint",
    "QPointer",
    "QPointF",
    "QPolygon",
    "QPolygonF",
    "QPrintDialog",
    "QPrintEngine",
    "QPrinter",
    "QPrinterInfo",
    "QPrintPreviewDialog",
    "QPrintPreviewWidget",
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
    "QScriptable",
    "QScriptClass",
    "QScriptClassPropertyIterator",
    "QScriptContext",
    "QScriptContextInfo",
    "QScriptContextInfoList",
    "QScriptEngine",
    "QScriptEngineAgent",
    "QScriptEngineDebugger",
    "QScriptExtensionInterface",
    "QScriptExtensionPlugin",
    "QScriptProgram",
    "QScriptString",
    "QScriptSyntaxCheckResult",
    "QScriptValue",
    "QScriptValueIterator",
    "QScriptValueList",
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
    "QSsl",
    "QSslCertificate",
    "QSslCipher",
    "QSslConfiguration",
    "QSslError",
    "QSslKey",
    "QSslSocket",
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
    "QStyledItemDelegate",
    "QStyleFactory",
    "QStyleFactoryInterface",
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
    "QSvgRenderer",
    "QSyntaxHighlighter",
    "QSystemTrayIcon",
    "Qt",
    "QTabBar",
    "QTableView",
    "QTableWidget",
    "QTableWidgetItem",
    "QTableWidgetSelectionRange",
    "QTabWidget",
    "QtAlgorithms",
    "QtCleanUpFunction",
    "QtConcurrentFilter",
    "QtConcurrentMap",
    "QtConcurrentRun",
    "QtConfig",
    "QtContainerFwd",
    "QtCore",
    "QTcpServer",
    "QTcpSocket",
    "QtDBus",
    "QtDebug",
    "QtDeclarative",
    "QTemporaryFile",
    "QtEndian",
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
    "QTestLightXmlStreamer",
    "QTestMouseEvent",
    "QTestXmlStreamer",
    "QTestXunitStreamer",
    "QtEvents",
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
    "QTextDecoder",
    "QTextDocument",
    "QTextDocumentFragment",
    "QTextDocumentWriter",
    "QTextEdit",
    "QTextEncoder",
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
    "QTextStreamManipulator",
    "QTextTable",
    "QTextTableCell",
    "QTextTableCellFormat",
    "QTextTableFormat",
    "QtGlobal",
    "QtGui",
    "QThread",
    "QThreadPool",
    "QTileRules",
    "QTime",
    "QTimeEdit",
    "QTimeLine",
    "QTimer",
    "QTimerEvent",
    "QtMsgHandler",
    "QtNetwork",
    "QToolBar",
    "QToolBox",
    "QToolButton",
    "QToolTip",
    "QtPlugin",
    "QtPluginInstanceFunction",
    "QTransform",
    "QTranslator",
    "QTreeView",
    "QTreeWidget",
    "QTreeWidgetItem",
    "QTreeWidgetItemIterator",
    "QtScript",
    "QtScriptTools",
    "QtSvg",
    "QtTest",
    "QtTestGui",
    "QtUiTools",
    "QtXml",
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
    "QUrlInfo",
    "QUuid",
    "QValidator",
    "QVariant",
    "QVariantAnimation",
    "QVariantHash",
    "QVariantList",
    "QVariantMap",
    "QVarLengthArray",
    "QVBoxLayout",
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
    "QWidgetItemV2",
    "QWidgetList",
    "QWidgetMapper",
    "QWidgetSet",
    "QWindowsStyle",
    "QWindowStateChangeEvent",
    "QWizard",
    "QWizardPage",
    "QWorkspace",
    "QX11EmbedContainer",
    "QX11EmbedWidget",
    "QX11Info",
    "QXmlAttributes",
    "QXmlContentHandler",
    "QXmlDeclHandler",
    "QXmlDefaultHandler",
    "QXmlDTDHandler",
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
