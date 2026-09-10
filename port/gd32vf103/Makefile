###############################################################################
# milliForth-style minimal FORTH for GD32VF103 (RISC-V).
#
# Windows/MinGW friendly: uses cmd `mkdir/del` and 8.3-safe flags.  Adjust
# RISCV_DIR / RISCV_PREFIX to your RISC-V toolchain install.
###############################################################################

RISCV_DIR    ?= C:/ProgramData/xpack-riscv-none-embed-gcc
RISCV_PREFIX ?= riscv-none-embed

CC      := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-gcc
AS      := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-gcc
LD      := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-gcc
SIZE    := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-size
OBJCOPY := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-objcopy

TARGET   := milliForth-gd32vf103
OBJDIR   := obj
LDSCRIPT := firmware/RISCV/env/GD32VF103x8.lds

# --- architecture --------------------------------------------------------------
ARCH   := -march=rv32imac -mabi=ilp32 -mcmodel=medlow
CFLAGS  = $(ARCH) -Os -ffunction-sections -fdata-sections -fno-common -g3 -Wall
CPPFLAGS = -I. -Iforth -Ibsp -Ifirmware/Include -Ifirmware/RISCV/drivers -Ifirmware/RISCV/env
LDFLAGS = $(ARCH) -T$(LDSCRIPT) -nostartfiles --specs=nano.specs --specs=nosys.specs -lgcc

# --- sources --------------------------------------------------------------------
C_SRC := $(wildcard firmware/Source/*.c) \
         bsp/usart_console.c \
         forth/forth.c \
         main.c
S_SRC := forth/forth.S forth/forth_boot.S \
         firmware/RISCV/env/entry.S firmware/RISCV/env/start.S

# map each source to an object (name kept; collisions are improbable here)
OBJS  := $(C_SRC:%.c=$(OBJDIR)/%.o) $(S_SRC:%.S=$(OBJDIR)/%.o)

all: $(TARGET).elf $(TARGET).bin
	$(SIZE) $(TARGET).elf

$(TARGET).elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $@ $<

$(OBJDIR)/%.o: %.c
	@if not exist "$(OBJDIR)/$(dir $@)" cmd /c "mkdir $(subst /,\,$(OBJDIR)/$(dir $@))" 2>nul || true
	$(CC) $(CFLAGS) $(CPPFLAGS) -c "$<" -o "$@"

$(OBJDIR)/%.o: %.S
	@if not exist "$(OBJDIR)/$(dir $@)" cmd /c "mkdir $(subst /,\,$(OBJDIR)/$(dir $@))" 2>nul || true
	$(AS) $(CFLAGS) $(CPPFLAGS) -x assembler-with-cpp -c "$<" -o "$@"

clean:
	@if exist "$(OBJDIR)" rmdir /s /q "$(OBJDIR)"
	@if exist "$(TARGET).elf" del /q "$(TARGET).elf"
	@if exist "$(TARGET).bin" del /q "$(TARGET).bin"

.PHONY: all clean