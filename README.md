# Diplom Thesis: Automatic Profile-Based Characterisation of Performance Traces for Highly Parallel Applications

Read the [blog post](https://blog.automaton2000.com/2018/11/diplom-thesis-automatic-profile-based-characterisation-of-performance-traces-for-highly-parallel-applications.html) or [paper](http://automaton2000.com/publications/2016-ipdps-structural-clustering-a-new-approach-to-support-performance-analysis-at-scale.pdf) for more info. [(defense slides)](https://github.com/hydroo/diplom-thesis-structural-clustering/raw/master/defense-presentation/slides.pdf), [(thesis)](https://github.com/hydroo/diplom-thesis-structural-clustering/raw/master/report/diploma-thesis.pdf)

## Structure

- `hs14-visualizing-structural-differences` contains the [presentation](https://github.com/hydroo/diplom-thesis-structural-clustering/raw/master/hs14-visualizing-structural-differences/presentation/slides.pdf) and [write-up](https://github.com/hydroo/diplom-thesis-structural-clustering/raw/master/hs14-visualizing-structural-differences/report/report.pdf) for the independent study course prior to starting the thesis.
- `/report` The diplom thesis
- `/tools` All source code
    - `/call-*` provide similar analyses for the different profile types. The most important tool is `call-matrix`.
    - `/common` Code shared between all applications. Most interesting might be the concept lattice implementation. It also sports an incremental generation algorithm extracted from [a paper](https://doi.org/10.1007/978-3-540-24651-0_31), because the most common algorithm does not scale well. The basic OTF and OTF2 reader library might also be useful for you.
- `/misc` Example trace files, test trace files and scripts

## Requirements

To build the code you need to install:

- [OTF](https://tu-dresden.de/zih/otf)
- [OTF2](http://www.vi-hps.org/projects/score-p/), [(Alternative link)](https://doi.org/10.5281/zenodo.1240852)
- [Qt 5](https://www.qt.io/download-qt-installer)

## License

Everything is licensed under the [Creative Commons Attribution-NonCommercial-ShareAlike 4.0](http://creativecommons.org/licenses/by-nc-sa/4.0) license.
If this licensing scheme is a problem for your intentions, please contact me.
