#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include "call-list/call-list.hpp"
#include "call-matrix/call-matrix.hpp"
#include "call-tree/call-tree.hpp"
#include "common/prereqs.hpp"

class QToolButton;
class QUndoStack;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(const QString& traceFileName, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames, const QMap<functiongroup_t, QString>& functionGroupNames, const QMap<function_t, functiongroup_t>& functionToFunctionGroup, const QMap<functiongroup_t, QSet<function_t>>& functionGroupToFunction, const CallList& accumulatedCallList, const CallMatrix& accumulatedCallMatrix, const CallTree& accumulatedCallTree, const QMap<process_t, CallList>& callLists, const QMap<process_t, CallMatrix>& callMatrices, const QMap<process_t, CallTree>& callTrees, QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
	virtual ~MainWindow();

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void reset();

private:
	void createActions();
	void createMenus();
	void createToolsBars();
	void createStatusBar();

	void readSettings();
	void writeSettings();

private:
	const QMap<process_t, QString>& processNames;
	const QMap<function_t, QString>& functionNames;
	const QMap<functiongroup_t, QString>& functionGroupNames;
	const QMap<function_t, functiongroup_t>& functionToFunctionGroup;
	const QMap<functiongroup_t, QSet<function_t>>& functionGroupToFunction;

	const CallList& accumulatedCallList;
	const CallMatrix& accumulatedCallMatrix;
	const CallTree& accumulatedCallTree;

	const QMap<process_t, CallList>& callLists;
	const QMap<process_t, CallMatrix>& callMatrices;
	const QMap<process_t, CallTree>& callTrees;

	QAction* quitAction;
	QAction* resetAction;
	QAction* undoAction;
	QAction* redoAction;

	QMenu* fileMenu;
	QMenu* undoMenu;
	QMenu* redoMenu;

	QToolBar* mainToolBar;
	QToolButton* undoToolButton;
	QToolButton* redoToolButton;

	QUndoStack* undoStack;
};

#endif // MAINWINDOW_HPP
