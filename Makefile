SHELL:=/bin/bash # Use bash syntax, mitigates dash's printf on Debian


.PHONY: help
help:
	@echo
	@echo "▍Help"
	@echo "▀▀▀▀▀▀"
	@echo
	@echo "Available targets:"
	@echo "    debug:       Build debug binary, default."
	@echo "    release:     Build optimized release binary."
	@echo
	@echo "    clean:              Clean all generated files."
	@echo


all: debug


.PHONY: debug
debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build


.PHONY: release
release:
	cmake -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build


.PHONY: clean
clean:
	rm -rf build
