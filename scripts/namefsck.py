#!/usr/bin/python

import sys, os, re

# generated via find /usr/include/katie/ -name 'Q*' -printf '    "%f",\n' | sort -u
classlist = {
    "QAbstractAnimation",
    "QAbstractButton",
    "QAbstractEventDispatcher",
    "QAbstractExtensionFactory",
    "QAbstractExtensionManager",
    "QAbstractFileEngine",
    "QAbstractFileEngineHandler",
    "QAbstractFileEngineIterator",
    "QAbstractFormBuilder",
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
    "QAbstractTransition",
    "QAccessible",
    "QAccessibleBridge",
    "QAccessibleObject",
    "QAccessiblePlugin",
    "QAccessibleTextInterface",
    "QAccessibleValueInterface",
    "QAccessibleWidget",
    "QAccessibleWidgetEx",
    "QAction",
    "QActionEvent",
    "QActionGroup",
    "QAnimationDriver",
    "QAnimationGroup",
    "QApplication",
    "QArgument",
    "QAtomicInt",
    "QAtomicPointer",
    "QAuthDevice",
    "QAuthenticator",
    "QBasicTimer",
    "QBBSystemLocaleData",
    "QBitArray",
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
    "QCDEStyle",
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
    "QColormap",
    "QColumnView",
    "QComboBox",
    "QCommandLineOption",
    "QCommandLineParser",
    "QCommandLinkButton",
    "QCommonStyle",
    "QCompleter",
    "QConicalGradient",
    "QContextMenuEvent",
    "QContiguousCache",
    "QContiguousCacheData",
    "QContiguousCacheTypedData",
    "QCoreApplication",
    "QCryptographicHash",
    "QCursor",
    "QDataStream",
    "QDataWidgetMapper",
    "QDate",
    "QDateEdit",
    "QDateTime",
    "QDateTimeEdit",
    "QDBusAbstractAdaptor",
    "QDBusAbstractInterface",
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
    "QDBusReply",
    "QDBusServer",
    "QDBusServiceWatcher",
    "QDBusUnixFileDescriptor",
    "QDBusVariant",
    "QDBusVirtualObject",
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
    "QDeclarativeNetworkAccessManagerFactory",
    "QDeclarativeParserStatus",
    "QDeclarativeProperties",
    "QDeclarativeProperty",
    "QDeclarativePropertyMap",
    "QDeclarativePropertyValueInterceptor",
    "QDeclarativePropertyValueSource",
    "QDeclarativeScriptString",
    "QDeclarativeTypeInfo",
    "QDeclarativeView",
    "QDesignerActionEditorInterface",
    "QDesignerBrushManagerInterface",
    "QDesignerComponents",
    "QDesignerContainerExtension",
    "QDesignerCustomWidgetCollectionInterface",
    "QDesignerCustomWidgetInterface",
    "QDesignerDnDItemInterface",
    "QDesignerDynamicPropertySheetExtension",
    "QDesignerExportWidget",
    "QDesignerExtraInfoExtension",
    "QDesignerFormEditorInterface",
    "QDesignerFormEditorPluginInterface",
    "QDesignerFormWindowCursorInterface",
    "QDesignerFormWindowInterface",
    "QDesignerFormWindowManagerInterface",
    "QDesignerFormWindowToolInterface",
    "QDesignerIconCacheInterface",
    "QDesignerIntegrationInterface",
    "QDesignerLanguageExtension",
    "QDesignerLayoutDecorationExtension",
    "QDesignerMemberSheetExtension",
    "QDesignerMetaDataBaseInterface",
    "QDesignerMetaDataBaseItemInterface",
    "QDesignerObjectInspectorInterface",
    "QDesignerPromotionInterface",
    "QDesignerPropertyEditorInterface",
    "QDesignerPropertySheetExtension",
    "QDesignerResourceBrowserInterface",
    "QDesignerTaskMenuExtension",
    "QDesignerWidgetBoxInterface",
    "QDesignerWidgetDataBaseInterface",
    "QDesignerWidgetDataBaseItemInterface",
    "QDesignerWidgetFactoryInterface",
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
    "QDragResponseEvent",
    "QDropEvent",
    "QDynamicPropertyChangeEvent",
    "QEasingCurve",
    "QElapsedTimer",
    "QErrorMessage",
    "QEvent",
    "QEventLoop",
    "QEventTransition",
    "QExplicitlySharedDataPointer",
    "QExtensionFactory",
    "QExtensionManager",
    "QFactoryInterface",
    "QFile",
    "QFileDialog",
    "QFileIconProvider",
    "QFileInfo",
    "QFileInfoList",
    "QFileOpenEvent",
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
    "QFormBuilder",
    "QFormLayout",
    "QFrame",
    "QFSFileEngine",
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
    "QGesture",
    "QGestureRecognizer",
    "QGlobalStatic",
    "QGlobalStaticDeleter",
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
    "QGraphicsSvgItem",
    "QGraphicsTextItem",
    "QGraphicsView",
    "QGraphicsWidget",
    "QGridLayout",
    "QGroupBox",
    "QGuiPlatformPlugin",
    "QHash",
    "QHashData",
    "QHashDummyNode",
    "QHashDummyValue",
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
    "QIconDragEvent",
    "QIconEngine",
    "QIconEngineFactoryInterface",
    "QIconEngineFactoryInterfaceV2",
    "QIconEnginePlugin",
    "QIconEnginePluginV2",
    "QIdentityProxyModel",
    "QImage",
    "QImageIOHandler",
    "QImageIOHandlerFactoryInterface",
    "QImageReader",
    "QImageWriter",
    "QIncompatibleFlag",
    "QInputDialog",
    "QIntegerForSize",
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
    "QJsonArray",
    "QJsonDocument",
    "QJsonObject",
    "QJsonParseError",
    "QJsonValue",
    "QKeyEvent",
    "QKeyEventTransition",
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
    "QMetaEnum",
    "QMetaMethod",
    "QMetaObject",
    "QMetaObjectAccessor",
    "QMetaObjectExtraData",
    "QMetaProperty",
    "QMetaType",
    "QMetaTypeId",
    "QMetaTypeId2",
    "QMimeData",
    "QModelIndex",
    "QModelIndexList",
    "QMotifStyle",
    "QMouseDriverFactory",
    "QMouseDriverPlugin",
    "QMouseEvent",
    "QMouseEventTransition",
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
    "QMutex",
    "QMutexData",
    "QMutexLocker",
    "QMYSQLDriver",
    "QMYSQLResult",
    "QNetworkAccessManager",
    "QNetworkAddressEntry",
    "QNetworkCacheMetaData",
    "QNetworkConfiguration",
    "QNetworkConfigurationManager",
    "QNetworkCookie",
    "QNetworkCookieJar",
    "QNetworkDiskCache",
    "QNetworkInterface",
    "QNetworkProxy",
    "QNetworkProxyFactory",
    "QNetworkProxyQuery",
    "QNetworkReply",
    "QNetworkRequest",
    "QNetworkSession",
    "QNoDebug",
    "QObject",
    "QObjectCleanupHandler",
    "QObjectData",
    "QObjectList",
    "QObjectUserData",
    "QODBCDriver",
    "QODBCResult",
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
    "QPanGesture",
    "QParallelAnimationGroup",
    "QPauseAnimation",
    "QPen",
    "QPersistentModelIndex",
    "QPixmap",
    "QPixmapCache",
    "QPlainTextDocumentLayout",
    "QPlainTextEdit",
    "QPlastiqueStyle",
    "QPlatformCursor",
    "QPlatformCursorImage",
    "QPlatformCursorPrivate",
    "QPlatformEventLoopIntegration",
    "QPlatformFontDatabase",
    "QPlatformIntegration",
    "QPlatformIntegrationFactoryInterface",
    "QPlatformIntegrationPlugin",
    "QPlatformNativeInterface",
    "QPlatformScreen",
    "QPlugin",
    "QPluginLoader",
    "QPoint",
    "QPointer",
    "QPointF",
    "QPolygon",
    "QPolygonF",
    "QPoolEntry",
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
    "QProxyScreen",
    "QProxyScreenCursor",
    "QProxyStyle",
    "QPSQLDriver",
    "QPSQLResult",
    "QPushButton",
    "QQnxMouseHandler",
    "QQnxScreen",
    "QQuaternion",
    "QQueue",
    "QRadialGradient",
    "QRadioButton",
    "QReadLocker",
    "QReadWriteLock",
    "QRect",
    "QRectF",
    "QRegExp",
    "QRegion",
    "QResizeEvent",
    "QResource",
    "QReturnArgument",
    "QRgb",
    "QRubberBand",
    "QRunnable",
    "QScopedArrayPointer",
    "QScopedPointer",
    "QScopedPointerArrayDeleter",
    "QScopedPointerDeleter",
    "QScopedPointerPodDeleter",
    "QScopedValueRollback",
    "QScreen",
    "QScreenCursor",
    "QScreenDriverFactory",
    "QScreenDriverFactoryInterface",
    "QScreenDriverPlugin",
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
    "QSharedMemory",
    "QSharedPointer",
    "QShortcut",
    "QShortcutEvent",
    "QShowEvent",
    "QSignalMapper",
    "QSignalSpy",
    "QSignalTransition",
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
    "QSqlDatabase",
    "QSqlDriver",
    "QSqlDriverCreator",
    "QSqlDriverCreatorBase",
    "QSqlDriverFactoryInterface",
    "QSqlDriverPlugin",
    "QSqlError",
    "QSqlField",
    "QSqlIndex",
    "QSQLiteDriver",
    "QSQLiteResult",
    "QSqlQuery",
    "QSqlQueryModel",
    "QSqlRecord",
    "QSqlRelation",
    "QSqlRelationalDelegate",
    "QSqlRelationalTableModel",
    "QSqlResult",
    "QSqlTableModel",
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
    "QStaticText",
    "QStatusBar",
    "QStatusTipEvent",
    "QStdWString",
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
    "QStyleOptionDockWidget",
    "QStyleOptionDockWidgetV2",
    "QStyleOptionFocusRect",
    "QStyleOptionFrame",
    "QStyleOptionFrameV2",
    "QStyleOptionFrameV3",
    "QStyleOptionGraphicsItem",
    "QStyleOptionGroupBox",
    "QStyleOptionMenuItem",
    "QStyleOptionProgressBar",
    "QStyleOptionProgressBarV2",
    "QStyleOptionSizeGrip",
    "QStyleOptionSlider",
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
    "QSupportedWritingSystems",
    "QSvgGenerator",
    "QSvgRenderer",
    "QSvgWidget",
    "QSwipeGesture",
    "QSyntaxHighlighter",
    "QSystemLocale",
    "QSystemSemaphore",
    "QSystemTrayIcon",
    "Qt",
    "QTabBar",
    "QTableView",
    "QTableWidget",
    "QTableWidgetItem",
    "QTableWidgetSelectionRange",
    "QTabWidget",
    "QtAlgorithms",
    "QTapAndHoldGesture",
    "QTapGesture",
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
    "QtDesigner",
    "QtDesignerComponents",
    "QTemporaryFile",
    "QtEndian",
    "QTest",
    "QTestBasicStreamer",
    "QTestCoreElement",
    "QTestCoreList",
    "QTestData",
    "QTestElement",
    "QTestElementAttribute",
    "QTestEvent",
    "QTestEventLoop",
    "QTestFileLogger",
    "QTestLightXmlStreamer",
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
    "QTextCodecFactoryInterface",
    "QTextCodecPlugin",
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
    "QToolBarChangeEvent",
    "QToolBox",
    "QToolButton",
    "QToolTip",
    "QTouchEvent",
    "QtPlugin",
    "QtPluginInstanceFunction",
    "QTransform",
    "QTransformedScreen",
    "QTranslator",
    "QTransportAuth",
    "QTreeView",
    "QTreeWidget",
    "QTreeWidgetItem",
    "QTreeWidgetItemIterator",
    "QtScript",
    "QtScriptTools",
    "QtSql",
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
    "QVFbHeader",
    "QVFbKeyboardHandler",
    "QVFbKeyData",
    "QVFbMouseHandler",
    "QVFbScreen",
    "QWaitCondition",
    "QWeakPointer",
    "QWhatsThis",
    "QWhatsThisClickedEvent",
    "QWheelEvent",
    "QWidget",
    "QWidgetAction",
    "QWidgetData",
    "QWidgetMapper",
    "QWidgetSet",
    "QWindowsStyle",
    "QWindowStateChangeEvent",
    "QWizard",
    "QWizardPage",
    "QWorkspace",
    "QWriteLocker",
    "QX11EmbedContainer",
    "QX11EmbedWidget",
    "QX11Info",
    "QXmlDeclHandler",
    "QXmlDefaultHandler",
    "QXmlDTDHandler",
    "QXmlEntityResolver",
    "QXmlErrorHandler",
    "QXmlInputSource",
    "QXmlLexicalHandler",
    "QXmlLocator",
    "QXmlNamespaceSupport",
    "QXmlReader",
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
}
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
