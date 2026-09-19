# RoboMaster C 板电控大作业工程

基于 **大疆 RoboMaster 开发板 C 型（STM32F407IGH6）** 的完整大作业工程，覆盖任务一~五，
使用 **FreeRTOS（CMSIS-RTOS V2）** 架构。已在 `ARM GCC 14.3.1 + CMake + Ninja` 下编译验证：
**0 错误、0 警告**，固件约 56 KB。

## 一、任务与代码对应表

| 任务 | 功能 | 实现文件（App 任务） | 硬件 |
|---|---|---|---|
| 任务一 15分 | 上电响 + 两种报错音 | `App/task_buzzer.c` | 蜂鸣器 PD14 / TIM4_CH3 |
| 任务二 5分 | RGB 流水灯（PWM + FreeRTOS 任务） | `App/task_led.c` | PH10/PH11/PH12 / TIM5_CH1/2/3 |
| 任务三 10分 | 舵机 1 秒转 45°（0→180） | `App/task_servo.c` | MG996R PE9 / TIM1_CH1 |
| 任务四 20分 | 串口 + Synex + JustFloat 收发 | `App/task_synex.c` + `Protocol/` | USART1 PA9/PB7 115200 |
| 任务五 20分 | 3508 角度闭环 PID + CAN | `App/task_motor.c` + `Control/` | CAN1 PD0/PD1 1Mbps |

报错音触发：**报错音1**（高音三短声）= 收到无法识别的串口指令；**报错音2**（低音两长声）= 电机反馈失联。
两个音都能在实验室当场演示。

## 二、目录结构（分层架构）

```text
project/
├─ Core/       程序入口、时钟、中断（main.c 在这里）
├─ BSP/        板级驱动：蜂鸣器/ LED/ 舵机/ 串口/ CAN
├─ Protocol/   JustFloat 组帧 + 指令解析（kp=0.1\r）
├─ Control/    PID 控制器 + 电机反馈解析/角度累计/单位换算
├─ App/        FreeRTOS 任务（每个任务一个文件 + 任务注册表）
├─ Drivers/    HAL + CMSIS（ST 官方库，不需要看）
├─ Middlewares/ FreeRTOS 内核（不需要看）
├─ CMakeLists.txt / CMakePresets.json   构建配置
├─ .vscode/    编译/烧录按钮、断点调试配置
└─ docs/       四本调试手册（强烈建议先读 01）
```

依赖方向：`App → Control/Protocol → BSP → HAL`，禁止反向依赖（任务六"软件架构"评分点）。

## 三、快速上手（3 步）

1. **打开**：VS Code → 文件 → 打开文件夹 → 选择 `project` 目录；
2. **编译**：底部状态栏点"**编译**"按钮（或 `Ctrl+Shift+B`）；首次会弹 CMake 预设选择，选 **Debug**；
   成功后 `build/Debug/` 里出现 `project.elf / project.hex / project.bin`；
3. **烧录**：接好 ST-LINK（SWDIO/SWCLK/GND/3.3V 参考）→ 点"**烧录**"按钮。
   详见 `docs/02_编译与烧录.md`。

## 四、常用可调参数（全部是宏/常量，改完重新编译）

| 位置 | 参数 | 默认 | 说明 |
|---|---|---|---|
| `Core/Inc/board.h` | `MOTOR_ID` | 1 | C620 电调 ID（上电看绿灯闪几下） |
| `BSP/bsp_servo.h` | `SERVO_PULSE_0DEG_US / _180DEG_US` | 500/2500 | 舵机脉宽（换舵机改这里） |
| `Control/motor.h` | `MOTOR_CURRENT_LIMIT_A` | 4.0 | 输出电流限幅（**调 PID 前别调大**） |
| `Control/motor.h` | `MOTOR_SEQ_CYCLE_MS` | 2000 | 整个序列 0→90→-90 的完成时限（验收要求 2 秒内） |
| `Control/motor.h` | `MOTOR_SEQ_T0_MS / T1_MS` | 100 / 700 | 0°展示 / 0°→90°用时（剩余 1200ms 给 90°→-90°） |
| `Control/motor.h` | `MOTOR_SETTLE_BAND_DEG / _HOLD_MS` | 5.0 / 50 | 到位判定（误差 ±5° 持续 50ms） |
| `Control/motor.c` | `PID_DEFAULT_KP/KI/KD` | 0.5/0.1/0.02 | 初始 PID（现场用 Synex 在线调） |
| `App/task_servo.c` | `SERVO_SPEED_DPS` | 45 | 舵机角速度 |

## 五、实验室 7 小时作战顺序

1. **0~1h**：烧录 → 蜂鸣器启动音 → LED 流水灯（先验证工具链和 FreeRTOS）
2. **1~1.5h**：舵机 0→180°
3. **1.5~2.5h**：串口对发（按 `docs/03_Synex调试.md`）
4. **2.5~6h**：CAN + PID（大头，按 `docs/04_PID调参.md`）
5. **6~7h**：综合演示 + GitHub 提交

## 六、GitHub 提交建议（任务五/六）

每个功能验证通过就提交一次，message 格式 `<type>: <subject>`（subject ≤50 字符）：

```text
feat: add freertos base project
feat: add buzzer startup and error tones
feat: add rgb led flow task
feat: add servo 45dps sweep task
feat: add synex justfloat uart protocol
feat: add m3508 can feedback and angle pid
```

## 七、手册目录

- `docs/01_接线与上电自检.md` — 所有线怎么接、上电顺序、安全红线（**进实验室前必读**）
- `docs/02_编译与烧录.md` — VS Code 操作、按钮、命令行备选、故障排查
- `docs/03_Synex调试.md` — Synex 配置、通道对应表、发送指令
- `docs/04_PID调参与验收问答.md` — 调参流程 + 验收提问速记（单位换算必背）
