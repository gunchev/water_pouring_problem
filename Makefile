SHELL:=/bin/bash # Use bash syntax, mitigates dash's printf on Debian


.PHONY: help
help:
	@echo
	@echo "▍Help"
	@echo "▀▀▀▀▀▀"
	@echo
	@echo "Available targets:"
	@echo "    help:        This help message, default."
	@echo
	@echo "    debug:       Build debug binary."
	@echo "    release:     Build optimized release binary."
	@echo
	@echo "    clean:       Clean all generated files."
	@echo


all: debug


.PHONY: debug
debug:
	cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/debug


.PHONY: release
release:
	cmake -B build/release -DCMAKE_BUILD_TYPE=Release
	cmake --build build/release


.PHONY: clean
clean:
	rm -rf build


.PHONY: dc
dc: clean
