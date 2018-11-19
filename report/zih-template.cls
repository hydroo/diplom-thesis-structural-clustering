\ProvidesClass{zih-template}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% packages

\RequirePackage{fancyhdr}
\RequirePackage[T1]{fontenc}
\RequirePackage[utf8]{inputenc}
\RequirePackage{setspace}
\usepackage{lmodern} % times is ugly, therefore we replace it with computer modern

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% options

\newif\if@proseminar
\@proseminarfalse
\DeclareOption{proseminar}{\@proseminartrue}

\newif\if@hauptseminar
\@hauptseminarfalse
\DeclareOption{hauptseminar}{\@hauptseminartrue}

% grosser beleg
\newif\if@beleg
\@belegfalse
\DeclareOption{beleg}{\@belegtrue}

% dipl inf
\newif\if@diplominf
\@diplominffalse
\DeclareOption{diplominf}{\@diplominftrue}

% bach inf
\newif\if@bachinf
\@bachinffalse
\DeclareOption{bachinf}{\@bachinftrue}

% mast inf
\newif\if@mastinf
\@mastinffalse
\DeclareOption{mastinf}{\@mastinftrue}

% diss rer nat
\newif\if@dissrernat
\@dissrernatfalse
\DeclareOption{dissrernat}{\@dissrernattrue}

% diss ing
\newif\if@dissing
\@dissingfalse
\DeclareOption{dissing}{\@dissingtrue}

% only for dissertations. no task description
\newif\if@kurz
\@kurzfalse
\DeclareOption{kurz}{\@kurztrue}

% female title
\newif\if@female
\@femalefalse
\DeclareOption{female}{\@femaletrue}

\newif\if@plainarticle
\@plainarticlefalse
\DeclareOption{plainarticle}{\@plainarticletrue}

\newif\if@plainreport
\@plainreportfalse
\DeclareOption{plainreport}{\@plainreporttrue}

% place zih instead chair of computer architecture on title page
\newif\if@zihtitle
\@zihtitlefalse
\DeclareOption{zihtitle}{\@zihtitletrue}

% list of figures
\newif\if@lof
\@loffalse
\DeclareOption{lof}{\@loftrue}

% list of tables
\newif\if@lot
\@lotfalse
\DeclareOption{lot}{\@lottrue}

% nomenclature/abbreviations
\newif\if@nomencl
\@nomenclfalse
\DeclareOption{nomencl}{\@nomencltrue}

% german
\newif\if@german
\@germanfalse
\DeclareOption{german}{\@germantrue}

% enable zih report formatting
\newif\if@final
\@finalfalse
\DeclareOption{final}{\@finaltrue}

% no table of content
\newif\if@notoc
\@notocfalse
\DeclareOption{notoc}{\@notoctrue}

% no task description
\newif\if@noproblem
\@noproblemfalse
\DeclareOption{noproblem}{\@noproblemtrue}

% twosides if not 'final'
\newif\if@twoside
\@twosidefalse
\DeclareOption{twoside}{\@twosidetrue}

\ProcessOptions

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% user definable variables

