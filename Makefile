# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2023-2024

export BLOCKSDS			?= /opt/blocksds/core
export BLOCKSDSEXT		?= /opt/blocksds/external

export WONDERFUL_TOOLCHAIN	?= /opt/wonderful
ARM_NONE_EABI_PATH	?= $(WONDERFUL_TOOLCHAIN)/toolchain/gcc-arm-none-eabi/bin/

# User config
# ===========

NAME		:= UndertaleNDS

GAME_TITLE	:= Undertale NDS
GAME_SUBTITLE	:= Built with BlocksDS
GAME_AUTHOR	:= TobyFox, Cervi
GAME_ICON	:= icon.bmp

# DLDI and internal SD slot of DSi
# --------------------------------

# Root folder of the SD image
SDROOT		:= sdroot
# Name of the generated image it "DSi-1.sd" for no$gba in DSi mode
SDIMAGE		:= image.bin

# Source code paths
# -----------------

SOURCEDIRS	:= source
INCLUDEDIRS	:= include
GFXDIRS		:=
BINDIRS		:=
AUDIODIRS	:=
# A single directory that is the root of NitroFS:
NITROFSDIR	:= nitrofs

# Defines passed to all files
# ---------------------------

DEFINES		:= -DBLOCKSDS_SDK -DPICOLIBC_LONG_LONG_PRINTF_SCANF

# Libraries
# ---------

LIBS		:= -lmm9 -lnds9
LIBDIRS		:= $(BLOCKSDS)/libs/maxmod \
		   $(BLOCKSDS)/libs/libnds

# Build artifacts
# ---------------

BUILDDIR	:= build/$(NAME)
ELF		:= build/$(NAME).elf
DUMP		:= build/$(NAME).dump
MAP		:= build/$(NAME).map
ROM		:= $(NAME).nds

# If NITROFSDIR is set, the soundbank created by mmutil will be saved to NitroFS
SOUNDBANKINFODIR	:= $(BUILDDIR)/maxmod
ifeq ($(strip $(NITROFSDIR)),)
    SOUNDBANKDIR	:= $(BUILDDIR)/maxmod
else
    SOUNDBANKDIR	:= $(BUILDDIR)/maxmod_nitrofs
endif

# Tools
# -----

PREFIX		:= $(ARM_NONE_EABI_PATH)arm-none-eabi-
CC		:= $(PREFIX)gcc
CXX		:= $(PREFIX)g++
LD		:= $(PREFIX)gcc
OBJDUMP		:= $(PREFIX)objdump
MKDIR		:= mkdir
RM		:= rm -rf

# Verbose flag
# ------------

ifeq ($(VERBOSE),1)
V		:=
else
V		:= @
endif

# Source files
# ------------

ifneq ($(BINDIRS),)
    SOURCES_BIN	:= $(shell find -L $(BINDIRS) -name "*.bin")
    INCLUDEDIRS	+= $(addprefix $(BUILDDIR)/,$(BINDIRS))
endif
ifneq ($(GFXDIRS),)
    SOURCES_PNG	:= $(shell find -L $(GFXDIRS) -name "*.png")
    INCLUDEDIRS	+= $(addprefix $(BUILDDIR)/,$(GFXDIRS))
endif
ifneq ($(AUDIODIRS),)
    SOURCES_AUDIO	:= $(shell find -L $(AUDIODIRS) -regex '.*\.\(it\|mod\|s3m\|wav\|xm\)')
    ifneq ($(SOURCES_AUDIO),)
        INCLUDEDIRS	+= $(SOUNDBANKINFODIR)
    endif
endif

SOURCES_S	:= $(shell find -L $(SOURCEDIRS) -name "*.s")
SOURCES_C	:= $(shell find -L $(SOURCEDIRS) -name "*.c")
SOURCES_CPP	:= $(shell find -L $(SOURCEDIRS) -name "*.cpp")

# Compiler and linker flags
# -------------------------

ARCH		:= -mthumb -mcpu=arm946e-s+nofp

SPECS		:= $(BLOCKSDS)/sys/crts/ds_arm9.specs

WARNFLAGS	:= -Wall

ifeq ($(SOURCES_CPP),)
	LIBS	+= -lc
else
	LIBS	+= -lstdc++ -lc
endif

INCLUDEFLAGS	:= $(foreach path,$(INCLUDEDIRS),-I$(path)) \
		   $(foreach path,$(LIBDIRS),-I$(path)/include)

LIBDIRSFLAGS	:= $(foreach path,$(LIBDIRS),-L$(path)/lib)

ASFLAGS		+= -x assembler-with-cpp $(INCLUDEFLAGS) $(DEFINES) \
		   $(ARCH) -ffunction-sections -fdata-sections \
		   -specs=$(SPECS)

CFLAGS		+= -std=gnu17 $(WARNFLAGS) $(INCLUDEFLAGS) $(DEFINES) \
		   $(ARCH) -O2 -ffunction-sections -fdata-sections \
		   -specs=$(SPECS)

CXXFLAGS	+= -std=gnu++17 $(WARNFLAGS) $(INCLUDEFLAGS) $(DEFINES) \
		   $(ARCH) -O2 -ffunction-sections -fdata-sections \
		   -fno-exceptions -fno-rtti \
		   -specs=$(SPECS)

LDFLAGS		:= $(ARCH) $(LIBDIRSFLAGS) -Wl,-Map,$(MAP) $(DEFINES) \
		   -Wl,--start-group $(LIBS) -Wl,--end-group -specs=$(SPECS)

# Intermediate build files
# ------------------------

