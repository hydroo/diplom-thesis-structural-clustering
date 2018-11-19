#include "profile/mainwindow.hpp"

#include <QAction>
#include <QCloseEvent>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QUndoStack>

MainWindow::MainWindow(const QString& traceFileName, const QMap<process_t, QString>& pn, const QMap<function_t, QString>& fn, const QMap<functiongroup_t, QString>& gn, const QMap<function_t, functiongroup_t>& f2g, const QMap<functiongroup_t, QSet<function_t>>& g2f, const CallList& acl, const CallMatrix& acm, const CallTree& act, const QMap<process_t, CallList>& cls, const QMap<process_t, CallMatrix>& cms, const QMap<process_t, CallTree>& cts, QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags), processNames(pn), functionNames(fn), functionGroupNames(gn), functionToFunctionGroup(f2g), functionGroupToFunction(g2f), accumulatedCallList(acl), accumulatedCallMatrix(acm), accumulatedCallTree(act), callLists(cls), callMatrices(cms), callTrees(cts) {

	setWindowTitle("profile - " + QFileInfo(traceFileName).absoluteFilePath());

	undoStack = new QUndoStack(this);

	createActions();
	createMenus();
	createToolsBars();
	createStatusBar();

	readSettings();
}

MainWindow::~MainWindow() {
}

void MainWindow::closeEvent(QCloseEvent* event) {
	writeSettings();
	event->accept();
}

void MainWindow::reset() {
	// TODO
	resetAction->setEnabled(false);
}

void MainWindow::createActions() {
	quitAction = new QAction("&Quit", this);
	quitAction->setShortcuts(QKeySequence::Quit);
	quitAction->setIcon(QIcon::fromTheme("application-exit"));
	connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

	resetAction = new QAction("Reset", this);
	resetAction->setIcon(QIcon::fromTheme("system-reboot"));
	resetAction->setEnabled(false);
	connect(resetAction, SIGNAL(triggered()), this, SLOT(reset()));

	undoAction = undoStack->createUndoAction(this, "Undo");
	undoAction->setIcon(QIcon::fromTheme("edit-undo"));
	undoAction->setShortcuts(QKeySequence::Undo);

	redoAction = undoStack->createRedoAction(this, "Redo");
	redoAction->setIcon(QIcon::fromTheme("edit-redo"));
	redoAction->setShortcuts(QKeySequence::Redo);
}

void MainWindow::createMenus() {
	fileMenu = menuBar()->addMenu("&File");
	fileMenu->addAction(quitAction);

	undoMenu = new QMenu(this);
	redoMenu = new QMenu(this);
}

void MainWindow::createToolsBars() {
	mainToolBar = addToolBar("Main");
	mainToolBar->setObjectName("MainToolBar");

	mainToolBar->addAction(resetAction);

	mainToolBar->addSeparator();

	mainToolBar->addAction(undoAction);
	undoToolButton = new QToolButton(this);
	undoToolButton->setFixedWidth(10);
	undoToolButton->setEnabled(false);
	undoToolButton->setMenu(undoMenu);
	undoToolButton->setPopupMode(QToolButton::InstantPopup);
	mainToolBar->addWidget(undoToolButton);

	mainToolBar->addAction(redoAction);
	redoToolButton = new QToolButton(this);
	redoToolButton->setFixedWidth(10);
	redoToolButton->setEnabled(false);
	redoToolButton->setMenu(redoMenu);
	redoToolButton->setPopupMode(QToolButton::InstantPopup);
	mainToolBar->addWidget(redoToolButton);
}

void MainWindow::createStatusBar() {
	statusBar()->showMessage("Ready");
}

void MainWindow::readSettings() {
	QSettings s("automaton2000", "profile");
	restoreGeometry(s.value("mainWindowGeometry").toByteArray());
	restoreState(s.value("mainWindowState").toByteArray());
}

void MainWindow::writeSettings() {
	QSettings s("automaton2000", "profile");
	s.setValue("mainWindowGeometry", saveGeometry());
	s.setValue("mainWindowState", saveState());
}