\newcommand\abstractx[1]{\def\@abstractx{#1}}
\newcommand\acknowledgments[1]{\def\@acknowledgments{#1}}
\newcommand\betreuer[1]{\def\@betreuer{#1}}
\newcommand\bibfiles[1]{\def\@bibfiles{#1}} % where are the bibtex entries
\newcommand\birthday[1]{\def\@birthday{#1}}
\newcommand\birthmonth[1]{\def\@birthmonth{#1}}
\newcommand\birthyear[1]{\def\@birthyear{#1}}
\newcommand\birthplace[1]{\def\@birthplace{#1}}
\newcommand\copyrighterklaerung[1]{\def\@copyrightinformation{#1}}
\newcommand\gutachter[1]{\def\@gutachter{#1}}
\newcommand\matno[1]{\def\@matno{#1}}
\newcommand\pageheadertitle[1]{\def\@pageheadertitle{#1}} % redefine if title too long for page header
\newcommand\taskdescription[1]{\def\@taskdescription{#1}}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% set variables to @empty

\def\@abstractx{}
\def\@acknowledgments{}
\def\@bibfiles{}
\def\@copyrightinformation{}
\def\@isreport{}
\def\@isplain{}
\def\@isdiploma{}
\def\@isdiss{}
\def\@pageheadertitle{\@title} % default is \title
\def\@taskdescription{}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% set document type

\if@beleg
	\def\@isreport{1}
\fi
\if@diplominf
	\def\@isreport{1}
	\def\@isdiploma{1}
\fi
\if@mastinf
	\def\@isreport{1}
	\def\@isdiploma{1}
\fi
\if@bachinf
	\def\@isreport{1}
	\def\@isdiploma{1}
\fi
\if@dissrernat
	\if@kurz
	\else
		\def\@isreport{1}
	\fi
	\def\@isdiss{1}
\fi
\if@dissing
	\if@kurz
	\else
		\def\@isreport{1}
	\fi
	\def\@isdiss{1}
\fi
\if@plainreport
	\def\@isreport{1}
	\def\@isplain{1}
\fi
\if@plainarticle
	\def\@isplain{1}
\fi

\ifx\@empty\@isreport
	\if@final
		\LoadClass[cleardoublepage=empty, listof=totoc, bibliography=totoc, index=totoc, 11pt, abstracton, numbers=noenddot, twoside,titlepage,headings=normal]{scrartcl}
	\else
		\if@twoside
			\LoadClass[cleardoublepage=empty, listof=totoc, bibliography=totoc, index=totoc, 11pt, abstracton, numbers=noenddot, twoside,titlepage,headings=normal]{scrartcl}
		\else
			\LoadClass[cleardoublepage=empty, listof=totoc, bibliography=totoc, index=totoc, 11pt, abstracton, numbers=noenddot, oneside,titlepage,headings=normal]{scrartcl}
		\fi
	\fi
\else
	\if@final
		\LoadClass[cleardoublepage=empty, listof=totoc, bibliography=totoc, index=totoc, 11pt, abstracton, numbers=noenddot, twoside,titlepage,openright,headings=normal]{scrreprt}
	\else
		\if@twoside
			\LoadClass[cleardoublepage=empty, listof=totoc, bibliography=totoc, index=totoc, 11pt, abstracton, numbers=noenddot, twoside,titlepage,openright,headings=normal]{scrreprt}
		\else
			\LoadClass[cleardoublepage=empty, listof=totoc, bibliography=totoc, index=totoc, 11pt, abstracton, numbers=noenddot, oneside,titlepage,openright,headings=normal]{scrreprt}
		\fi
	\fi
\fi

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% set up title page

% defaults
\newcommand{\titleheader}[0]{$\quad$}
\newcommand{\copyrightinfo}[0]{ }

% title header
\if@proseminar
	\renewcommand{\titleheader}[0]{Proseminar ``Rechnerarchitektur''}
\fi
\if@hauptseminar
	\renewcommand{\titleheader}[0]{Hauptseminar\\ Rechnerarchitektur und Programmierung}
	% original: resulted in a line break, therefore I skipped the quotes
	% \renewcommand{\titleheader}[0]{Hauptseminar\\ ``Rechnerarchitektur und Programmierung''}
\fi
\if@beleg
	\renewcommand{\titleheader}[0]{Gro{\ss}er Beleg}
\fi
\if@diplominf%
	\if@german
		\renewcommand{\titleheader}[0]{Diplomarbeit \\[1cm]\large zur Erlangung des akademischen Grades\\ Diplom-Informatiker\@female}
	\else
		\renewcommand{\titleheader}[0]{Diploma Thesis \\[0.5cm]\large submitted in partial fulfilment of the requirements \\ for the degree of Diplom-Informatiker}
	\fi
\fi
\if@bachinf%
\renewcommand{\titleheader}[0]{Bachelor-Arbeit \\[1cm]\large zur Erlangung des akademischen Grades\\ Bachelor of Science}
\fi
\if@mastinf%
\renewcommand{\titleheader}[0]{Master-Arbeit \\[1cm]\large zur Erlangung des akademischen Grades\\ Master of Science}
\fi
\if@dissrernat%
	\if@kurz
		\renewcommand{\titleheader}[0]{\begin{large}\textbf{Kurzfassung}\\[1cm]zur Erlangung des akademischen Grades Doktor rerum naturalium (Dr.\ rer.\ nat.)\end{large}}
	\else
		\renewcommand{\titleheader}[0]{\begin{large}\textbf{Dissertation}\\[1cm]zur Erlangung des akademischen Grades Doktor rerum naturalium (Dr.\ rer.\ nat.)\end{large}}
	\fi
\fi
\if@dissing%
	\if@kurz
		\renewcommand{\titleheader}[0]{\begin{large}\textbf{Kurzfassung}\\[1cm]zur Erlangung des akademischen Grades Doktoringenieur (Dr.-Ing.)\end{large}}
	\else
		\renewcommand{\titleheader}[0]{\begin{large}\textbf{Dissertation}\\[1cm]zur Erlangung des akademischen Grades Doktoringenieur (Dr.-Ing.)\end{large}}
	\fi
\fi

% female
\if@female
	\def\@female{in}
\else
	\def\@female{}
\fi

% doctype
\ifx\@empty\@isdiploma
\else
	\if@bachinf
		\def\@doctype{Bachelor-Arbeit}
	\else
		\if@mastinf
			\def\@doctype{Master-Arbeit}
		\else
			\def\@doctype{Diplomarbeit}
		\fi
	\fi
\fi

% institution
\if@zihtitle
	\newcommand{\@institution}{
		\begin{center}
			\huge
			\textsc{Technische Universit\"{a}t Dresden}

			\bigskip
			\Large
			\textsc{Center for Information Services\\
			\& High Performance Computing\\
			Prof. Dr. Wolfgang E. Nagel\\
			$\quad$\\
			$\quad$}
		\end{center}
	}
\else
	\newcommand{\@institution}{ % customized for my hauptseminar
		\begin{center}
			\includegraphics[width=3cm]{images/tu-logo}~\\[1cm]
			\textsc{\LARGE Technische Universit\"at Dresden}\\[0.5cm]
			\textsc{\Large Department of Computer Science}\\[0.2cm]
			\textsc{\large Institute of Computer Engineering}\\[0.2cm]
			\textsc{\large Chair of Computer Architecture}
		\end{center}
	}
	% \newcommand{\@institution}{ % original
	% 	\begin{center}
	% 		\huge
	% 		\textsc{Technische Universit\"{a}t Dresden}

	% 		\bigskip
	% 		\Large
	% 		\textsc{Center for Information Services\\
	% 		\& High Performance Computing\\
	% 		Prof. Dr. Wolfgang E. Nagel\\
	% 		$\quad$\\
	% 		$\quad$}
	% 	\end{center}
	% }
\fi

% line below author
\ifx\@empty\@isdiss
	\ifx\@empty\@isdiploma
		\ifx\@empty\@isplain
			\def\@belowauthor{(Mat.-No.: \@matno)}
		\else
			\def\@belowauthor{$\quad$}
		\fi
	\else
		\def\@belowauthor{Born~on~the~\ordinalnum{\@birthday}~\monthname[\@birthmonth],~\@birthyear~in~\@birthplace}
	\fi
\else
	\def\@belowauthor{Born~on~the~\ordinalnum{\@birthday}~\monthname[\@birthmonth],~\@birthyear~in~\@birthplace}
\fi

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% page layout

%\setlength{\hoffset}{-1in}
%\addtolength{\hoffset}{2cm}
%\setlength{\voffset}{-1in}
%\addtolength{\voffset}{2cm}
%\addtolength{\voffset}{-\baselineskip}
%\setlength{\topmargin}{0mm}
%\setlength{\headheight}{2.0\baselineskip}
%\setlength{\headsep}{\baselineskip}
\setlength{\textheight}{237mm} % a4 margin vert 30mm
\setlength{\textwidth}{150mm} % a4 margin hor 30mm
\setlength{\evensidemargin}{5mm}
\setlength{\oddsidemargin}{5mm}
%\setlength{\marginparsep}{0mm}
%\setlength{\marginparwidth}{0mm}
%\setlength{\marginparpush}{0mm}
%\setlength{\footskip}{\baselineskip}
%\setlength{\parindent}{0mm}
%\setlength{\parskip}{2mm}
%\setlength{\partopsep}{0mm}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% copyright

\renewcommand\copyrightinfo[0]{
	\cleardoublepage
	\ifx\@empty\@isreport
		\section*{Copyright Information}
	\else
		\chapter*{Copyright Information}
	\fi
	\@copyrightinformation}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% create nomenclature/abbreviations

\if@nomencl
	\RequirePackage[intoc]{nomencl}
	\def\@nomenclmargin{3cm}
	\newcommand\setnomenclmargin[1]{\def\@nomenclmargin{#1}}
	\makenomenclature
\fi

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% table of contents settings

\setcounter{secnumdepth}{3}
\setcounter{tocdepth}{3}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% page header for chapters, too

\ifx\@empty\@isreport
\else
	\renewcommand*{\chapterpagestyle}{fancy}
\fi

% page header for abstracts, too
\let\@altab=\abstract
\let\end@altab=\endabstract
\renewenvironment{abstract}{%
\fancyfoot{}
\fancyhead{}
\if@final
	\fancyhead[LE,RO]{\thepage}
\else
	\if@twoside
		\fancyhead[LE,RO]{\thepage}
	\else
		\fancyhead[R]{\thepage}
	\fi
\fi
\begin{@altab}%
\thispagestyle{fancy}}{\end{@altab}%
\cleardoublepage
\ifx\@empty\@isreport
	\if@final
		\fancyhead[LO]{\@author}
		\fancyhead[RE]{\@pageheadertitle}
	\else
		\if@twoside
			\fancyhead[LO]{\@author}
			\fancyhead[RE]{\@pageheadertitle}
		\else
			\fancyhead[L]{\@author -- \@pageheadertitle}
		\fi
	\fi
\else
	\if@final
		\fancyhead[LO]{\rightmark}
		\fancyhead[RE]{\leftmark}
	\else
		\if@twoside
			\fancyhead[LO]{\rightmark}
			\fancyhead[RE]{\leftmark}
		\else
			\fancyhead[L]{\rightmark}
		\fi
	\fi
\fi}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% list of figures, list of tables

\if@lof
	\newcommand{\printlof}{\listoffigures}
\else
	\newcommand{\printlof}{ }
\fi
\if@lot
	\newcommand{\printlot}{\listoftables}
\else
	\newcommand{\printlot}{ }
\fi

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% bibliography, list of figures, list of tables before appendix

\def\@haveappendix{}
\let\@alternativeappendix=\appendix
\renewcommand{\appendix}[0]{\cleardoublepage
	\ifx\@empty\@bibfiles
	\else
		\nocite{*} % insert not cited references
		\bibliography{\@bibfiles}
	\fi
	\cleardoublepage
	\printlof
	\cleardoublepage
	\printlot
	\cleardoublepage
	\@alternativeappendix
	\def\@haveappendix{1}
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% generate document skeleton

\ifx\@empty\@isdiss
	\let\@altdoc=\document
	\let\end@altdoc=\enddocument
	\renewenvironment{document}{
		\begin{@altdoc} % ends in the next pair of {}

			\pagenumbering{roman}

			\pagestyle{fancy}
			\fancyhead{}
			\fancyfoot{}
			\fancyhead[LE,RO]{\thepage}
			\edef\headrulewidthx{\headrulewidth}
			\renewcommand{\headrulewidth}{0pt}
			\renewcommand{\footrulewidth}{0pt}

			% matthias weber uses this. But it seems not necessary
			% \fancypagestyle{plain}{
			% 	\fancyhead{}
			% 	\fancyfoot{}
			% 	\fancyhead[LE,RO]{\thepage}
			% 	\renewcommand{\headrulewidth}{0pt}
			% }
			% \fancypagestyle{empty}{
			% 	\fancyhead{}
			% 	\fancyfoot{}
			% 	\renewcommand{\headrulewidth}{0pt}
			% }

			\begin{titlepage} % customized for my diploma thesis
				\enlargethispage{5mm}

				\@institution
				\vspace{1cm}

				\begin{center}
					\Huge
					\titleheader\\[1.5cm]
					\huge
					\@title
					\large
				\end{center}
				\vfill

				\begin{flushleft}
					\normalsize
					\ifx\@empty\@isplain
						Author: \@author~(\@belowauthor)\\
						Professor: Prof. Dr. Wolfgang E. Nagel\\
						Tutors: \@betreuer \\[1cm]
					\else
						$\quad$\\[1cm]
					\fi
					Dresden, \@date
				\end{flushleft}
			\end{titlepage}

			\thispagestyle{empty}
			\cleardoublepage

			\onehalfspacing
			\normalsize

			\chapter*{Task}
			\@taskdescription
			\cleardoublepage

			\chapter*{Statement of Academic Integrity}
			I hereby declare that I prepared this thesis independently and without use of tools other than specified. Foreign thoughts, taken literally or in spirit, are marked as such. I also declare that I have not filed the present work at any other location or will submit it.
			\vspace{2cm} \\
			Ronny Brendel
			\cleardoublepage

			\chapter*{Abstract}
			\@abstractx
			\cleardoublepage

			\chapter*{Acknowledgments}
			\@acknowledgments
			\cleardoublepage


			\tableofcontents
			\cleardoublepage
			\chaptermark{}

			\pagenumbering{arabic}

			\renewcommand\headrulewidth{\headrulewidthx}

			\renewcommand{\chaptermark}[1]{\markboth{\thechapter\ ##1}{}}
			\renewcommand{\sectionmark}[1]{\markright{\thesection\ ##1}{}}
			\fancyhead[LO]{\nouppercase{\rightmark}}
			\fancyhead[RE]{\nouppercase{\leftmark}}
	}{
			% bibliography, list of figures, list of tables, if not already inserted at \appendix
			\ifx\@empty\@haveappendix
				\cleardoublepage
				\nocite{*} % insert not cited references
				\bibliography{\@bibfiles}
				\cleardoublepage
				\printlof
				\cleardoublepage
				\printlot
			\fi
			\cleardoublepage

			% copyright information
			\ifx\@empty\@copyrightinformation
			\else
				\@mkboth{Copyright Information}{}
				\copyrightinfo
			\fi
			\cleardoublepage

			\pagestyle{fancy}

		\end{@altdoc}
	}
\else
	% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	% %%% incorrect. look at the other if-branch of what it is supposed to look like approx %%%
	% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	% \let\@altdoc=\document
	% \let\end@altdoc=\enddocument
	% \renewenvironment{document}{
	% 	\begin{@altdoc} % ends in the next pair of {}
	% 		\begin{titlepage}
	% 			\enlargethispage{1cm}
	% 			\begin{center}
	% 				\sffamily
	% 				\vspace*{25mm}
	% 				\begin{Large}
	% 					\textbf{\@title}
	% 				\end{Large}

	% 				\vspace{20mm}
	% 				\titleheader\\[1.5cm]
	% 				\begin{large}
	% 					\vspace{18mm}
	% 					vorgelegt an der\\
	% 					Technischen Universit\"at Dresden\\
	% 					Fakult\"{a}t Informatik

	% 					\vspace{12mm}

	% 					eingereicht von

	% 					\vspace{6mm}

	% 					{\textbf{\@author}}\\\@belowauthor\\[2cm]
	% 					\vfill
	% 					Betreuender Hochschullehrer: \@gutachter\\[2cm]
	% 					\bigskip
	% 					Dresden, \@date\\[2cm]
	% 				\end{large}
	% 			\end{center}
	% 		\end{titlepage}

	% 		\cleardoubleemptypage

	% 		\ifx\@dedication\@empty
	% 		\else
	% 			\next@tpage\null\vfill
	% 			{\centering \em \@dedication \par}
	% 			\vskip \z@ \@plus3fill
	% 			\if@twoside
	% 				\next@tpage\cleardoubleemptypage
	% 			\fi
	% 		\fi
	% 		\cleardoublepage

	% 		\fancyfoot{}
	% 		\fancyhead{}
	% 		\pagestyle{fancy}
	% 		\thispagestyle{fancy}

	% 		% vary spacing depending on option 'final'
	% 		\if@final
	% 		  %\singlespacing
	% 		\else
	% 			\onehalfspacing
	% 		\fi

	% 		\normalsize

	% 		\if@kurz
	% 		\else
	% 			% abstract
	% 			{
	% 			\vspace{2cm}
	% 			\minisec{Abstract}
	% 			\@abstractx
	% 			}
	% 		\fi
	% 		\cleardoublepage

	% 		% table of contents
	% 		\if@notoc
	% 		\else
	% 			\tableofcontents
	% 			\cleardoublepage
	% 		\fi

	% 		% nomenclature/abbreviations
	% 		\if@nomencl
	% 		  \printnomenclature[\@nomenclmargin]
	% 		\fi

	% 		\pagenumbering{arabic}
	% 		\if@final
	% 			\fancyhead[LE,RO]{\thepage}
	% 		\else
	% 			\if@twoside
	% 				\fancyhead[LE,RO]{\thepage}
	% 			\else
	% 				\fancyhead[R]{\thepage}
	% 			\fi
	% 		\fi

	% 		% create page header
	% 		\markboth{}{}
	% 		\ifx\@empty\@isreport
	% 			\if@final
	% 				\fancyhead[LO]{\@author}
	% 				\fancyhead[RE]{\@pageheadertitle}
	% 			\else
	% 				\if@twoside
	% 					\fancyhead[LO]{\@author}
	% 					\fancyhead[RE]{\@pageheadertitle}
	% 				\else
	% 					\fancyhead[L]{\@author -- \@pageheadertitle}
	% 				\fi
	% 			\fi
	% 		\else
	% 			\if@final
	% 				\fancyhead[LO]{\rightmark}
	% 				\fancyhead[RE]{\leftmark}
	% 			\else
	% 				\if@twoside
	% 					\fancyhead[LO]{\rightmark}
	% 					\fancyhead[RE]{\leftmark}
	% 				\else
	% 					\fancyhead[L]{\leftmark}
	% 				\fi
	% 			\fi
	% 		\fi
	% }{
	% 		% bibliography, list of figures, list of tables, if not already inserted at \appendix
	% 		\ifx\@empty\@haveappendix
	% 			\cleardoublepage
	% 			\ifx\@empty\@bibfiles
	% 			\else
	% 				\nocite{*} % insert not cited references
	% 				\bibliography{\@bibfiles}
	% 			\fi
	% 			\cleardoublepage
	% 			\printlof
	% 			\cleardoublepage
	% 			\printlot
	% 		\fi
	% 		\cleardoublepage

	% 		% acknowledgments
	% 		\ifx\@empty\@acknowledgments
	% 		\else
	% 			\@mkboth{Acknowledgments}
	% 			\ifx\@empty\@isreport
	% 				\section*{Acknowledgments}
	% 			\else
	% 				\chapter*{Acknowledgments}
	% 			\fi
	% 			\@acknowledgments
	% 		\fi
	% 		\cleardoublepage

	% 		% copyright information
	% 		\ifx\@empty\@copyrightinformation
	% 		\else
	% 			\@mkboth{Copyright Information}{}
	% 			\copyrightinfo
	% 		\fi
	% 		\cleardoublepage
	% 	\end{@altdoc}
	% }
\fi

