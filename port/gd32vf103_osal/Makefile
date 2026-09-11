###############################################################################
# OSAL 移植例程 for GD32VF103 (RISC-V Bumblebee 核)
#
# 目录:
#   osal/     - OSAL 核心层(平台无关)
#   hal/      - 移植层(中断/睡眠/机器定时器心跳)
#   osal_app/ - 应用任务
#   Firmware/ - GD32VF103 官方固件库(标准外设库 + RISCV 内核环境)
#
# Windows/MinGW friendly. Adjust RISCV_DIR / RISCV_PREFIX to your toolchain.
###############################################################################

RISCV_DIR    ?= C:/ProgramData/xpack-riscv-none-embed-gcc
RISCV_PREFIX ?= riscv-none-embed

CC      := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-gcc
AS      := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-gcc
LD      := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-gcc
SIZE    := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-size
OBJCOPY := $(RISCV_DIR)/bin/$(RISCV_PREFIX)-objcopy

TARGET   := osal-gd32vf103
OBJDIR   := obj

# 本地固件库路径
FW_STD     := Firmware/GD32VF103_standard_peripheral
FW_RISCV   := Firmware/RISCV
FW_ENV     := $(FW_RISCV)/env_Eclipse
LDSCRIPT   := $(FW_ENV)/GD32VF103x8.lds

# --- architecture --------------------------------------------------------------
ARCH     := -march=rv32imac -mabi=ilp32 -mcmodel=medlow
# GD32VF103V_EVAL: 板型宏, 决定 HXTAL 频率(8MHz); 换板请同步修改
CFLAGS   = $(ARCH) -Os -ffunction-sections -fdata-sections -fno-common -g3 -Wall \
           -DGD32VF103V_EVAL
CPPFLAGS = -Iosal -Ihal -Iosal_app \
           -I$(FW_STD) \
           -I$(FW_STD)/Include \
           -I$(FW_RISCV)/drivers
LDFLAGS  = $(ARCH) -T$(LDSCRIPT) -nostartfiles --specs=nano.specs --specs=nosys.specs -lgcc

# --- sources --------------------------------------------------------------------
# 本目录源码: OSAL 核心 + 移植层 + 应用任务
C_SRC := $(wildcard osal/*.c) \
         $(wildcard hal/*.c) \
         $(wildcard osal_app/*.c)
# 固件库源码: 标准外设库 + CLINT/ECLIC 驱动 + newlib 存根(printf 重定向)
#            + 启动环境(init.c 提供 _init: SystemInit + ECLIC 初始化)
C_SRC += $(wildcard $(FW_STD)/Source/*.c) \
         $(FW_RISCV)/drivers/n200_func.c \
         $(wildcard $(FW_RISCV)/stubs/*.c) \
         $(FW_ENV)/init.c \
         $(FW_ENV)/handlers.c
S_SRC := $(FW_ENV)/entry.S \
         $(FW_ENV)/start.S

OBJS  := $(addprefix $(OBJDIR)/,$(notdir $(C_SRC:.c=.o) $(S_SRC:.S=.o)))
VPATH := osal hal osal_app \
         $(FW_STD)/Source \
         $(FW_RISCV)/drivers \
         $(FW_RISCV)/stubs \
         $(FW_ENV)

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

.PHONY: all clean
