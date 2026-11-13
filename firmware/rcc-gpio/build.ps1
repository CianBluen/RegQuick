param(
    [Parameter(Mandatory)]
    [string]$CompilerBin,

    [switch]$SkipGpioClock,

    [switch]$ClockExti,

    [switch]$TimUsart,

    [string]$LearningSource
)

$ErrorActionPreference = 'Stop'
if ($LearningSource) {
    if ($ClockExti -or $TimUsart -or $SkipGpioClock) {
        throw 'Build the learning source by itself.'
    }
    $learningFile = Get-Item -LiteralPath $LearningSource
    if ($learningFile.PSIsContainer -or $learningFile.Extension -ne '.c') {
        throw 'LearningSource must be one C file with main().'
    }
}
if ($ClockExti -and $SkipGpioClock) {
    throw 'Use the WrongPort target for the clock and EXTI fault exercise.'
}
if ($TimUsart -and ($ClockExti -or $SkipGpioClock)) {
    throw 'Select the timer/USART lab by itself.'
}
$compiler = (Resolve-Path -LiteralPath $CompilerBin).Path

function Invoke-CompilerTool {
    param([string]$Tool, [string[]]$Arguments)

    & (Join-Path $compiler "$Tool.exe") @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Tool failed with exit code $LASTEXITCODE"
    }
}

Push-Location -LiteralPath $PSScriptRoot
try {
    $targets = if ($LearningSource) {
        @($learningFile.BaseName)
    } elseif ($TimUsart) {
        @('PwmRegister', 'PwmSPL', 'PwmWrongEdge', 'Reaction', 'ReactionWrongTick', 'Tim1', 'Tim1NoMoe')
    } elseif ($ClockExti) {
        @('HSI', 'HSE', 'WrongPort', 'NoPreempt')
    } else {
        @('Register', 'SPL')
    }
    foreach ($target in $targets) {
        $output = "build/cli-$target"
        if ($LearningSource) { $output = "../learning/build/$target" }
        if ($ClockExti) { $output = "../clock-exti/build/$target" }
        if ($TimUsart) { $output = "../tim-usart/build/$target" }
        if ($SkipGpioClock) { $output += '-fault' }
        New-Item -ItemType Directory -Force -Path $output | Out-Null

        $startup = "$output/startup.o"
        Invoke-CompilerTool armasm @('--cpu', 'Cortex-M3', '--debug', '-o', $startup, 'vendor/startup_stm32f10x_md.s')

        $defines = @('-DSTM32F10X_MD')
        if ($SkipGpioClock) { $defines += '-DLAB_SKIP_GPIO_CLOCK=1' }
        $sources = @('app/system_hsi.c')
        if ($LearningSource) {
            $defines += @('-DUSE_STDPERIPH_DRIVER', '-DLAB_USE_SPL')
            $sources += @($learningFile.FullName, 'vendor/stm32f10x_gpio.c', 'vendor/stm32f10x_rcc.c',
                          'vendor/stm32f10x_exti.c', 'vendor/stm32f10x_tim.c', 'vendor/stm32f10x_usart.c')
        } elseif ($TimUsart) {
            $sources += '../tim-usart/console.c'
            switch -Wildcard ($target) {
                'Pwm*' { $sources += '../tim-usart/pwm.c' }
                'Reaction*' { $sources += '../tim-usart/reaction.c' }
                'Tim1*' { $sources += '../tim-usart/advanced.c' }
            }
            switch ($target) {
                'PwmSPL' {
                    $defines += @('-DLAB_USE_SPL', '-DUSE_STDPERIPH_DRIVER')
                    $sources += @('vendor/stm32f10x_gpio.c', 'vendor/stm32f10x_rcc.c',
                                  'vendor/stm32f10x_tim.c', 'vendor/stm32f10x_usart.c')
                }
                'PwmWrongEdge' { $defines += '-DLAB_WRONG_EDGE' }
                'ReactionWrongTick' { $defines += '-DLAB_WRONG_TICK' }
                'Tim1NoMoe' { $defines += '-DLAB_NO_MOE' }
            }
        } elseif ($ClockExti) {
            $sources += @('../clock-exti/main.c', '../clock-exti/clock.c')
            switch ($target) {
                'HSE' { $defines += '-DLAB_USE_HSE' }
                'WrongPort' { $defines += '-DLAB_WRONG_PORT' }
                'NoPreempt' { $defines += '-DLAB_NO_PREEMPT' }
            }
        } elseif ($target -eq 'Register') {
            $sources += 'app/register.c'
        } else {
            $defines += '-DUSE_STDPERIPH_DRIVER'
            $sources += @('app/spl.c', 'vendor/stm32f10x_gpio.c', 'vendor/stm32f10x_rcc.c')
        }

        $objects = @($startup)
        foreach ($source in $sources) {
            $object = "$output/$([IO.Path]::GetFileNameWithoutExtension($source)).o"
            $options = @('--cpu', 'Cortex-M3', '--c99', '-g', '-O1', '-Ivendor', '-Iapp')
            Invoke-CompilerTool armcc ($options + $defines + @('-c', $source, '-o', $object))
            $objects += $object
        }

        $imageName = if ($ClockExti) { 'clock-exti' } else { 'button-led' }
        if ($TimUsart) { $imageName = 'tim-usart' }
        if ($LearningSource) { $imageName = 'learning' }
        $image = "$output/$imageName.axf"
        $layout = @('--cpu', 'Cortex-M3', '--ro_base', '0x08000000', '--rw_base', '0x20000000', '--first', 'startup.o(RESET)')
        Invoke-CompilerTool armlink ($layout + $objects + @('-o', $image))
        Invoke-CompilerTool fromelf @('--i32combined', '--output', "$output/$imageName.hex", $image)
        Write-Output "Built $output/$imageName.hex"
    }
} finally {
    Pop-Location
}
