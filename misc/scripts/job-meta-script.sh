#!/bin/bash

#APP=amg
#APP=par
APP=bt

T=m #call-matrix
#T=l #call-list

echo $APP $T

#PROC="0004 0008 0016 0032 0064 0128 0256 0512 1024 2048 4096" # 4 digit processes needed for amg and par
PROC="00064 00128 00256 00512 01024 02048 04096 08192 16384 32768 65536" # 5 digit processes needed for bt
REPS="1 2 3 4 5"
for n in $PROC
do
	for r in $REPS
	do
		SCRIPT="tmp-$APP$T$n-$r-jobscript"

		echo "#!/bin/bash" > $SCRIPT
		echo "" >> $SCRIPT
		echo "#SBATCH -p sandy" >> $SCRIPT
		echo "#SBATCH --time=08:00:00" >> $SCRIPT
		echo "#SBATCH --output=$APP$T$n-$r.out" >> $SCRIPT
		echo "#SBATCH --error=$APP$T$n-$r.err" >> $SCRIPT
		echo "#SBATCH -n 1" >> $SCRIPT
		echo "#SBATCH -N 1" >> $SCRIPT
		echo "#SBATCH --exclusive" >> $SCRIPT
		echo "#SBATCH --mem-per-cpu 24576" >> $SCRIPT
		echo "#SBATCH -J $APP$T$n-$r" >> $SCRIPT
		echo "" >> $SCRIPT

		if [ $APP = amg ]
		then
			echo "TRACE=\"/trcdata/tracefiles/maweber/ronny-lattice/amg2006/filt/$n/*.otf\"" >> $SCRIPT
		elif [ $APP = par ]
		then
			echo "TRACE=\"/trcdata/tracefiles/maweber/ronny-lattice/paradis/paradis-2.5.1.1_sk_$n-proc_filt/*.otf\"" >> $SCRIPT
		elif [ $APP = bt ]
		then
			echo "TRACE=\"/trcdata/tracefiles/brendel/scorep-bt-mz-${n}p-?-*/*.otf2\"" >> $SCRIPT
		fi
		if [ $T = m ]
		then
			echo "TOOL=\"/home/brendel/vis-git/tools/call-matrix/call-matrix\"" >> $SCRIPT
		elif [ $T = l ]
		then
			echo "TOOL=\"/home/brendel/vis-git/tools/call-list/call-list\"" >> $SCRIPT
		fi
		echo "PARAM=\"-%2 --simple-similarity\"" >> $SCRIPT
		echo "" >> $SCRIPT
		echo "srun --cpu-freq=2900000 -N 1 -n 1 -T 1 \$TOOL \$PARAM \$TRACE" >> $SCRIPT

		sbatch $SCRIPT
		rm $SCRIPT
	done
done
