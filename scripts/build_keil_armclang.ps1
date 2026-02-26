param(
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"

if ($Configuration -ne "Debug") {
    throw "Only Debug is supported in this script."
}

$armclang = "C:\Keil_v5\ARM\ARMCLANG\bin\armclang.exe"
$armasm   = "C:\Keil_v5\ARM\ARMCLANG\bin\armasm.exe"
$armlink  = "C:\Keil_v5\ARM\ARMCLANG\bin\armlink.exe"
$fromelf  = "C:\Keil_v5\ARM\ARMCLANG\bin\fromelf.exe"

New-Item -ItemType Directory -Force Build\Obj | Out-Null

@'
LR_IROM1 0x08000000 0x00040000  {
  ER_IROM1 0x08000000 0x00040000  {
    *.o (RESET, +First)
    *(InRoot$$Sections)
    .ANY (+RO)
  }
  RW_IRAM1 0x20000000 0x00010000  {
    .ANY (+RW +ZI)
  }
}
'@ | Set-Content -Encoding ascii Build\song.sct

Remove-Item Build\Obj\*.o -ErrorAction SilentlyContinue

$includePaths = @(
    "Core\Inc",
    "Drivers\STM32F4xx_HAL_Driver\Inc",
    "Drivers\STM32F4xx_HAL_Driver\Inc\Legacy",
    "Drivers\CMSIS\Device\ST\STM32F4xx\Include",
    "Drivers\CMSIS\Include",
    "MDK-ARM\RTE\Device\STM32F401VCTx"
)

$commonArgs = @(
    "--target=arm-arm-none-eabi",
    "-mcpu=cortex-m4",
    "-mthumb",
    "-mfpu=fpv4-sp-d16",
    "-mfloat-abi=softfp",
    "-std=c99",
    "-O0",
    "-g",
    "-ffunction-sections",
    "-fdata-sections",
    "-DSTM32F401xC",
    "-DUSE_HAL_DRIVER"
)

$incArgs = @()
foreach ($p in $includePaths) { $incArgs += @("-I", $p) }

$sources = @(
    "Core\Src\main.c",
    "Core\Src\motor.c",
    "Core\Src\sensors.c",
    "Core\Src\seven_seg.c",
    "Core\Src\buzzer.c",
    "Core\Src\indicators.c",
    "Core\Src\bluetooth.c",
    "Core\Src\navigation.c",
    "Core\Src\lcd1602.c",
    "Core\Src\stm32f4xx_it.c",
    "Core\Src\stm32f4xx_hal_msp.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_cortex.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_dma.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_dma_ex.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_exti.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_flash.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_flash_ex.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_flash_ramfunc.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_gpio.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_pwr.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_pwr_ex.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_rcc.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_rcc_ex.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_adc.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_adc_ex.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_tim.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_tim_ex.c",
    "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal_uart.c",
    "MDK-ARM\RTE\Device\STM32F401VCTx\system_stm32f4xx.c"
)

$objects = @()
foreach ($src in $sources) {
    $obj = "Build\Obj\" + [System.IO.Path]::GetFileNameWithoutExtension($src) + ".o"
    & $armclang -c @commonArgs @incArgs $src -o $obj
    if ($LASTEXITCODE -ne 0) { throw "Compile failed: $src" }
    $objects += $obj
}

$startupObj = "Build\Obj\startup_stm32f401xc.o"
& $armasm --cpu Cortex-M4.fp.sp --apcs=interwork --diag_suppress=1296 --diag_suppress=1431 "MDK-ARM\RTE\Device\STM32F401VCTx\startup_stm32f401xc.s" -o $startupObj
if ($LASTEXITCODE -ne 0) { throw "Assemble failed: startup_stm32f401xc.s" }

$linkArgs = @(
    "--cpu=Cortex-M4.fp.sp",
    "--strict",
    "--scatter=Build\song.sct",
    "--entry=Reset_Handler",
    "--summary_stderr",
    "--map",
    "--list=Build\song.map",
    "--output=Build\song.axf"
) + @($startupObj) + $objects

& $armlink @linkArgs
if ($LASTEXITCODE -ne 0) { throw "Link failed" }

& $fromelf --i32combined --output Build\song.hex Build\song.axf
if ($LASTEXITCODE -ne 0) { throw "HEX generation failed" }

Write-Host "Build completed: Build\\song.axf, Build\\song.hex, Build\\song.map"
