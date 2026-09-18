# ==============================================================================
# ARM GNU 交叉编译工具链文件（STM32F407, Cortex-M4F）
#
# 工具链查找顺序：
#   1. 缓存变量 ARM_TOOLCHAIN_DIR（手动指定，指向 gnu 工具链根目录）
#   2. 环境变量 CUBE_BUNDLE_PATH 下的 gnu-tools-for-stm32/<版本>（ST 扩展 Bundle）
#   3. 系统 PATH 中的 arm-none-eabi-gcc
# ==============================================================================

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ---------- 1) 解析工具链根目录 ----------
if(NOT DEFINED ARM_TOOLCHAIN_DIR)
  if(DEFINED ENV{CUBE_BUNDLE_PATH} AND EXISTS "$ENV{CUBE_BUNDLE_PATH}/gnu-tools-for-stm32")
    file(GLOB _gnu_versions LIST_DIRECTORIES true "$ENV{CUBE_BUNDLE_PATH}/gnu-tools-for-stm32/*")
    list(FILTER _gnu_versions INCLUDE REGEX "gnu-tools-for-stm32/[0-9]")
    list(SORT _gnu_versions ORDER DESCENDING)
    if(_gnu_versions)
      list(GET _gnu_versions 0 ARM_TOOLCHAIN_DIR)
    endif()
  endif()
endif()

# ---------- 2) 定位编译器 ----------
if(DEFINED ARM_TOOLCHAIN_DIR)
  set(_tool_hints "${ARM_TOOLCHAIN_DIR}/bin")
  find_program(CMAKE_C_COMPILER   NAMES arm-none-eabi-gcc   PATHS ${_tool_hints} NO_DEFAULT_PATH)
  find_program(CMAKE_CXX_COMPILER NAMES arm-none-eabi-g++   PATHS ${_tool_hints} NO_DEFAULT_PATH)
  find_program(CMAKE_ASM_COMPILER NAMES arm-none-eabi-gcc   PATHS ${_tool_hints} NO_DEFAULT_PATH)
  find_program(CMAKE_OBJCOPY      NAMES arm-none-eabi-objcopy PATHS ${_tool_hints} NO_DEFAULT_PATH)
  find_program(CMAKE_SIZE         NAMES arm-none-eabi-size  PATHS ${_tool_hints} NO_DEFAULT_PATH)
endif()

# 兜底：从系统 PATH 查找
function(_tool_fallback _var _name)
  if(NOT DEFINED ${_var})
    find_program(${_var} NAMES ${_name})
  elseif(${_var} STREQUAL "${_var}-NOTFOUND")
    unset(${_var} CACHE)
    find_program(${_var} NAMES ${_name})
  endif()
endfunction()

_tool_fallback(CMAKE_C_COMPILER   arm-none-eabi-gcc)
_tool_fallback(CMAKE_CXX_COMPILER arm-none-eabi-g++)
_tool_fallback(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
_tool_fallback(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
_tool_fallback(CMAKE_SIZE         arm-none-eabi-size)

if(NOT CMAKE_C_COMPILER)
  message(FATAL_ERROR "未找到 arm-none-eabi-gcc。请设置 ARM_TOOLCHAIN_DIR 或确认 CUBE_BUNDLE_PATH。")
endif()

# ---------- 3) CPU 编译参数 ----------
set(_cpu_flags "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS_INIT   "${_cpu_flags} -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS_INIT "${_cpu_flags} -fdata-sections -ffunction-sections")
set(CMAKE_ASM_FLAGS_INIT "${_cpu_flags} -x assembler-with-cpp")
