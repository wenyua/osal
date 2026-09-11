###############################################################################
# OSAL 精简移植 for CH32V003 (青稞 V2A, rv32ec, 16KB Flash / 2KB RAM)
#
# 目录:
#   osal/     - OSAL 核心层(堆已裁到 512B)
#   hal/      - 移植层(中断/睡眠/STK 心跳)
#   osal_app/ - 应用任务(无 printf, 轻量串口输出)
#   wch/      - 最小启动文件/寄存器定义/链接脚本(不依赖 WCH 官方库)
###############################################################################

RISCV_DIR    ?= C:/ProgramData/xpack-riscv-none-embed-gcc
RISCV_PREFIX ?= riscv-none-embed

CC      := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-gcc
AS      := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-gcc
LD      := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-gcc
SIZE    := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-size
OBJCOPY := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-objcopy

TARGET   := osal-ch32v003
OBJDIR   := obj
LDSCRIPT := wch/ch32v003.ld

# --- architecture ---------------------------------------------------------------
# CH32V003 青稞核: rv32ec(E=16个寄存器, C=压缩指令, 无M扩展)
ARCH     := -march=rv32ec -mabi=ilp32e -mcmodel=medlow -msmall-data-limit=0
CFLAGS   = $(ARCH) -Os -ffunction-sections -fdata-sections -fno-common -g3 -Wall
CPPFLAGS = -Iosal -Ihal -Iosal_app -Iwch
LDFLAGS  = $(ARCH) -T$(LDSCRIPT) -nostartfiles --specs=nano.specs --specs=nosys.specs \
           -Wl,--gc-sections -Wl,-Map=$(TARGET).map

# --- sources --------------------------------------------------------------------
C_SRC := $(wildcard osal/*.c) \
         $(wildcard hal/*.c) \
         $(wildcard osal_app/*.c)
S_SRC := wch/startup_ch32v003.S

OBJS  := $(addprefix $(OBJDIR)/,$(notdir $(C_SRC:.c=.o) $(S_SRC:.S=.o)))
VPATH := osal hal osal_app wch

all: $(TARGET).elf $(TARGET).bin
	$(SIZE) $(TARGET).elf

$(TARGET).elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $@ $<

$(OBJDIR)/%.o: %.c
	@if not exist "$(OBJDIR)" cmd /c "mkdir $(OBJDIR)" 2>nul || true
	$(CC) $(CFLAGS) $(CPPFLAGS) -c "$<" -o "$@"

$(OBJDIR)/%.o: %.S
	@if not exist "$(OBJDIR)" cmd /c "mkdir $(OBJDIR)" 2>nul || true
	$(AS) $(CFLAGS) $(CPPFLAGS) -x assembler-with-cpp -c "$<" -o "$@"

clean:
	@if exist "$(OBJDIR)" rmdir /s /q "$(OBJDIR)"
	@if exist "$(TARGET).elf" del /q "$(TARGET).elf"
	@if exist "$(TARGET).bin" del /q "$(TARGET).bin"
	@if exist "$(TARGET).map" del /q "$(TARGET).map"

.PHONY: all clean
