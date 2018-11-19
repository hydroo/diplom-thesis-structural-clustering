template<typename Identifier>
void Profile_add(const Identifier& i, int64_t inv, int64_t exc, int64_t inc, Profile<Identifier>* p) {

	if (p->invocations.contains(i) == false) {
		Measure_init(&p->invocations[i]);
	}
	if (p->exclusiveTime.contains(i) == false) {
		Measure_init(&p->exclusiveTime[i]);
	}
	if (p->inclusiveTime.contains(i) == false) {
		Measure_init(&p->inclusiveTime[i]);
	}

	Measure_record(&p->invocations[i]  , inv);
	Measure_record(&p->exclusiveTime[i], exc);
	Measure_record(&p->inclusiveTime[i], inc);
}

template<typename Identifier>
void Profile_finalize(Profile<Identifier>* p) {

	assert(p->invocations.size()   == p->exclusiveTime.size());
	assert(p->exclusiveTime.size() == p->inclusiveTime.size());

	QMapIterator<Identifier, Measure<int64_t>> i(p->invocations);
	while (i.hasNext()) {
		i.next();

		assert(p->exclusiveTime.contains(i.key()));
		assert(p->inclusiveTime.contains(i.key()));

		Measure_finalize(&p->invocations  [i.key()]);
		Measure_finalize(&p->exclusiveTime[i.key()]);
		Measure_finalize(&p->inclusiveTime[i.key()]);
	}
}

