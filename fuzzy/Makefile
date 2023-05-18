EXTENSION = fuzzy
MODULE_big = fuzzy
DATA = fuzzy--1.0.sql
OBJS = fuzzy.o algorithms/bitap.o algorithms/levinshtein.o algorithms/simple_match.o algorithms/jaro_winkler.o algorithms/wagner_fisher.o algorithms/trigram_match.o algorithms/utils.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)