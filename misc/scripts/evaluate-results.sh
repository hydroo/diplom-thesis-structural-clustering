#!/bin/bash

#APP=amg
#APP=par
APP=bt

T=m #call-matrix
#T=l #call-list

echo $APP $T

for A in $(ls -l | awk '{print $9;}' | egrep "$APP$T.*\-[1]\.err")
do
	if [ $APP = amg ]
	then
		N=$(expr substr $A 5 4)
	elif [ $APP = par ]
	then
		N=$(expr substr $A 5 4)
	elif [ $APP = bt ]
	then
		N=$(expr substr $A 4 5) #5 digit processes instead of 4 as for amg and par
	fi

	FILE=$APP$T$N-1.err
	# this should only be one line
	# LINE=$(tail -n 1 $FILE)
	LINE=$(cat $FILE)

	ATTRIBUTE_COUNT=$(echo $LINE | awk '{print  $8;}' | sed s/,//)
	NODE_COUNT_W=$(   echo $LINE | awk '{print $12;}' | sed s/,//)
	NODE_COUNT_WO=$(  echo $LINE | awk '{print $16;}' | sed s/,//)
	GROUP_COUNT=$(    echo $LINE | awk '{print $19;}' | sed s/,//)

	MERGE_L=""
	FINALIZE_L=""
	CALC_SIM_L=""
	LATTICE_L=""
	LOADING_L=""
	TOTAL_L=""

	for R in $(seq 1 5)
	do
		FILE=$APP$T$N-$R.err
		# this should only be one line
		# LINE=$(tail -n 1 $FILE)
		LINE=$(cat $FILE)

		ATTRIBUTE_COUNT_=$(echo $LINE | awk '{print  $8;}' | sed s/,//)
		NODE_COUNT_W_=$(   echo $LINE | awk '{print $12;}' | sed s/,//)
		NODE_COUNT_WO_=$(  echo $LINE | awk '{print $16;}' | sed s/,//)
		GROUP_COUNT_=$(    echo $LINE | awk '{print $19;}' | sed s/,//)

		if [ $ATTRIBUTE_COUNT != $ATTRIBUTE_COUNT_ ]; then echo "wrong ATTRIBUTE_COUNT for $FILE"; fi
		if [ $NODE_COUNT_W    != $NODE_COUNT_W_    ]; then echo "wrong NODE_COUNT_W for $FILE"   ; fi
		if [ $NODE_COUNT_WO   != $NODE_COUNT_WO_   ]; then echo "wrong NODE_COUNT_WO for $FILE"  ; fi
		if [ $GROUP_COUNT     != $GROUP_COUNT_     ]; then echo "wrong GROUP_COUNT for $FILE"    ; fi

		FINALIZE=$(echo $LINE | awk '{print  $5;}' | sed s/s,//)
		CALC_SIM=$(echo $LINE | awk '{print $22;}' | sed s/s,//)
		LATTICE=$( echo $LINE | awk '{print $28;}' | sed s/s,//)
		LOADING=$( echo $LINE | awk '{print $26;}' | sed s/s,//)
		TOTAL=$(   echo $LINE | awk '{print $24;}' | sed s/s,//)

		MERGE=$(awk "BEGIN {printf \"%.2f\" , $LATTICE - $FINALIZE - $CALC_SIM}")

		MERGE_L="$MERGE_L $MERGE"
		FINALIZE_L="$FINALIZE_L $FINALIZE"
		CALC_SIM_L="$CALC_SIM_L $CALC_SIM"
		LATTICE_L="$LATTICE_L $LATTICE"
		LOADING_L="$LOADING_L $LOADING"
		TOTAL_L="$TOTAL_L $TOTAL"
	done

	MERGE_L=$(   printf "%s\n" $MERGE_L    | sort -n | tr "\\n" " ")
	FINALIZE_L=$(printf "%s\n" $FINALIZE_L | sort -n | tr "\\n" " ")
	CALC_SIM_L=$(printf "%s\n" $CALC_SIM_L | sort -n | tr "\\n" " ")
	LATTICE_L=$( printf "%s\n" $LATTICE_L  | sort -n | tr "\\n" " ")
	LOADING_L=$( printf "%s\n" $LOADING_L  | sort -n | tr "\\n" " ")
	TOTAL_L=$(   printf "%s\n" $TOTAL_L    | sort -n | tr "\\n" " ")

	MERGE_MIN=$(echo $MERGE_L | awk '{print $1}')
	MERGE_MED=$(echo $MERGE_L | awk '{print $3}')
	MERGE_MAX=$(echo $MERGE_L | awk '{print $5}')
	MERGE_AVG=$(echo $MERGE_L | awk '{print ($1+$2+$3+$4+$5)/5}')

	FINALIZE_MIN=$(echo $FINALIZE_L | awk '{print $1}')
	FINALIZE_MED=$(echo $FINALIZE_L | awk '{print $3}')
	FINALIZE_MAX=$(echo $FINALIZE_L | awk '{print $5}')
	FINALIZE_AVG=$(echo $FINALIZE_L | awk '{print ($1+$2+$3+$4+$5)/5}')

	CALC_SIM_MIN=$(echo $CALC_SIM_L | awk '{print $1}')
	CALC_SIM_MED=$(echo $CALC_SIM_L | awk '{print $3}')
	CALC_SIM_MAX=$(echo $CALC_SIM_L | awk '{print $5}')
	CALC_SIM_AVG=$(echo $CALC_SIM_L | awk '{print ($1+$2+$3+$4+$5)/5}')

	LATTICE_MIN=$( echo $LATTICE_L  | awk '{print $1}')
	LATTICE_MED=$( echo $LATTICE_L  | awk '{print $3}')
	LATTICE_MAX=$( echo $LATTICE_L  | awk '{print $5}')
	LATTICE_AVG=$( echo $LATTICE_L  | awk '{print ($1+$2+$3+$4+$5)/5}')

	LOADING_MIN=$( echo $LOADING_L  | awk '{print $1}')
	LOADING_MED=$( echo $LOADING_L  | awk '{print $3}')
	LOADING_MAX=$( echo $LOADING_L  | awk '{print $5}')
	LOADING_AVG=$( echo $LOADING_L  | awk '{print ($1+$2+$3+$4+$5)/5}')

	TOTAL_MIN=$(   echo $TOTAL_L    | awk '{print $1}')
	TOTAL_MED=$(   echo $TOTAL_L    | awk '{print $3}')
	TOTAL_MAX=$(   echo $TOTAL_L    | awk '{print $5}')
	TOTAL_AVG=$(   echo $TOTAL_L    | awk '{print ($1+$2+$3+$4+$5)/5}')

	echo "procs $N attr $ATTRIBUTE_COUNT  nodesw $NODE_COUNT_W nodeswo $NODE_COUNT_WO groups $GROUP_COUNT"
	printf "  merge    min %5.5f avg %5.5f med %5.5f max %5.5f\n" $MERGE_MIN $MERGE_AVG $MERGE_MED $MERGE_MAX
	printf "  finalize min %5.5f avg %5.5f med %5.5f max %5.5f\n" $FINALIZE_MIN $FINALIZE_AVG $FINALIZE_MED $FINALIZE_MAX
	printf "  calcsim  min %5.5f avg %5.5f med %5.5f max %5.5f\n" $CALC_SIM_MIN $CALC_SIM_AVG $CALC_SIM_MED $CALC_SIM_MAX
	printf "  lattice  min %5.5f avg %5.5f med %5.5f max %5.5f\n" $LATTICE_MIN $LATTICE_AVG $LATTICE_MED $LATTICE_MAX
	printf "  loading  min %5.5f avg %5.5f med %5.5f max %5.5f\n" $LOADING_MIN $LOADING_AVG $LOADING_MED $LOADING_MAX
	printf "  total    min %5.5f avg %5.5f med %5.5f max %5.5f\n" $TOTAL_MIN $TOTAL_AVG $TOTAL_MED $TOTAL_MAX
done
