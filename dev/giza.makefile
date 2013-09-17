CORPUS=europarl-v6
PAIR=es-en
SL=es
TL=en
DATA=/home/philip/Apertium/apertium-en-es

LEX_TOOLS=/home/philip/Apertium/apertium-lex-tools
SCRIPTS=$(LEX_TOOLS)/scripts
MOSESDECODER=/home/philip/mosesdecoder/scripts/training
TRAINING_LINES=200000
BIN_DIR=/home/philip/giza-pp/bin
LM=/home/philip/Apertium/gsoc2013/giza/dummy.lm

crisphold=1

all: data-$(SL)-$(TL)/$(CORPUS).ngrams.$(SL)-$(TL).lrx data-$(SL)-$(TL)/$(CORPUS).biltrans-entries.$(SL)-$(TL)

# TAG CORPUS
data-$(SL)-$(TL)/$(CORPUS).tagged.$(SL): $(CORPUS).$(PAIR).$(SL)
	if [ ! -d data-$(SL)-$(TL) ]; then mkdir data-$(SL)-$(TL); fi
	cat $(CORPUS).$(PAIR).$(SL) | head -n $(TRAINING_LINES) \
	| apertium-destxt \
	| apertium -f none -d $(DATA) $(SL)-$(TL)-tagger \
	| apertium-pretransfer > $@;

data-$(SL)-$(TL)/$(CORPUS).tagged.$(TL): $(CORPUS).$(PAIR).$(TL)
	if [ ! -d data-$(SL)-$(TL) ]; then mkdir data-$(SL)-$(TL); fi
	cat $(CORPUS).$(PAIR).$(TL) | head -n $(TRAINING_LINES) \
	| apertium-destxt \
	| apertium -f none -d $(DATA) $(TL)-$(SL)-tagger \
	| apertium-pretransfer > $@;

# REMOVE LINES WITH NO ANALYSES
data-$(SL)-$(TL)/$(CORPUS).tagged.new.$(SL): data-$(SL)-$(TL)/$(CORPUS).tagged.$(SL) data-$(SL)-$(TL)/$(CORPUS).tagged.$(TL)
	paste data-$(SL)-$(TL)/$(CORPUS).tagged.$(SL) data-$(SL)-$(TL)/$(CORPUS).tagged.$(TL) \
	| grep '<' \
	| cut -f1 \
	| sed 's/ /~/g' | sed 's/$$[^\^]*/$$ /g' > $@

data-$(SL)-$(TL)/$(CORPUS).tagged.new.$(TL): data-$(SL)-$(TL)/$(CORPUS).tagged.$(SL) data-$(SL)-$(TL)/$(CORPUS).tagged.$(TL)
	paste data-$(SL)-$(TL)/$(CORPUS).tagged.$(SL) data-$(SL)-$(TL)/$(CORPUS).tagged.$(TL) \
	| grep '<' \
	| cut -f2 \
	| sed 's/ /~/g' | sed 's/$$[^\^]*/$$ /g' > $@

data-$(SL)-$(TL)/$(CORPUS).tag-clean.$(SL) data-$(SL)-$(TL)/$(CORPUS).tag-clean.$(TL): data-$(SL)-$(TL)/$(CORPUS).tagged.new.$(SL) data-$(SL)-$(TL)/$(CORPUS).tagged.new.$(TL)
	perl $(MOSESDECODER)/clean-corpus-n.perl data-$(SL)-$(TL)/$(CORPUS).tagged.new $(SL) $(TL) data-$(SL)-$(TL)/$(CORPUS).tag-clean 1 40;

# ALIGN
model: data-$(SL)-$(TL)/$(CORPUS).tag-clean.$(SL) data-$(SL)-$(TL)/$(CORPUS).tag-clean.$(TL)
	-perl $(MOSESDECODER)/train-model.perl -external-bin-dir $(BIN_DIR) -corpus data-$(SL)-$(TL)/$(CORPUS).tag-clean \
	 -f $(TL) -e $(SL) -alignment grow-diag-final-and -reordering msd-bidirectional-fe \
	-lm 0:5:$(LM):0 2>&1

