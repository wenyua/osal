#工程名称
TARGET 			= linux-osal-example

#设置编译器
#CC    			= arm-linux-gnueabihf-gcc
CC     			= gcc

#获取当前工作目录
TOP=.

#设置源文件后缀，c或cpp
EXT				= c

#设置源文件搜索路径
VPATH			+= $(TOP)/app:$(TOP)/hal:$(TOP)/osal

#设置自定义源文件目录
APP_DIR			= $(TOP)/app
HARD_DIR		= $(TOP)/hal

#设置中间目标文件目录
OBJ_DIR			= $(TOP)/obj

#设定头文件包含目录
INC_FLAGS 		+= -I $(TOP)/app
INC_FLAGS 		+= -I $(TOP)/osal
INC_FLAGS 		+= -I $(TOP)/hal

#编译选项
CFLAGS 			+= -W -g -O0 -std=gnu11

#链接选项
LFLAGS 			+= -pthread

#固定源文件添加
C_SRC			= $(shell find $(TOP)/app -name '*.$(EXT)')
C_SRC			+= $(shell find $(TOP)/hal -name '*.$(EXT)')
C_SRC			+= $(shell find $(TOP)/osal -name '*.$(EXT)')

#中间目标文件
C_SRC_NODIR		= $(notdir $(C_SRC))
C_OBJ 			= $(patsubst %.$(EXT), $(OBJ_DIR)/%.o,$(C_SRC_NODIR))

#依赖文件
C_DEP			= $(patsubst %.$(EXT), $(OBJ_DIR)/%.d,$(C_SRC_NODIR))

.PHONY: all clean rebuild ctags gd32 clean-gd32 rebuild-gd32

all:$(C_OBJ)
	@echo "linking object to $(TARGET).elf"
	@$(CC) $(C_OBJ) -o $(TARGET).elf $(LFLAGS)

$(OBJ_DIR)/%.o:%.$(EXT)
	@mkdir -p obj
	@echo "building $<"
	@$(CC) -c $(CFLAGS) $(INC_FLAGS) -o $@ $<

-include $(C_DEP)
$(OBJ_DIR)/%.d:%.$(EXT)
	@mkdir -p obj
	@echo "making $@"
	@set -e;rm -f $@;$(CC) -MM $(CFLAGS) $(INC_FLAGS) $< > $@.$$$$;sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $(OBJ_DIR)/\1.d:,g' < $@.$$$$ > $@;rm -f $@.$$$$

#===========================================================================
# GD32E230 适配编译目标（依赖 arm-none-eabi-gcc，Cortex-M23 内核）
#   make gd32         构建 GD32E230 固件 gd32e230-osal-example.elf/.bin
#   make clean-gd32   清理 GD32E230 编译产物
#===========================================================================
GD32_CC  		= arm-none-eabi-gcc
GD32_TARGET 	= gd32e230-osal-example
GD32_OBJ_DIR	= $(TOP)/obj_gd32
GD32_LINKER 	= $(TOP)/gd32e230/gd32e230.ld
GD32_STARTUP	= $(TOP)/gd32e230/startup_gd32e230.s

GD32_CFLAGS  	= -mcpu=cortex-m23 -mthumb -mfloat-abi=soft -Wall -g -O0 -std=gnu11
GD32_CFLAGS  	+= -ffunction-sections -fdata-sections
GD32_LDFLAGS  	= -T $(GD32_LINKER) -Wl,--gc-sections -Map $(GD32_TARGET).map
GD32_SPECS  	= --specs=nano.specs --specs=nosys.specs

GD32_SRC_NODIR	= $(notdir $(C_SRC))
GD32_OBJ     	= $(patsubst %.$(EXT), $(GD32_OBJ_DIR)/%.o,$(GD32_SRC_NODIR))
GD32_OBJ     	+= $(GD32_OBJ_DIR)/startup_gd32e230.o

gd32: $(GD32_OBJ)
	@mkdir -p $(GD32_OBJ_DIR)
	@echo "linking object to $(GD32_TARGET).elf"
	@$(GD32_CC) $(GD32_OBJ) -o $(GD32_TARGET).elf $(GD32_LDFLAGS) $(GD32_SPECS)
	@$(GD32_CC) -objcopy -O binary $(GD32_TARGET).elf $(GD32_TARGET).bin

$(GD32_OBJ_DIR)/%.o:%.$(EXT)
	@mkdir -p $(GD32_OBJ_DIR)
	@echo "building $<"
	@$(GD32_CC) -c $(GD32_CFLAGS) $(INC_FLAGS) -o $@ $<

$(GD32_OBJ_DIR)/startup_gd32e230.o: $(GD32_STARTUP)
	@mkdir -p $(GD32_OBJ_DIR)
	@echo "building startup_gd32e230.s"
	@$(GD32_CC) -c $(GD32_CFLAGS) -o $@ $<

clean-gd32:
	-rm -rf $(GD32_OBJ_DIR)
	-rm -f $(GD32_TARGET).elf $(GD32_TARGET).bin $(GD32_TARGET).map

rebuild-gd32: clean-gd32 gd32

clean:
	-rm -f obj/*
	-rm -f $(shell find ./ -name '*.elf')

rebuild: clean all

ctags:
	@ctags -R `pwd`
	@echo "making tags file"
