# GD32E230 (Cortex-M23) OSAL 移植工程
#
# 用法:
#   make                构建 gd32e230-osal-example.elf / .bin
#   make clean          清理编译产物
#   make list           列出参与编译的源文件
#
# 依赖: arm-none-eabi-gcc (ARM GNU 工具链)
# 串口: USART0 PA9=TX, 波特率 115200, 8N1

TARGET      = gd32e230-osal-example

# ARM GNU 工具链路径
ARM_TC      = D:/arm_none_eabi_gcc/bin

CC          = $(ARM_TC)/arm-none-eabi-gcc
OBJCOPY     = $(ARM_TC)/arm-none-eabi-objcopy
SIZE        = $(ARM_TC)/arm-none-eabi-size

TOP         = .

OBJ_DIR     = $(TOP)/obj

# GD32E23x 标准外设库路径
FW_DIR      = $(TOP)/firmware
FW_INC      = $(FW_DIR)/CMSIS
FW_CMSIS    = $(FW_DIR)/CMSIS/GD32E23x/Include
FW_STDINC   = $(FW_DIR)/CMSIS/GD32E23x/Include
FW_STDSRC   = $(FW_DIR)/Std32E23x

# 头文件包含目录
INC_FLAGS   += -I $(TOP)/osal_app
INC_FLAGS   += -I $(TOP)/osal
INC_FLAGS   += -I $(TOP)/hal
INC_FLAGS   += -I $(TOP)/board
INC_FLAGS   += -I $(FW_INC)
INC_FLAGS   += -I $(FW_CMSIS)
INC_FLAGS   += -I $(FW_STDINC)

# 链接脚本与启动文件: 使用 GD32E23x 官方 GCC 版本 (gd32e230x8: 64KB Flash/8KB SRAM)
LINKER      = $(TOP)/firmware/CMSIS/GD32E23x/Source/GCC/Ld/gd32e230x8_flash.ld
STARTUP     = $(TOP)/firmware/CMSIS/GD32E23x/Source/GCC/startup_gd32e23x.S

# 编译标志: Cortex-M23, 软浮点, nano 库
CFLAGS      = -mcpu=cortex-m23 -mthumb -mfloat-abi=soft
CFLAGS     += -Wall -g -O0 -std=gnu11
CFLAGS     += -ffunction-sections -fdata-sections
CFLAGS     += -DFLASH_START=0x08000000 -DFLASH_SIZE=64K -DUSE_GD32_FW

LDFLAGS     = -mcpu=cortex-m23 -mthumb -mfloat-abi=soft -T $(LINKER) -Wl,--gc-sections -Wl,-Map=$(TARGET).map
SPECS       = --specs=nano.specs --specs=nosys.specs

# 源文件
C_SRC       = $(wildcard $(TOP)/osal_app/*.c)
C_SRC      += $(wildcard $(TOP)/osal/*.c)
C_SRC      += $(wildcard $(TOP)/hal/*.c)
C_SRC      += $(wildcard $(TOP)/board/*.c)
C_SRC      += $(wildcard $(FW_STDSRC)/*.c)
# system_gd32e23x.c 在 firmware 内, 由下方独立规则编译, 不放进 C_SRC

FW_SYS_SRC  = $(FW_DIR)/CMSIS/GD32E23x/Source/system_gd32e23x.c

C_OBJ       = $(patsubst $(TOP)/%.c,$(OBJ_DIR)/%.o,$(C_SRC))
C_OBJ      += $(OBJ_DIR)/system_gd32e23x.o
ALL_OBJ     = $(C_OBJ) $(OBJ_DIR)/startup.o

.PHONY: all clean list size dirs

# 默认目标
all: dirs $(TARGET).bin
	@$(SIZE) $(TARGET).elf

# 预创建对象目录树
dirs:
	@if not exist "$(OBJ_DIR)\\osal_app" mkdir "$(OBJ_DIR)\\osal_app"
	@if not exist "$(OBJ_DIR)\\osal" mkdir "$(OBJ_DIR)\\osal"
	@if not exist "$(OBJ_DIR)\\hal" mkdir "$(OBJ_DIR)\\hal"
	@if not exist "$(OBJ_DIR)\\board" mkdir "$(OBJ_DIR)\\board"
	@if not exist "$(OBJ_DIR)\\firmware\\Std32E23x" mkdir "$(OBJ_DIR)\\firmware\\Std32E23x"
	@if not exist "$(OBJ_DIR)\\firmware\\CMSIS\\GD32E23x\\Source" mkdir "$(OBJ_DIR)\\firmware\\CMSIS\\GD32E23x\\Source"

$(TARGET).elf: $(ALL_OBJ)
	@echo "linking $@"
	@$(CC) $(ALL_OBJ) -o $@ $(LDFLAGS) $(SPECS)

# system_gd32e23x.c (CMSIS 源目录, 独立规则, 避免与 %.o 规则路径冲突)
$(OBJ_DIR)/system_gd32e23x.o: $(FW_SYS_SRC)
	@echo "building system_gd32e23x.c"
	@$(CC) -c $(CFLAGS) $(INC_FLAGS) -o $@ $<

$(TARGET).bin: $(TARGET).elf
	@echo "generating $@"
	@$(OBJCOPY) -O binary $< $@

$(OBJ_DIR)/%.o: $(TOP)/%.c
	@echo "building $<"
	@$(CC) -c $(CFLAGS) $(INC_FLAGS) -o $@ $<

$(OBJ_DIR)/startup.o: $(STARTUP)
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	@echo "building startup_gd32e230.s"
	@$(CC) -c $(CFLAGS) -o $@ $<

list:
	@echo $(ALL_OBJ)

# Windows(MinGW)下无 rm, 用 shell 命令替代
clean:
	@if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"
	@if exist "$(TARGET).elf" del /q "$(TARGET).elf"
	@if exist "$(TARGET).bin" del /q "$(TARGET).bin"
	@if exist "$(TARGET).map" del /q "$(TARGET).map"