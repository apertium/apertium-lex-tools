CORPUS=setimes
PAIR=en-mk
DATA=/home/philip/Apertium/apertium-mk-en
SL=mk
TL=en
TRAINING_LINES=10000
THR=1
MIN=0
MODEL=/home/philip/Apertium/corpora/language-models/en/setimes.en.5.blm

SCRIPTS=/home/philip/Apertium/apertium-lex-tools/scripts

LEX_TOOLS=/home/philip/Apertium/apertium-lex-tools
AUTOBIL=$(SL)-$(TL).autobil.bin
DIR=$(SL)-$(TL)
YASMET=$(LEX_TOOLS)/yasmet

all: data/$(CORPUS).$(DIR).freq.lrx.bin data/$(CORPUS).$(DIR).lm.xml

data/$(CORPUS).$(DIR).tagger: $(CORPUS).$(PAIR).$(SL)
	if [ ! -d data ]; then mkdir data; fi
	cat $(CORPUS).$(PAIR).$(SL) | head -n $(TRAINING_LINES) | apertium-destxt | apertium -f none -d $(DATA) $(DIR)-tagger | apertium-pretransfer > $@
 
data/$(CORPUS).$(DIR).ambig: data/$(CORPUS).$(DIR).tagger
	cat data/$(CORPUS).$(DIR).tagger | $(LEX_TOOLS)/multitrans $(DATA)/$(AUTOBIL) -b -t -f -n > $@

data/$(CORPUS).$(DIR).multi-trimmed: data/$(CORPUS).$(DIR).tagger
	cat data/$(CORPUS).$(DIR).tagger | $(LEX_TOOLS)/multitrans $(DATA)/$(AUTOBIL) -m -t -f > $@

data/$(CORPUS).$(DIR).annotated: data/$(CORPUS).$(DIR).tagger data/$(CORPUS).$(DIR).multi-trimmed
	cat data/$(CORPUS).$(DIR).tagger | $(LEX_TOOLS)/multitrans $(DATA)/$(AUTOBIL) -m -f | apertium -f none -d $(DATA) $(DIR)-multi | $(LEX_TOOLS)/irstlm-ranker $(MODEL) data/$(CORPUS).$(DIR).multi-trimmed -f 2>/dev/null > $@ 

data/$(CORPUS).$(DIR).freq: data/$(CORPUS).$(DIR).ambig data/$(CORPUS).$(DIR).annotated
	python3 $(SCRIPTS)/biltrans-extract-frac-freq.py data/$(CORPUS).$(DIR).ambig data/$(CORPUS).$(DIR).annotated > $@
 
data/$(CORPUS).$(DIR).freq.lrx:  data/$(CORPUS).$(DIR).freq
	python3 $(SCRIPTS)/extract-alig-lrx.py $< > $@

data/$(CORPUS).$(DIR).freq.lrx.bin: data/$(CORPUS).$(DIR).freq.lrx
	lrx-comp $< $@

data/$(CORPUS).$(DIR).events data/$(CORPUS).$(DIR).ngrams: data/$(CORPUS).$(DIR).annotated data/$(CORPUS).$(DIR).freq
	python3 $(SCRIPTS)/biltrans-count-patterns-frac-maxent.py data/$(CORPUS).$(SL)-$(TL).freq data/$(CORPUS).$(SL)-$(TL).ambig data/$(CORPUS).$(SL)-$(TL).annotated > data/$(CORPUS).$(DIR).events 2>data/$(CORPUS).$(DIR).ngrams

data/$(CORPUS).$(DIR).all-lambdas: data/$(CORPUS).$(DIR).events
	cat data/$(CORPUS).$(DIR).events | grep -v -e '\$$ 0\.0 #' -e '\$$ 0 #' > data/$(CORPUS).$(DIR).events.trimmed
	cat data/$(CORPUS).$(DIR).events.trimmed | python $(SCRIPTS)/merge-all-lambdas.py > $@

data/$(CORPUS).$(DIR).rules-all: data/$(CORPUS).$(DIR).ngrams data/$(CORPUS).$(DIR).all-lambdas
	python3 $(SCRIPTS)/merge-ngrams-lambdas.py data/$(CORPUS).$(DIR).ngrams data/$(CORPUS).$(DIR).all-lambdas > $@

data/$(CORPUS).$(DIR).ngrams-all: data/$(CORPUS).$(DIR).freq data/$(CORPUS).$(DIR).rules-all
	python3 $(SCRIPTS)/lambdas-to-rules.py data/$(CORPUS).$(DIR).freq data/$(CORPUS).$(DIR).rules-all > $@

data/$(CORPUS).$(DIR).ngrams-trimmed: data/$(CORPUS).$(DIR).ngrams-all
	cat $< | python3 $(SCRIPTS)/ngram-pareto-trim.py > $@

data/$(CORPUS).$(DIR).lm.xml: data/$(CORPUS).$(DIR).ngrams-trimmed
	python3 $(SCRIPTS)/ngrams-to-rules-me.py data/$(CORPUS).$(DIR).ngrams-trimmed > $@


