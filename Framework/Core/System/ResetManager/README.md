# STM32 Reset Tracking Library

## Giới Thiệu

Reset Tracking Library là thư viện MCU-agnostic giúp theo dõi và phân tích nguyên nhân reset của hệ thống STM32. Thư viện cung cấp khả năng:

- **Phân loại nguyên nhân reset**: 17 loại reset khác nhau (watchdog, fault, software, hardware, etc.)
- **Persistence**: Lưu trữ qua RTC backup registers, duy trì thông tin sau reset
- **Cross-family**: Hỗ trợ STM32L1xx, STM32L4xx và các families khác
- **Debugging**: Giúp chẩn đoán lỗi và phân tích stability của hệ thống

## Ý Nghĩa & Vai Trò

### 🎯 **Tại Sao Cần Reset Tracking?**

1. **Debug & Troubleshooting**: Xác định nguyên nhân thiết bị reset bất ngờ
2. **System Stability**: Theo dõi tần suất các loại reset để cải thiện firmware
3. **Field Diagnostics**: Phân tích lỗi từ xa trên các thiết bị deployed
4. **Compliance**: Đáp ứng yêu cầu logging cho các ứng dụng critical

### 📊 **Các Loại Reset Được Hỗ Trợ**

| Reset Type | Nguồn Gốc | Ý Nghĩa |
|------------|-----------|---------|
| `POWER_ON_RESET` | Hardware | Cấp nguồn lần đầu hoặc brown-out |
| `EXTERNAL_RESET` | Hardware | Nhấn nút reset hoặc tín hiệu NRST |
| `HARDFAULT_RESET` | Firmware | Lỗi nghiêm trọng trong code |
| `IWDG_RESET` | Hardware | Independent watchdog timeout |
| `SOFTWARE_RESET` | Firmware | Gọi `NVIC_SystemReset()` |
| `LOW_POWER_RESET` | Hardware | Reset từ low-power mode |

*(Và 11 loại khác...)*

## Cấu Trúc Thư Viện

```
Framework/
├── Core/SystemManagement/
│   ├── system_reset_tracking.h     # Public API
│   └── system_reset_tracking.c     # Core implementation
└── Common/
    └── system_reset_platform_stm32.c  # STM32 platform hooks
```

## Hướng Dẫn Sử Dụng

### 1. **Setup & Integration**

#### Auto Integration (Khuyên Dùng)
```bash
# Apply cho tất cả projects
.\scripts\apply_reset_tracking.ps1
```

#### Manual Integration
```c
// Trong file interrupt handler (stm32xxx_it.c)
#include <Core/SystemManagement/system_reset_tracking.h>

void HardFault_Handler(void) {
    system_reset_tracking_set_reason(HARDFAULT_RESET);
    __NVIC_SystemReset();
}
```

### 2. **Sử Dụng Cơ Bản**

```c
#include <Core/SystemManagement/system_reset_tracking.h>

void app_main(void) {
    // Khởi tạo và đọc reset reason
    system_reset_tracking_init();
    e_reset_reason reason = system_reset_tracking_get_reason();
    const char* reason_str = system_reset_tracking_reason_to_string(reason);

    // Log hoặc xử lý
    printf("Last reset: %s\n", reason_str);

    // Clear để chuẩn bị cho lần reset tiếp theo
    system_reset_tracking_clear();
}
```

### 3. **Tracking Software Reset**

```c
// Thay vì gọi trực tiếp NVIC_SystemReset()
void safe_system_reset(void) {
    system_reset_tracking_set_reason(SOFTWARE_RESET);
    __NVIC_SystemReset();
}

void firmware_update_complete(void) {
    system_reset_tracking_set_reason(FWUPDATE_RESET);
    __NVIC_SystemReset();
}
```

### 4. **Persistence & RTC**

```c
// External RTC handle (cần khai báo trong rtc.c)
extern RTC_HandleTypeDef hrtc;

// Library tự động sử dụng RTC backup registers:
// - DR0: Magic number validation
// - DR1: Reset reason storage
```

## API Reference

### Core Functions

```c
// Khởi tạo (gọi sớm trong boot sequence)
void system_reset_tracking_init(void);

// Đọc reset reason hiện tại
e_reset_reason system_reset_tracking_get_reason(void);

// Set reset reason trước khi reset
void system_reset_tracking_set_reason(e_reset_reason reason);

// Convert enum to string
const char* system_reset_tracking_reason_to_string(e_reset_reason reason);

// Clear tracking data
void system_reset_tracking_clear(void);
```

### Platform Hooks (Weak Functions)

```c
// Implement trong platform-specific code nếu cần custom
bool platform_reset_persist_write(e_reset_reason reason);
bool platform_reset_persist_read(e_reset_reason *reason);
void platform_reset_persist_clear(void);
bool platform_reset_detect_hardware(e_reset_reason *reason);
```

## Ví Dụ Thực Tế

### Scenario 1: Debug HardFault
```c
// Boot sequence
system_reset_tracking_init();
if (system_reset_tracking_get_reason() == HARDFAULT_RESET) {
    // Gửi alert, enable debug mode, etc.
    send_fault_report_to_server();
}
```

### Scenario 2: Watchdog Monitoring
```c
e_reset_reason reason = system_reset_tracking_get_reason();
if (reason == IWDG_RESET || reason == WWDG_RESET) {
    // Tăng counter, phân tích performance
    increment_watchdog_reset_counter();
    analyze_system_performance();
}
```

### Scenario 3: Field Diagnostics
```c
// Logging cho remote debugging
char log_msg[100];
sprintf(log_msg, "Device reset: %s at %lu",
        system_reset_tracking_reason_to_string(reason),
        HAL_GetTick());
send_log_to_cloud(log_msg);
```

## Compatibility

- **STM32L1xx**: STM32L152, STM32L151, etc.
- **STM32L4xx**: STM32L452, STM32L431, etc.
- **Other STM32**: Dễ dàng mở rộng với HAL support
- **RTOS**: FreeRTOS compatible
- **Toolchain**: STM32CubeIDE, Keil, GCC

## Best Practices

1. **Gọi `init()` sớm**: Trước khi HAL clear reset flags
2. **Always clear**: Gọi `clear()` sau khi xử lý xong
3. **Fault handlers**: Luôn set reason trước khi reset
4. **RTC dependency**: Đảm bảo RTC được config nếu cần persistence
5. **Thread safety**: Library không thread-safe, cần mutex nếu dùng multi-thread

## Troubleshooting

**Q: Luôn nhận `NORMAL_BOOT`?**
A: Gọi `init()` quá muộn hoặc RTC không được config.

**Q: RTC persistence không hoạt động?**
A: Kiểm tra `hrtc` extern declaration và RTC backup register config.

**Q: Compile errors về RCC flags?**
A: Normal với IntelliSense, sẽ OK khi build với target MCU.

---

**Author**: This PC
**Date**: August 2025
**Version**: 1.0
**License**: Internal Use