# EXTRACT AND TRIM
data-$(SL)-$(TL)/$(CORPUS).phrasetable.$(SL)-$(TL): model
	zcat giza.$(SL)-$(TL)/$(SL)-$(TL).A3.final.gz | $(SCRIPTS)/giza-to-moses.awk > $@
	cat data-$(SL)-$(TL)/$(CORPUS).phrasetable.$(SL)-$(TL) | sed 's/ ||| /\t/g' | cut -f 1 \
	| sed 's/~/ /g' | $(LEX_TOOLS)/multitrans $(DATA)/$(TL)-$(SL).autobil.bin -p -t > tmp1
	cat data-$(SL)-$(TL)/$(CORPUS).phrasetable.$(SL)-$(TL) | sed 's/ ||| /\t/g' | cut -f 2 \
	| sed 's/~/ /g' | $(LEX_TOOLS)/multitrans $(DATA)/$(SL)-$(TL).autobil.bin -p -t > tmp2
	cat data-$(SL)-$(TL)/$(CORPUS).phrasetable.$(SL)-$(TL) | sed 's/ ||| /\t/g' | cut -f 3 > tmp3
	cat data-$(SL)-$(TL)/$(CORPUS).phrasetable.$(SL)-$(TL) | sed 's/ ||| /\t/g' | cut -f 2 \
	| sed 's/~/ /g' | $(LEX_TOOLS)/multitrans $(DATA)/$(SL)-$(TL).autobil.bin -b -t > data-$(SL)-$(TL)/$(CORPUS).clean-biltrans.$(PAIR)
	paste tmp1 tmp2 tmp3 | sed 's/\t/ ||| /g' > data-$(SL)-$(TL)/$(CORPUS).phrasetable.$(SL)-$(TL)
	rm tmp1 tmp2 tmp3

# SENTENCES
data-$(SL)-$(TL)/$(CORPUS).candidates.$(SL)-$(TL): data-$(SL)-$(TL)/$(CORPUS).phrasetable.$(SL)-$(TL) data-$(SL)-$(TL)/$(CORPUS).clean-biltrans.$(PAIR)
	python3 $(SCRIPTS)/extract-sentences.py data-$(SL)-$(TL)/$(CORPUS).phrasetable.$(SL)-$(TL) \
			data-$(SL)-$(TL)/$(CORPUS).clean-biltrans.$(PAIR) > $@ 2>/dev/null

# FREQUENCY LEXICON
data-$(SL)-$(TL)/$(CORPUS).lex.$(SL)-$(TL): data-$(SL)-$(TL)/$(CORPUS).candidates.$(SL)-$(TL)
	python $(SCRIPTS)/extract-freq-lexicon.py data-$(SL)-$(TL)/$(CORPUS).candidates.$(SL)-$(TL) > $@ 2>/dev/null

# BILTRANS CANDIDATES
data-$(SL)-$(TL)/$(CORPUS).biltrans-entries.$(SL)-$(TL): data-$(SL)-$(TL)/$(CORPUS).phrasetable.$(SL)-$(TL) data-$(SL)-$(TL)/$(CORPUS).clean-biltrans.$(PAIR)
	python3 $(SCRIPTS)/extract-biltrans-candidates.py data-$(SL)-$(TL)/$(CORPUS).phrasetable.$(SL)-$(TL) data-$(SL)-$(TL)/$(CORPUS).clean-biltrans.$(PAIR) \
	> $@ 2>/dev/null

# NGRAM PATTERNS
data-$(SL)-$(TL)/$(CORPUS).ngrams.$(SL)-$(TL): data-$(SL)-$(TL)/$(CORPUS).lex.$(SL)-$(TL) data-$(SL)-$(TL)/$(CORPUS).candidates.$(SL)-$(TL)
	python $(SCRIPTS)/ngram-count-patterns.py data-$(SL)-$(TL)/$(CORPUS).lex.$(SL)-$(TL) data-$(SL)-$(TL)/$(CORPUS).candidates.$(SL)-$(TL) $(crisphold) 2>/dev/null > $@

# NGRAMS TO RULES
data-$(SL)-$(TL)/$(CORPUS).ngrams.$(SL)-$(TL).lrx: data-$(SL)-$(TL)/$(CORPUS).ngrams.$(SL)-$(TL)
	python3 $(SCRIPTS)/ngrams-to-rules.py data-$(SL)-$(TL)/$(CORPUS).ngrams.$(SL)-$(TL) $(crisphold) > $@ 2>/dev/null