OBJS_ASSETS	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_BIN))) \
		   $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_PNG)))

HEADERS_ASSETS	:= $(patsubst %.bin,%_bin.h,$(addprefix $(BUILDDIR)/,$(SOURCES_BIN))) \
		   $(patsubst %.png,%.h,$(addprefix $(BUILDDIR)/,$(SOURCES_PNG)))

ifneq ($(SOURCES_AUDIO),)
    ifeq ($(strip $(NITROFSDIR)),)
        OBJS_ASSETS		+= $(SOUNDBANKDIR)/soundbank.c.o
    endif
    HEADERS_ASSETS	+= $(SOUNDBANKINFODIR)/soundbank.h
endif

OBJS_SOURCES	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_S))) \
		   $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_C))) \
		   $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_CPP)))

OBJS		:= $(OBJS_ASSETS) $(OBJS_SOURCES)

DEPS		:= $(OBJS:.o=.d)

# Targets
# -------

.PHONY: all clean dump dldipatch sdimage

all: $(ROM)

ifneq ($(strip $(NITROFSDIR)),)
# Additional arguments for ndstool
NDSTOOL_ARGS	:= -d $(NITROFSDIR)

ifneq ($(SOURCES_AUDIO),)
    NDSTOOL_ARGS	+= -d $(SOUNDBANKDIR)
endif

# Make the NDS ROM depend on the filesystem only if it is needed
$(ROM): $(NITROFSDIR)
endif

# Combine the title strings
ifeq ($(strip $(GAME_SUBTITLE)),)
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_AUTHOR)
else
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_SUBTITLE);$(GAME_AUTHOR)
endif

$(ROM): $(ELF) $(NITROFSDIR)/stamp_file.txt
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 $(BLOCKSDS)/sys/default_arm7/arm7.elf -9 $(ELF) \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		$(NDSTOOL_ARGS)

$(ELF): $(OBJS)
	@echo "  LD      $@"
	$(V)$(LD) -o $@ $(OBJS) $(LDFLAGS)

$(DUMP): $(ELF)
	@echo "  OBJDUMP   $@"
	$(V)$(OBJDUMP) -h -C -S $< > $@

dump: $(DUMP)

clean:
	@echo "  CLEAN"
	$(V)$(RM) $(ROM) $(DUMP) build $(SDIMAGE)

sdimage:
	@echo "  MKFATIMG $(SDIMAGE) $(SDROOT)"
	$(V)$(BLOCKSDS)/tools/mkfatimg/mkfatimg -t $(SDROOT) $(SDIMAGE)

dldipatch: $(ROM)
	@echo "  DLDIPATCH $(ROM)"
	$(V)$(BLOCKSDS)/tools/dldipatch/dldipatch patch \
		$(BLOCKSDS)/sys/dldi_r4/r4tf.dldi $(ROM)

# Rules
# -----

$(BUILDDIR)/%.s.o : %.s
	@echo "  AS      $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(ASFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.c.o : %.c
	@echo "  CC      $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.arm.c.o : %.arm.c
	@echo "  CC      $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -marm -mlong-calls -c -o $@ $<

$(BUILDDIR)/%.cpp.o : %.cpp
	@echo "  CXX     $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.arm.cpp.o : %.arm.cpp
	@echo "  CXX     $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CXX) $(CXXFLAGS) -MMD -MP -marm -mlong-calls -c -o $@ $<

$(BUILDDIR)/%.bin.o $(BUILDDIR)/%_bin.h : %.bin
	@echo "  BIN2C   $<"
	@$(MKDIR) -p $(@D)
	$(V)$(BLOCKSDS)/tools/bin2c/bin2c $< $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $(BUILDDIR)/$*.bin.o $(BUILDDIR)/$*_bin.c

$(BUILDDIR)/%.png.o $(BUILDDIR)/%.h : %.png %.grit
	@echo "  GRIT    $<"
	@$(MKDIR) -p $(@D)
	$(V)$(BLOCKSDS)/tools/grit/grit $< -ftc -W1 -o$(BUILDDIR)/$*
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $(BUILDDIR)/$*.png.o $(BUILDDIR)/$*.c
	$(V)touch $(BUILDDIR)/$*.png.o $(BUILDDIR)/$*.h

ifneq ($(SOURCES_AUDIO),)

$(SOUNDBANKINFODIR)/soundbank.h: $(SOURCES_AUDIO)
	@echo "  MMUTIL  $^"
	@$(MKDIR) -p $(SOUNDBANKDIR)
	@$(MKDIR) -p $(SOUNDBANKINFODIR)
	@$(BLOCKSDS)/tools/mmutil/mmutil $^ -d \
		-o$(SOUNDBANKDIR)/soundbank.bin -h$(SOUNDBANKINFODIR)/soundbank.h

ifeq ($(strip $(NITROFSDIR)),)
$(SOUNDBANKDIR)/soundbank.c.o: $(SOUNDBANKINFODIR)/soundbank.h
	@echo "  BIN2C   soundbank.bin"
	$(V)$(BLOCKSDS)/tools/bin2c/bin2c $(SOUNDBANKDIR)/soundbank.bin \
		$(SOUNDBANKDIR)
	@echo "  CC.9    soundbank_bin.c"
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $(SOUNDBANKDIR)/soundbank.c.o \
		$(SOUNDBANKDIR)/soundbank_bin.c
endif

endif

# All assets must be built before the source code
# -----------------------------------------------

$(SOURCES_S) $(SOURCES_C) $(SOURCES_CPP): $(HEADERS_ASSETS)

# Include dependency files if they exist
# --------------------------------------

-include $(DEPS)
