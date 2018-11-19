IMAGES = \
	images/bg-comp-vampir.png \
	images/bg-meas-trace.png \
	images/bg-vis-vampir.png \
	images/id-boxplot.pdf \
	images/id-call-matrix.png \
	images/id-call-tree.png \
	images/id-exclusive-inclusive.pdf \
	images/id-m-color-coded-boxplot.pdf \
	images/id-m-cs-1.png \
	images/id-m-cs-2.png \
	images/id-m-gradient.png \
	images/id-m-lp-1.png \
	images/id-m-lp-2.png \
	images/id-m-lp-3.png \
	images/id-m-pp-1-1.png \
	images/id-m-pp-1-2.png \
	images/id-m-pp-1-3.png \
	images/id-m-pp-4-1.png \
	images/id-m-pp-4-2.png \
	images/id-p-4-1.png \
	images/id-p-4-2.png \
	images/id-p-4-3.png \
	images/id-p-64-1.png \
	images/id-p-64-2.png \
	images/id-p-64-3.png \
	images/id-profile.png \
	images/tu-logo.png

BIBLIOGRAPHY_SOURCE = bibliography.bib

REPORT_SOURCE = report.tex
REPORT_AUX = report.aux
REPORT_TARGET = report.pdf
ZIH_TEMPLATE = zih-template.cls

LATEX_OUTPUT_FILTER = \
	grep -v OUTPUT_FILTERING_BEGINS_HERE | \
	sed 'h;:b;$b;N;/Package auxhook Warning: Cannot patch \\document,/{N;N;d};$b;P;D' | \
	sed 'h;:b;$b;N;/Package hyperref Message: Driver (autodetected): hpdftex./{N;d};$b;P;D' | \
	sed 'h;:b;$b;/Document Class: scrartcl 2012\/07\/29 v3.11b KOMA-Script document class (article)/{N;d};$b;P;D' | \
	sed 'h;:b;$b;/Package scrlfile, 2012\/06\/15 v3.12 KOMA-Script package (loading files)/{N;N;d};$b;P;D' | \
	sed 'h;:b;$b;/Package typearea, 2012\/07\/29 v3.11b KOMA-Script package (type area)/{N;N;N;d};$b;P;D'
	

.IGNORE: $(REPORT_TARGET)
$(REPORT_TARGET): $(REPORT_SOURCE) $(BIBLIOGRAPHY_SOURCE) $(ZIH_TEMPLATE) $(IMAGES)
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
	pdflatex -shell-escape -interaction=nonstopmode $(REPORT_SOURCE) | $(LATEX_OUTPUT_FILTER)

all: $(REPORT_TARGET)

.PHONY: clean
clean:
	@rm -f *.out *.aux *.log report.pdf *.bbl *.blg *.dvi *.toc *.lof *.lot
	
