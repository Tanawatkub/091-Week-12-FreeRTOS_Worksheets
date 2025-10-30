# 🧠 Week 12: ESP-IDF Basic Project and Environment Setup  
**รายงานโดย:** Tanawat Putta (67030091)  
**รายวิชา:** Microcontroller Applications  

## ✅ Checklist การทำงาน

- [✅] Flash และ Monitor สำเร็จ  
- [✅] เห็น Log messages ต่าง ๆ  
- [✅] ทดสอบ Log levels ต่าง ๆ  
- [✅] ใช้ formatted logging ได้  
- [✅] ปรับ Log level ผ่าน menuconfig  
- [✅] ทำแบบฝึกหัดครบ  

---

## 🧩 คำถามทบทวน

### 1️⃣ ความแตกต่างระหว่าง `printf()` และ `ESP_LOGI()`
| ฟังก์ชัน | คุณสมบัติ |
|-----------|------------|
| `printf()` | แสดงข้อความธรรมดาออกหน้าจอ โดยไม่มีระบบจัดการระดับ log |
| `ESP_LOGI()` | เป็นระบบ logging ของ ESP-IDF ที่สามารถกำหนดระดับข้อความได้ เช่น Info, Warning, Error, Debug |

**สรุป:**  
`ESP_LOGI()` เหมาะสำหรับการ debug และตรวจสอบสถานะระบบ เพราะสามารถกรอง log ตามระดับได้  
ขณะที่ `printf()` ใช้เพียงพิมพ์ข้อความธรรมดาโดยไม่ผ่านระบบ logging

---

### 2️⃣ Log level ที่แสดงในค่า Default configuration
ค่าเริ่มต้นคือ **Info**  
ดังนั้น log ที่ระดับ **Info**, **Warning**, และ **Error** จะถูกแสดง  
ส่วนระดับ **Debug** และ **Verbose** จะไม่แสดงจนกว่าจะไปปรับใน `menuconfig`

**สรุป:**  
ระดับ Log เริ่มต้น = *Info*  
→ เห็นเฉพาะข้อความที่สำคัญ ไม่รกหน้าจอ

---

### 3️⃣ การใช้ `ESP_ERROR_CHECK()` มีประโยชน์อย่างไร?
ฟังก์ชันนี้ใช้ตรวจสอบผลลัพธ์จากฟังก์ชันอื่นของ ESP-IDF เช่น
```c
ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0));
ถ้าคืนค่าเป็น Error → ระบบจะ หยุดการทำงานทันที และพิมพ์ข้อความ error ลง Serial Monitor

สรุป:
ESP_ERROR_CHECK() ช่วยให้ตรวจจับข้อผิดพลาดได้ง่าย
และป้องกันระบบ crash โดยไม่รู้สาเหตุ

4️⃣ คำสั่งออกจาก Monitor mode
ระบบปฏิบัติการ	ปุ่มที่ใช้
Linux / macOS	Ctrl + ]
Windows	Ctrl + T แล้วตามด้วย Ctrl + X

💡 เคล็ดลับ: กด Ctrl + T แล้ว Ctrl + H เพื่อดู Help ของ monitor mode ได้

5️⃣ การตั้งค่า Log level สำหรับ tag เฉพาะ
ใช้คำสั่ง:

c
คัดลอกโค้ด
esp_log_level_set("LOGGING_DEMO", ESP_LOG_DEBUG);
อธิบาย:

"LOGGING_DEMO" คือ tag ของโมดูลนั้น ๆ

ESP_LOG_DEBUG คือระดับ log ที่ต้องการ
ใช้เพื่อปรับ log เฉพาะส่วน ไม่กระทบกับ tag อื่นในระบบ

🧾 บทสรุป (Summary)
ในแลปนี้ได้เรียนรู้เกี่ยวกับ

การใช้งานระบบ Logging ของ ESP-IDF

ความแตกต่างระหว่าง printf() และ ESP_LOGx()

การกำหนดระดับของ log และการเปลี่ยนผ่าน menuconfig

การใช้ ESP_ERROR_CHECK() เพื่อตรวจจับข้อผิดพลาด

วิธีออกจาก monitor และจัดการ tag เฉพาะ

🧠 Reflection
หลังจากทำแลปนี้ เข้าใจว่าระบบ Logging ของ ESP-IDF
มีประโยชน์มากในการ debug โดยไม่ต้องใช้ printf เยอะ ๆ
สามารถควบคุมระดับของข้อความได้ ทำให้ดูเฉพาะสิ่งสำคัญ
และช่วยให้แก้ปัญหาในระบบ FreeRTOS ได้ง่ายและเป็นระบบมากขึ้น