.PHONY: all
all:
	@echo "Valid targets are:"
	@echo "    build - creates the instruction_set.db"
	@echo "    clean - removes build files"

.PHONY: build
build: instruction_set.db
	true

.PHONY: clean
clean:
	rm -f instruction_set.db

instruction_set.db:
	python3 make_db.py