template<typename Identifier>
QString Profile_print(const Profile<Identifier>& p, std::function<QString(const Identifier&)> printIdentifier, const QString& indent) {
	QString ret;
	QTextStream s(&ret);

	assert(p.invocations.size()   == p.exclusiveTime.size());
	assert(p.exclusiveTime.size() == p.inclusiveTime.size());

	QMapIterator<Identifier, Measure<int64_t>> i(p.invocations);
	while (i.hasNext()) {
		i.next();
	}

	QList<const QMap<Identifier, Measure<int64_t>>*> maps;
	maps.append(&p.invocations);
	maps.append(&p.exclusiveTime);
	maps.append(&p.inclusiveTime);

	int largestIdentifierString          = 0;
	int largestInvDataPointCount         = 0;
	int largestInvMin                    = 0;
	int largestInvSecondPercentile       = 0;
	int largestInvFirstQuartile          = 0;
	int largestInvMedian                 = 0;
	int largestInvThirdQuartile          = 0;
	int largestInvNinetyEighthPercentile = 0;
	int largestInvMax                    = 0;
	int largestInvMean                   = 0;
	int largestInvAccumulated            = 0;
	int largestExcDataPointCount         = 0;
	int largestExcMin                    = 0;
	int largestExcSecondPercentile       = 0;
	int largestExcFirstQuartile          = 0;
	int largestExcMedian                 = 0;
	int largestExcThirdQuartile          = 0;
	int largestExcNinetyEighthPercentile = 0;
	int largestExcMax                    = 0;
	int largestExcMean                   = 0;
	int largestExcAccumulated            = 0;
	int largestIncDataPointCount         = 0;
	int largestIncMin                    = 0;
	int largestIncSecondPercentile       = 0;
	int largestIncFirstQuartile          = 0;
	int largestIncMedian                 = 0;
	int largestIncThirdQuartile          = 0;
	int largestIncNinetyEighthPercentile = 0;
	int largestIncMax                    = 0;
	int largestIncMean                   = 0;
	int largestIncAccumulated            = 0;

	QMapIterator<Identifier, Measure<int64_t>> j_(p.invocations);
	while (j_.hasNext()) {
		j_.next();
		const Identifier& j = j_.key();

		assert(p.exclusiveTime.contains(j));
		assert(p.inclusiveTime.contains(j));

		int identifierString          = printIdentifier(j).size();
		int invDataPointCount         = QString("%1").arg(p.invocations  [j].dataPointCount)        .size();
		int invMin                    = QString("%1").arg(p.invocations  [j].min)                   .size();
		int invSecondPercentile       = QString("%1").arg(p.invocations  [j].secondPercentile)      .size();
		int invFirstQuartile          = QString("%1").arg(p.invocations  [j].firstQuartile)         .size();
		int invMedian                 = QString("%1").arg(p.invocations  [j].median)                .size();
		int invThirdQuartile          = QString("%1").arg(p.invocations  [j].thirdQuartile)         .size();
		int invNinetyEighthPercentile = QString("%1").arg(p.invocations  [j].ninetyEighthPercentile).size();
		int invMax                    = QString("%1").arg(p.invocations  [j].max)                   .size();
		int invMean                   = QString("%1").arg(p.invocations  [j].mean, 0, 'f', 2)       .size();
		int invAccumulated            = QString("%1").arg(p.invocations  [j].accumulated)           .size();
		int excDataPointCount         = QString("%1").arg(p.exclusiveTime[j].dataPointCount)        .size();
		int excMin                    = QString("%1").arg(p.exclusiveTime[j].min)                   .size();
		int excSecondPercentile       = QString("%1").arg(p.exclusiveTime[j].secondPercentile)      .size();
		int excFirstQuartile          = QString("%1").arg(p.exclusiveTime[j].firstQuartile)         .size();
		int excMedian                 = QString("%1").arg(p.exclusiveTime[j].median)                .size();
		int excThirdQuartile          = QString("%1").arg(p.exclusiveTime[j].thirdQuartile)         .size();
		int excNinetyEighthPercentile = QString("%1").arg(p.exclusiveTime[j].ninetyEighthPercentile).size();
		int excMax                    = QString("%1").arg(p.exclusiveTime[j].max)                   .size();
		int excMean                   = QString("%1").arg(p.exclusiveTime[j].mean, 0, 'f', 2)       .size();
		int excAccumulated            = QString("%1").arg(p.exclusiveTime[j].accumulated)           .size();
		int incDataPointCount         = QString("%1").arg(p.inclusiveTime[j].dataPointCount)        .size();
		int incMin                    = QString("%1").arg(p.inclusiveTime[j].min)                   .size();
		int incSecondPercentile       = QString("%1").arg(p.inclusiveTime[j].secondPercentile)      .size();
		int incFirstQuartile          = QString("%1").arg(p.inclusiveTime[j].firstQuartile)         .size();
		int incMedian                 = QString("%1").arg(p.inclusiveTime[j].median)                .size();
		int incThirdQuartile          = QString("%1").arg(p.inclusiveTime[j].thirdQuartile)         .size();
		int incNinetyEighthPercentile = QString("%1").arg(p.inclusiveTime[j].ninetyEighthPercentile).size();
		int incMax                    = QString("%1").arg(p.inclusiveTime[j].max)                   .size();
		int incMean                   = QString("%1").arg(p.inclusiveTime[j].mean, 0, 'f', 2)       .size();
		int incAccumulated            = QString("%1").arg(p.inclusiveTime[j].accumulated)           .size();

		if (identifierString          > largestIdentifierString         ) { largestIdentifierString          = identifierString         ; }
		if (invDataPointCount         > largestInvDataPointCount        ) { largestInvDataPointCount         = invDataPointCount        ; }
		if (invMin                    > largestInvMin                   ) { largestInvMin                    = invMin                   ; }
		if (invSecondPercentile       > largestInvSecondPercentile      ) { largestInvSecondPercentile       = invSecondPercentile      ; }
		if (invFirstQuartile          > largestInvFirstQuartile         ) { largestInvFirstQuartile          = invFirstQuartile         ; }
		if (invMedian                 > largestInvMedian                ) { largestInvMedian                 = invMedian                ; }
		if (invThirdQuartile          > largestInvThirdQuartile         ) { largestInvThirdQuartile          = invThirdQuartile         ; }
		if (invNinetyEighthPercentile > largestInvNinetyEighthPercentile) { largestInvNinetyEighthPercentile = invNinetyEighthPercentile; }
		if (invMax                    > largestInvMax                   ) { largestInvMax                    = invMax                   ; }
		if (invMean                   > largestInvMean                  ) { largestInvMean                   = invMean                  ; }
		if (invAccumulated            > largestInvAccumulated           ) { largestInvAccumulated            = invAccumulated           ; }
		if (excDataPointCount         > largestExcDataPointCount        ) { largestExcDataPointCount         = excDataPointCount        ; }
		if (excMin                    > largestExcMin                   ) { largestExcMin                    = excMin                   ; }
		if (excSecondPercentile       > largestExcSecondPercentile      ) { largestExcSecondPercentile       = excSecondPercentile      ; }
		if (excFirstQuartile          > largestExcFirstQuartile         ) { largestExcFirstQuartile          = excFirstQuartile         ; }
		if (excMedian                 > largestExcMedian                ) { largestExcMedian                 = excMedian                ; }
		if (excThirdQuartile          > largestExcThirdQuartile         ) { largestExcThirdQuartile          = excThirdQuartile         ; }
		if (excNinetyEighthPercentile > largestExcNinetyEighthPercentile) { largestExcNinetyEighthPercentile = excNinetyEighthPercentile; }
		if (excMax                    > largestExcMax                   ) { largestExcMax                    = excMax                   ; }
		if (excMean                   > largestExcMean                  ) { largestExcMean                   = excMean                  ; }
		if (excAccumulated            > largestExcAccumulated           ) { largestExcAccumulated            = excAccumulated           ; }
		if (incDataPointCount         > largestIncDataPointCount        ) { largestIncDataPointCount         = incDataPointCount        ; }
		if (incMin                    > largestIncMin                   ) { largestIncMin                    = incMin                   ; }
		if (incSecondPercentile       > largestIncSecondPercentile      ) { largestIncSecondPercentile       = incSecondPercentile      ; }
		if (incFirstQuartile          > largestIncFirstQuartile         ) { largestIncFirstQuartile          = incFirstQuartile         ; }
		if (incMedian                 > largestIncMedian                ) { largestIncMedian                 = incMedian                ; }
		if (incThirdQuartile          > largestIncThirdQuartile         ) { largestIncThirdQuartile          = incThirdQuartile         ; }
		if (incNinetyEighthPercentile > largestIncNinetyEighthPercentile) { largestIncNinetyEighthPercentile = incNinetyEighthPercentile; }
		if (incMax                    > largestIncMax                   ) { largestIncMax                    = incMax                   ; }
		if (incMean                   > largestIncMean                  ) { largestIncMean                   = incMean                  ; }
		if (incAccumulated            > largestIncAccumulated           ) { largestIncAccumulated            = incAccumulated           ; }
	}

	//s << indent << "# identifier (dataPointCount): min 2% 25% 50% 75% 98% max mean accumulated\n";
	s << indent << "# identifier: 2% 25% 50% 75% 98% accumulated\n";

	foreach (const auto* m, maps) {

		int largestDataPointCount;         (void)largestDataPointCount;
		int largestMin;                    (void)largestMin;
		int largestSecondPercentile;
		int largestFirstQuartile;
		int largestMedian;
		int largestThirdQuartile;
		int largestNinetyEighthPercentile;
		int largestMax;                    (void)largestMax;
		int largestMean;                   (void)largestMean;
		int largestAccumulated;

		if (m == &p.invocations) {
			s << indent << "invocations:\n";
			largestDataPointCount         = largestInvDataPointCount        ;
			largestMin                    = largestInvMin                   ;
			largestSecondPercentile       = largestInvSecondPercentile      ;
			largestFirstQuartile          = largestInvFirstQuartile         ;
			largestMedian                 = largestInvMedian                ;
			largestThirdQuartile          = largestInvThirdQuartile         ;
			largestNinetyEighthPercentile = largestInvNinetyEighthPercentile;
			largestMax                    = largestInvMax                   ;
			largestMean                   = largestInvMean                  ;
			largestAccumulated            = largestInvAccumulated           ;
		} else if (m == &p.exclusiveTime) {
			s << indent << "exclusive time:\n";
			largestDataPointCount         = largestExcDataPointCount        ;
			largestMin                    = largestExcMin                   ;
			largestSecondPercentile       = largestExcSecondPercentile      ;
			largestFirstQuartile          = largestExcFirstQuartile         ;
			largestMedian                 = largestExcMedian                ;
			largestThirdQuartile          = largestExcThirdQuartile         ;
			largestNinetyEighthPercentile = largestExcNinetyEighthPercentile;
			largestMax                    = largestExcMax                   ;
			largestMean                   = largestExcMean                  ;
			largestAccumulated            = largestExcAccumulated           ;
		} else { /* m == &p.inclusiveTime */
			s << indent << "inclusive time:\n";
			largestDataPointCount         = largestIncDataPointCount        ;
			largestMin                    = largestIncMin                   ;
			largestSecondPercentile       = largestIncSecondPercentile      ;
			largestFirstQuartile          = largestIncFirstQuartile         ;
			largestMedian                 = largestIncMedian                ;
			largestThirdQuartile          = largestIncThirdQuartile         ;
			largestNinetyEighthPercentile = largestIncNinetyEighthPercentile;
			largestMax                    = largestIncMax                   ;
			largestMean                   = largestIncMean                  ;
			largestAccumulated            = largestIncAccumulated           ;
		}

		QMapIterator<Identifier, Measure<int64_t>> j(*m);
		while (j.hasNext()) {
			j.next();

			s << indent + "\t" << QString("%1").arg(printIdentifier(j.key()), largestIdentifierString);
			//is always = number of processes if filled with 0s
			//s << QString(" (%1):").arg(j.value().dataPointCount        , largestDataPointCount);
			s << " :";
			//s << QString(" %1")   .arg(j.value().min                   , largestMin);
			s << QString(" %1")   .arg(j.value().secondPercentile      , largestSecondPercentile);
			s << QString(" %1")   .arg(j.value().firstQuartile         , largestFirstQuartile);
			s << QString(" %1")   .arg(j.value().median                , largestMedian);
			s << QString(" %1")   .arg(j.value().thirdQuartile         , largestThirdQuartile);
			s << QString(" %1")   .arg(j.value().ninetyEighthPercentile, largestNinetyEighthPercentile);
			//s << QString(" %1")   .arg(j.value().max                   , largestMax);
			//s << QString(" %1")   .arg(j.value().mean                  , largestMean, 'f', 2);
			s << QString(" %1")   .arg(j.value().accumulated           , largestAccumulated);
			s << "\n";
		}
	}

	return ret;
}
