# ================================================================== #
#  OTA Security Compiler — Top-Level Makefile                         #
# ================================================================== #
#
#  Targets:
#    make build         Build the LLVM pass shared library
#    make analyze-secure     Run analysis on secure.c
#    make analyze-insecure   Run analysis on insecure.c
#    make test          Run both analyses
#    make emit-ir       Generate .ll files only
#    make clean         Remove build artifacts
#
# ================================================================== #

# Tools (override with environment variables)
CLANG   ?= clang
OPT     ?= opt
CMAKE   ?= cmake

# Paths
ROOT        := $(shell pwd)
PASS_DIR    := $(ROOT)/llvm-pass
BUILD_DIR   := $(PASS_DIR)/build
PASS_LIB    := $(BUILD_DIR)/OTASecurityPass.so
TEST_DIR    := $(ROOT)/tests
LOG_FILE    := $(ROOT)/secure_log.txt

# Test files
SECURE_C    := $(TEST_DIR)/secure.c
INSECURE_C  := $(TEST_DIR)/insecure.c
SECURE_LL   := $(TEST_DIR)/secure.ll
INSECURE_LL := $(TEST_DIR)/insecure.ll

.PHONY: all build emit-ir analyze-secure analyze-insecure test clean help

# ------------------------------------------------------------------ #
#  Default                                                             #
# ------------------------------------------------------------------ #

all: build test

help:
	@echo ""
	@echo "  OTA Security Compiler"
	@echo "  ====================="
	@echo ""
	@echo "  make build              Build the LLVM pass"
	@echo "  make emit-ir            Generate .ll files from test sources"
	@echo "  make analyze-secure     Run pass on secure.c"
	@echo "  make analyze-insecure   Run pass on insecure.c"
	@echo "  make test               Run both analyses"
	@echo "  make clean              Remove build artifacts"
	@echo ""

# ------------------------------------------------------------------ #
#  Build the LLVM pass                                                 #
# ------------------------------------------------------------------ #

build: $(PASS_LIB)

$(PASS_LIB): $(PASS_DIR)/TraversalPass.cpp $(PASS_DIR)/CMakeLists.txt
	@echo "[BUILD] Compiling OTASecurityPass..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) .. && $(MAKE)
	@echo "[BUILD] Done → $(PASS_LIB)"

# ------------------------------------------------------------------ #
#  Emit LLVM IR                                                        #
# ------------------------------------------------------------------ #

emit-ir: $(SECURE_LL) $(INSECURE_LL)

$(SECURE_LL): $(SECURE_C)
	@echo "[IR] Compiling secure.c → secure.ll"
	$(CLANG) -S -emit-llvm -O0 $< -o $@

$(INSECURE_LL): $(INSECURE_C)
	@echo "[IR] Compiling insecure.c → insecure.ll"
	$(CLANG) -S -emit-llvm -O0 $< -o $@

# ------------------------------------------------------------------ #
#  Run analysis                                                        #
# ------------------------------------------------------------------ #

analyze-secure: $(PASS_LIB) $(SECURE_LL)
	@echo ""
	@echo "╔══════════════════════════════════════════╗"
	@echo "║  Analyzing: secure.c                     ║"
	@echo "╚══════════════════════════════════════════╝"
	@echo ""
	$(OPT) -load-pass-plugin $(PASS_LIB) \
		-passes="ota-security" \
		-disable-output \
		$(SECURE_LL)
	@echo ""
	@echo "--- secure_log.txt ---"
	@cat $(LOG_FILE)

analyze-insecure: $(PASS_LIB) $(INSECURE_LL)
	@echo ""
	@echo "╔══════════════════════════════════════════╗"
	@echo "║  Analyzing: insecure.c                   ║"
	@echo "╚══════════════════════════════════════════╝"
	@echo ""
	$(OPT) -load-pass-plugin $(PASS_LIB) \
		-passes="ota-security" \
		-disable-output \
		$(INSECURE_LL)
	@echo ""
	@echo "--- secure_log.txt ---"
	@cat $(LOG_FILE)

test: analyze-secure analyze-insecure
	@echo ""
	@echo "All analyses complete."

# ------------------------------------------------------------------ #
#  Clean                                                               #
# ------------------------------------------------------------------ #

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TEST_DIR)/*.ll $(TEST_DIR)/*.bc $(TEST_DIR)/*.o
	rm -f $(TEST_DIR)/secure $(TEST_DIR)/insecure
	rm -f $(LOG_FILE)
	@echo "[CLEAN] Done."
