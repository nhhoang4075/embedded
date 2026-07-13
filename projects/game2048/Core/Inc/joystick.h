/*
 * joystick.h — Đọc joystick analog 2 trục qua ADC, trả về 1 hướng gạt.
 *
 * Nguyên tắc chống "trôi": edge-trigger + deadzone.
 *   - Cần phải về vùng giữa (deadzone) rồi đẩy mới tính 1 nước đi.
 *   - Giữ cần ở 1 phía sẽ KHÔNG sinh ra nhiều nước liên tiếp.
 *
 * Cấu hình chân/kênh nằm ở board_config.h — file này không hardcode
 * ADC channel hay chân SW nữa. Tunable (threshold, deadzone, hướng
 * lắp) giữ ở đây để dễ chỉnh tại nơi dùng.
 */
#ifndef GAME2048_JOYSTICK_H
#define GAME2048_JOYSTICK_H

#include <stdbool.h>
#include <stdint.h>

/* Forward-declare handle ADC để không kéo cả stm32f4xx_hal.h vào mọi
 * TU dùng joystick. File .c include HAL đầy đủ. */
struct __ADC_HandleTypeDef;
typedef struct __ADC_HandleTypeDef ADC_HandleTypeDef;

#ifdef __cplusplus
extern "C" {
#endif

/* Ngưỡng trên thang 12-bit (0..4095). Tâm mặc định 2048; auto-calibrate
 * sẽ được khôi phục ở pha refactor sau (đọc N mẫu lúc joystick_init). */
#define JOY_THRESHOLD  1200u  /* lệch quá mức này mới tính là "đẩy"    */
#define JOY_DEADZONE   400u   /* trong khoảng tâm±400 coi như "thả"    */

/* --- Hiệu chỉnh chiều theo cách lắp joystick (0 = giữ nguyên, 1 = bật) ---
 * Cách dò: cầm joystick cố định 1 chiều, build & test, đổi các số 0/1 dưới đây
 * cho tới khi cả 4 hướng đúng.
 *  - JOY_INVERT_Y: trục dọc bị ngược (đẩy lên thì đi xuống) -> để 1. Mặc định 1
 *    vì màn hình có trục Y hướng xuống, ngược với joystick "đẩy lên tăng VRy".
 *  - JOY_INVERT_X: trục ngang bị ngược (đẩy phải thì đi trái) -> để 1.
 *  - JOY_SWAP_XY : gạt ngang lại đi dọc và ngược lại (joystick lắp xoay 90°) -> để 1.
 */
#ifndef JOY_INVERT_X
#define JOY_INVERT_X  1
#endif
#ifndef JOY_INVERT_Y
#define JOY_INVERT_Y  1
#endif
#ifndef JOY_SWAP_XY
#define JOY_SWAP_XY   0
#endif

/* Hướng gạt do joystick sinh ra. Giá trị 0..3 được chọn KHỚP với
 * g2048_dir_t (G2048_LEFT=0, RIGHT=1, UP=2, DOWN=3) để Model có thể
 * ép kiểu trực tiếp mà không cần translate — nhưng joystick.h vẫn
 * không include game2048.h để không leak domain enum vào driver. */
typedef enum {
    JOY_NONE      = -1,
    JOY_DIR_LEFT  = 0,
    JOY_DIR_RIGHT = 1,
    JOY_DIR_UP    = 2,
    JOY_DIR_DOWN  = 3,
} joy_dir_t;

/* Lưu handle ADC do CubeMX sinh ra (vd. &hadc1). Gọi 1 lần lúc đầu.
 * Cũng cấu hình chân SW làm input pull-up (theo board_config.h). */
void joystick_init(ADC_HandleTypeDef *hadc);

/*
 * Quét joystick. Trả về JOY_DIR_NONE nếu không có sự kiện, hoặc 1 hướng
 * khi vừa đẩy qua ngưỡng (mỗi lần đẩy chỉ phát 1 lần).
 */
joy_dir_t joystick_poll(void);

/*
 * Kiểm tra nút SW. Trả về true ĐÚNG 1 LẦN khi vừa được ấn xuống
 * (edge-trigger). Phải thả ra rồi ấn lại mới tính event tiếp theo.
 * Có debounce ~20ms để chống bouncing tiếp xúc cơ học.
 */
bool joystick_sw_pressed(void);

/*
 * Đọc giá trị ADC thô của hai trục — dùng làm nguồn entropy khi seed
 * RNG lúc user tương tác lần đầu. Không đụng edge-trigger state.
 */
uint16_t joystick_raw_x(void);
uint16_t joystick_raw_y(void);

#ifdef __cplusplus
}
#endif

#endif /* GAME2048_JOYSTICK_H */
