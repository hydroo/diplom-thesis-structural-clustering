#include <QApplication>

#include "common/prereqs.hpp"

#include "common/otf/otf.hpp"

/* level is the stack level inside this process.
 * processLevel is the base stack level of this process.
 * maxLevel is used to determine the total height of a process.
 */
void drawRecursively(QTextStream& o, const FunctionCall& call, uint64_t offset, uint64_t span, int level, int processLevel, int *maxLevel, int processIndex) {

	*maxLevel = qMax(*maxLevel, processLevel + level);

	int top = processLevel + level; // integer "number of levels" (times base height in the image)
	double width = (call.end - call.begin) / (double) span; // 0.0 - 1.0, relative width
	double middle = (call.begin - offset) / (double) span + width/2; // 0.0 - 1.0, relative position of the middle of the node

	QString indent = QString("	").repeated(level); // indention resembles stack over time

	o << QString("%1\\node [box, minimum width=\\w * %2, fill=%3] at (\\w * %4, \\h * -%5 + \\p * -%6) () {};\n").arg(indent).arg(width, 0, 'f', 15).arg(call.id).arg(middle, 0, 'f', 15).arg(top).arg(processIndex);

	for (int i = 0; i < call.calls.size(); i += 1) {
		drawRecursively(o, call.calls[i], offset, span, level+1, processLevel, maxLevel, processIndex);
	}

}

void generateTikz(QFile* file, const Trace& trace, const QMap<process_t, QString>& processNames, const QMap<function_t, QString>& functionNames) {

	QTextStream o(file);

	int width = 10; // arbitrary

	/* pre1 */
	o << QString(
		"% Note:\n" \
		"%   * function names are note inserted to avoid latex resizing/overriding rectangles. Can easily be done manually if desired.\n" \
		"%   * customize \\h, \\w and colors as you wish.\n" \
		"%   * height coordinates are always negative. would be flipped vertically otherwise.\n" \
		"%   * each function has its own color. function group based coloring could be implemented if desired.\n"
		"\n" \
		"\\documentclass[]{standalone}\n" \
		"\\usepackage{tikz}\n" \
		"	\\usetikzlibrary{shapes, positioning}\n" \
		"	\\tikzstyle{box} = [draw, rectangle, minimum height=\\h]\n" \
		"\\usepackage{color}\n" \
		"\n" \
		"\\newcommand{\\h}{1cm} % height per rectangle\n" \
		"\\newcommand{\\p}{1cm} % vertical gap from one process to the next\n" \
		"\\newcommand{\\w}{%2cm} % total width\n" \
		"\n").arg(width);

	/* colors (uses hashed function names as a basis) */
	o << "% function colors\n";
	QMapIterator<function_t, QString> i(functionNames);
	while (i.hasNext()) {
		i.next();
		function_t k = i.key();
		QString n = i.value();
		int r = qChecksum((    n).toStdString().c_str(), (    n).length()) % 255;
		int g = qChecksum((  n+n).toStdString().c_str(), (  n+n).length()) % 255;
		int b = qChecksum((n+n+n).toStdString().c_str(), (n+n+n).length()) % 255;
		o << QString("\\definecolor{%1}{RGB}{%2,%3,%4} % function %1 \"%5\"\n").arg(k).arg(r, 3).arg(g, 3).arg(b, 3).arg(n);
	}

	/* pre2 */
	o << "\n" \
		"\\begin{document}\n" \
		"\\begin{tikzpicture}[]\n" \
		"\n";

	/* draw tree */
	int processLevel = 0;
	int maxLevel = 0;
	int processIndex = 0;
	QMapIterator<process_t, QList<FunctionCall>> j(trace);
	while (j.hasNext()) {
		j.next();

		process_t id = j.key();
		auto calls = j.value();

		if (calls.isEmpty()) { continue; } // skip empty processes

		uint64_t beginTime = calls.first().begin;
		uint64_t endTime = calls.last().end;
		uint64_t offset = beginTime;

		o << QString("% process %1 \"%2\"\n").arg(id).arg(processNames[id]);

		for (int i = 0; i < calls.size(); i += 1) {
			drawRecursively(o, calls[i], offset, endTime-beginTime, 0, processLevel, &maxLevel, processIndex);
		}

		o << "\n";

		processLevel = maxLevel + 1;
		processIndex += 1;
	}

	/* post */
	o <<
		"\\end{tikzpicture}\n" \
		"\\end{document}\n";
}

#define HELP_TEXT \
	"trace-to-tikz <tracefilename> <texfilename>\n"

int main(int argc, char** args) {

	QApplication app(argc, args);

	init();

	QString traceFileName;
	QString texFileName;

	/* evaluate arguments */
	for (int i = 1; i < argc; i += 1) {
		QString arg(args[i]);
		if (arg.startsWith("-")) {
			if (arg == "--help" || arg == "-h") {
				qout << HELP_TEXT;
				exit(0);
			} else {
				qout << "unknown parameter \"" << arg << "\". aborting.\n";
				exit(0);
			}
		} else {
			if (traceFileName.isEmpty()) {
				traceFileName = arg;
			} else {
				if (texFileName.isEmpty()) {
					texFileName = arg;
				} else {
					qout << "too many arguments. aborting.\n";
					exit(0);
				}
			}
		}
	}

	if (traceFileName.isEmpty() || texFileName.isEmpty()) {
		qout << HELP_TEXT;
		exit(0);
	}

	QMap<process_t, QString> processNames;
	QMap<function_t, QString> functionNames;
	Trace trace;

	/* read otf */
	Otf_processNames(traceFileName, &processNames);
	Otf_functionNames(traceFileName, &functionNames);
	Otf_trace(traceFileName, &trace);

	/* generate tex */
	QFile file(texFileName);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false) {
		qerr << "cannot open \"" << texFileName << "\".aborting." << endl;
		exit(0);
	}

	generateTikz(&file, trace, processNames, functionNames);

	file.close();

	return 0;
}

