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
if [ ! -z $1 ]; then
	TYPE=$1
else
	TYPE="word"
fi
## Frequency

#for i in $TESTDIR/*.$LANG2; do 
for i in $TESTDIR/prova.n_f.$LANG2; do 
	SRC=$TEMPDIR"/"`basename $i`".$TYPE.src";
	TST=$TEMPDIR"/"`basename $i`".$TYPE.tst";
	REF=`echo $i | sed "s/\.$LANG2/.$TYPE/g"`;
	echo $SRC" "$TST" "$REF
	#cat $i | cut -f2- | apertium-destxt | apertium -f none -d $PAIRDIR $BILMODE > $SRC;
	cat $i | apertium-destxt | sed 's/\([0-9]*\)\[\t/[\1\t/g' | apertium -f none -d $PAIRDIR $BILMODE > $SRC;
	cat $SRC | python ../apertium-lex-freq.py ../examples/freq.txt > $TST".0";	
	RES=`python $EVAL $SRC $REF $TST".0" 2>/dev/null`;
	RES1=`python $EVAL $SRC $REF $TST".0" 2>/dev/null | cut -f2`;
	#echo -e $RES"\tfreq\t"`basename $i`;
	cat $SRC | ../apertium-lex-defaults $PAIRDIR/ca-en.autoldx.bin | python ../apertium-lex-freq.py ../examples/freq.txt > $TST".1";	
	RES=`python $EVAL $SRC $REF $TST".1" 2>/dev/null`;
	RES2=`python $EVAL $SRC $REF $TST".1" 2>/dev/null | cut -f2`;
	#echo -e $RES"\tling\t"`basename $i`;
	cat $SRC | ../apertium-lex-defaults $PAIRDIR/dev/lex.f2e.top.bin | python ../apertium-lex-freq.py ../examples/freq.txt > $TST".2";	
	RES=`python $EVAL $SRC $REF $TST".2" 2>/dev/null`;
	RES3=`python $EVAL $SRC $REF $TST".2" 2>/dev/null | cut -f2`;
	#echo -e $RES"\tling\t"`basename $i`;
	cat $SRC | python ../apertium-lex-rules.py ../examples/rules.txt | python ../apertium-lex-freq.py ../examples/freq.txt > $TST".3";
	RES=`python $EVAL $SRC $REF $TST".3" 2>/dev/null`;
	RES4=`python $EVAL $SRC $REF $TST".3" 2>/dev/null | cut -f2`;

	echo -e `basename $i`"\t"$RES1"\t"$RES2"\t"$RES3"\t"$RES4;
done

exit;
for i in $TESTDIR/*.$LANG2; do 
	rm $TEMPDIR"/"$i".src";
	rm $TEMPDIR"/"$i".tst";
done
