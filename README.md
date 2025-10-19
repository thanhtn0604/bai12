# STM32 FreeRTOS Blink LED using Queue

### Đề bài
Viết chương trình sử dụng FreeRTOS có tối thiểu 2 tác vụ:
- Task 1: Thực hiện Blink LED theo tần số, độ rộng xung nhận được qua Queue.
- Task 2: Định kỳ tạo ra tần số và độ rộng xung thay đổi và gửi sang Task 1 qua Queue.
2 Task hoạt động song song, kết quả LED nhấp nháy với tần số, độ rộng xung khác nhau theo thời gian

### ⚙️ Chức năng
- **Task UART**: 
  - Đọc chuỗi từ UART theo định dạng `"freq,duty"` (ví dụ: `5,50` nghĩa là 5 Hz - 50% duty cycle).
  - Gửi dữ liệu này vào hàng đợi (`Queue`) để task khác xử lý.

- **Task LED**:
  - Nhận tín hiệu `frequency` và `duty_cycle` từ hàng đợi.
  - Nhấp nháy LED tương ứng trong 5 giây theo tần số và chu kỳ đặt trước.

### Cấu hình phần cứng
- LED nối với **PA0**
- UART1: 
  - TX: PA9  
  - RX: PA10  
  - Baudrate: 9600 bps

### Thành phần sử dụng
- **STM32 Standard Peripheral Library**
- **FreeRTOS**
- **USART1** cho giao tiếp UART
- **GPIOA** cho LED output

