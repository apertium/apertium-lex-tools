#!/bin/bash

LANG1="en"
LANG2="ca"
BASENAME="apertium-"$LANG1'-'$LANG2
PAIR1=$LANG1'-'$LANG2
PAIR2=$LANG2'-'$LANG1
PAIRDIR=../../apertium-en-ca/
EVAL=../scripts/apertium-lex-evaluate.py
BILMODE=$PAIR2"-biltrans"
TESTDIR="test"
TEMPDIR="/tmp"

## Frequency

for i in $TESTDIR/*.$LANG2; do 
	SRC=$TEMPDIR"/"`basename $i`".src";
	TST=$TEMPDIR"/"`basename $i`".tst";
	REF=`echo $i | sed "s/\.$LANG2//g"`;
	cat $i | cut -f2- | apertium-destxt | apertium -f none -d $PAIRDIR $BILMODE > $SRC;
	cat $SRC | python ../apertium-lex-freq.py ../freq.txt > $TST;	
	RES=`python $EVAL $SRC $REF $TST 2>/dev/null`;
	RES1=`python $EVAL $SRC $REF $TST 2>/dev/null | cut -f2`;
	#echo -e $RES"\tfreq\t"`basename $i`;
	cat $SRC | ../apertium-lex-defaults $PAIRDIR/ca-en.autoldx.bin | python ../apertium-lex-freq.py ../freq.txt > $TST;	
	RES=`python $EVAL $SRC $REF $TST 2>/dev/null`;
	RES2=`python $EVAL $SRC $REF $TST 2>/dev/null | cut -f2`;
	#echo -e $RES"\tling\t"`basename $i`;

	echo -e `basename $i`"\t"$RES1"\t"$RES2;
done

exit;
for i in $TESTDIR/*.$LANG2; do 
	rm $TEMPDIR"/"$i".src";
	rm $TEMPDIR"/"$i".tst";
done


