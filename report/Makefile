IMAGES = \
	images/allinea-map.png \
	images/allinea-map2.png \
	images/box-plot-profile-mockup.png \
	images/clustering-in-vampir.png \
	images/fd4-4096p-new-clustering.png \
	images/fd4-4096p-old-clustering.png \
	images/gromacs-call-tree.png \
	images/gromacs-process-timeline.png \
	images/hpctoolkit.png \
	images/more-processes-than-pixels-vertically-vampir-screenshot.png \
	images/pi-ring-100p-hybrid-new-clustering.png \
	images/pi-ring-100p-hybrid-old-clustering.png \
	images/pi-ring-100p-hybrid-process-summary.png \
	images/profile-displays-in-vampir.png \
	images/taurus.png \
	images/trace-example-screenshot.png \
	images/tu-logo.png \
	images/vampir-example-screenshot.png \
	images/wrf-hybrid-16p-user-presentable-concept-lattice.png

TIKZ = \
	tikz/amg2006-512p-concept-lattice.tex \
	tikz/amg2006-group-counts.tex \
	tikz/amg2006-lattice-timing.tex \
	tikz/amg2006-node-counts.tex \
	tikz/bounded-lattice-example.tex \
	tikz/box-plot.tex \
	tikz/bt-16p-concept-lattice.tex \
	tikz/bt-16p-transitive-concept-lattice.tex \
	tikz/ccg-counter-example.tex \
	tikz/ccg-example-1.tex \
	tikz/ccg-example-2.tex \
	tikz/concept-lattice-example-redundant.tex \
	tikz/concept-lattice-example-not-redundant.tex \
	tikz/example-call-tree.tex \
	tikz/exclusive-inclusive-execution-time.tex \
	tikz/paradis-group-counts.tex \
	tikz/paradis-l-group-counts.tex \
	tikz/paradis-l-lattice-timing.tex \
	tikz/paradis-l-node-counts.tex \
	tikz/paradis-l-non-empty-node-counts.tex \
	tikz/paradis-lattice-timing-1.tex \
	tikz/paradis-lattice-timing-2.tex \
	tikz/paradis-node-counts.tex \
	tikz/paradis-non-empty-node-counts.tex \
	tikz/partial-order-example.tex \
	tikz/subsumption-example-call-stack.tex \
	tikz/subsumption-example-concept-lattice.tex \
	tikz/subsumption-example-transitive-concept-lattice.tex \
	tikz/total-order-example.tex \
	tikz/wrf-64p-concept-lattice.tex \
	tikz/wrf-hybrid-16p-concept-lattice.tex \
	tikz/wrf-hybrid-16p-transitive-concept-lattice.tex

BIBLIOGRAPHY_SOURCE = bibliography.bib

REPORT_SOURCE = report.tex
REPORT_AUX = report.aux
REPORT_TARGET = report.pdf
ZIH_TEMPLATE = zih-template.cls

LATEX_OUTPUT_FILTER = \
	grep -v OUTPUT_FILTERING_BEGINS_HERE | \
	sed 'h;:b;$b;N;/This is pdfTeX.*/{N;N;d};$b;P;D' | \
	sed 'h;:b;$b;N;/LaTeX2e <20.*/{N;N;d};$b;P;D' | \
	sed 'h;:b;$b;N;/Document Class: scrreprt 20.*/{N;N;d};$b;P;D' | \
	sed 'h;:b;$b;N;/For additional information on amsmath.*/{N;N;d};$b;P;D' | \
	sed 'h;:b;$b;N;/Package auxhook Warning: Cannot patch \\document,/{N;N;d};$b;P;D' | \
	sed 'h;:b;$b;N;/Package hyperref Message: Driver (autodetected): hpdftex./{N;d};$b;P;D' | \
	sed 'h;:b;$b;/Document Class: scrartcl 2012\/07\/29 v3.11b KOMA-Script document class (article)/{N;d};$b;P;D' | \
	sed 'h;:b;$b;/Package scrlfile, 2013\/12\/19 v3.12 KOMA-Script package (loading files)/{N;N;d};$b;P;D' | \
	sed 'h;:b;$b;/Package typearea, 2013\/12\/19 v3.12 KOMA-Script package (type area)/{N;N;N;d};$b;P;D' | \
	\
	sed '/^Underfull.*hbox.*badness.*10000.*at lines 12--16$$/{$$!{N;N;s/.*//gi;}}' | \
	sed '/^Underfull.*hbox.*badness.*10000.*at lines 212--216$$/{$$!{N;N;s/.*//gi;}}' | \
	sed '/^Underfull.*hbox.*badness.*10000.*at lines 448--452$$/{$$!{N;N;s/.*//gi;}}' | \
	sed '/^Underfull.*hbox.*badness.*5374.*at lines 499--503$$/{$$!{N;N;N;s/.*//gi;}}' | \
	sed '/^Underfull.*hbox.*badness.*1077.*at lines 505--509$$/{$$!{N;N;s/.*//gi;}}'
	#  12 allinea user guide
	# 212 hyperthreading
	# 441 turboboost
	# 492 xeon e5 2690
	# 498 xeon phi

RED=`echo '\033[0;31m'`
YELLOW=`echo '\033[0;33m'`
BLUE=`echo '\033[0;34m'`
NOCOLOR=`echo '\033[m'`
BOLD=`echo '\033[1m'`
NORMAL=`echo '\033[0m'`

COLORIZE = \
	sed "s/Overfull/${RED}${BOLD}&${NORMAL}${NOCOLOR}/gi" | \
	sed "s/Underfull/${YELLOW}${BOLD}&${NORMAL}${NOCOLOR}/gi" | \
	sed "s/[0-9]*--[0-9]*/${BLUE}${BOLD}&${NORMAL}${NOCOLOR}/gi"

.IGNORE: $(REPORT_TARGET)
$(REPORT_TARGET): $(REPORT_SOURCE) $(BIBLIOGRAPHY_SOURCE) $(ZIH_TEMPLATE) $(IMAGES) $(TIKZ)
	@echo ">>> PRERUN PDFLATEX <<<"
	@echo ""
	pdflatex -shell-escape -interaction=nonstopmode $(REPORT_SOURCE) 1> /dev/null 2> /dev/null
	@echo ""
	@echo ">>> BIBTEX <<<"
	@echo ""
	bibtex $(REPORT_AUX)
	@echo ""
	@echo ">>> PRERUN PDFLATEX <<<"
	@echo ""
	pdflatex -shell-escape -interaction=nonstopmode $(REPORT_SOURCE) 1> /dev/null 2> /dev/null
	@echo ""
	@echo ">>> PDFLATEX <<<"
	@echo ""
	pdflatex -shell-escape -interaction=nonstopmode $(REPORT_SOURCE) | $(LATEX_OUTPUT_FILTER) | $(COLORIZE)

all: $(REPORT_TARGET)

.PHONY: clean
clean:
	@rm -f *.out *.aux *.log report.pdf *.bbl *.blg *.dvi *.toc *.lof *.lot
	
