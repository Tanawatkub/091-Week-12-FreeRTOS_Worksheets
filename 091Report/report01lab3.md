# 🧠 Week 12: ESP-IDF Basic Project and Environment Setup  
**รายงานโดย:** Tanawat Putta (67030091)  
**รายวิชา:** Microcontroller Applications  

## ✅ Checklist การทำงาน

- [✅] สร้าง Task พื้นฐานสำเร็จ  
- [✅] เข้าใจ Task parameters และ return values  
- [✅] ทดสอบ Task priorities  
- [✅] ใช้ Task management APIs (suspend/resume)  
- [✅] แสดง runtime statistics  
- [✅] ทำแบบฝึกหัดครบ  

---

## 🧩 คำถามทบทวน

### 1️⃣ เหตุใด Task function ต้องมี **infinite loop** ?
เพราะในระบบ FreeRTOS แต่ละ Task จะต้อง **ทำงานต่อเนื่องตลอดเวลา**  
เช่นใน `led1_task()`, `led2_task()`, หรือ `system_info_task()`  
ถ้าไม่มี loop ฟังก์ชันจะจบและ Task จะถูกลบออกจากระบบทันที  

**สรุป:**  
Task ต้องอยู่ในลูปเพื่อให้ทำงานซ้ำได้อย่างต่อเนื่อง (เช่นกระพริบไฟ LED, อ่านค่าเซนเซอร์)

---

### 2️⃣ ความหมายของ **stack size** ใน `xTaskCreate()`
ค่าที่กำหนด stack size คือ **ขนาดของหน่วยความจำ (RAM)** ที่จัดสรรให้กับ Task นั้น ๆ  
ใช้เก็บตัวแปรภายในฟังก์ชัน, ค่าที่ return, และ context ของ Task  

**สรุป:**  
- Stack size มากเกินไป → เปลืองหน่วยความจำ  
- Stack size น้อยเกินไป → อาจเกิด stack overflow  

---

### 3️⃣ ความแตกต่างระหว่าง `vTaskDelay()` และ `vTaskDelayUntil()`
| ฟังก์ชัน | ลักษณะการทำงาน |
|-----------|----------------|
| `vTaskDelay()` | หน่วงเวลาโดยนับจากเวลาปัจจุบันของระบบ |
| `vTaskDelayUntil()` | หน่วงเวลาแบบอ้างอิงเวลาเริ่มต้นเดิม → ใช้สำหรับให้ Task ทำงาน “คาบเท่ากัน” |

**สรุป:**  
- `vTaskDelay()` → เหมาะกับการหน่วงทั่วไป  
- `vTaskDelayUntil()` → เหมาะกับ Task แบบ periodic (เช่น sampling sensor ทุก 100 ms)

---

### 4️⃣ การใช้ `vTaskDelete(NULL)` vs `vTaskDelete(handle)`
| ฟังก์ชัน | หน้าที่ |
|-----------|----------|
| `vTaskDelete(NULL)` | ลบ Task ตัวเองออกจากระบบ |
| `vTaskDelete(handle)` | ลบ Task อื่นโดยใช้ handle ที่ได้จาก `xTaskCreate()` |

**สรุป:**  
`vTaskDelete(NULL)` → Task ลบตัวเอง  
`vTaskDelete(handle)` → ใช้ลบ Task อื่นที่สร้างไว้ก่อนหน้า

---

### 5️⃣ Priority 0 กับ Priority 24 อันไหนสูงกว่า?
Priority **24 สูงกว่า 0**  
เพราะใน FreeRTOS ค่ายิ่งมาก → ยิ่งมีสิทธิ์ได้ CPU ก่อน  

**สรุป:**  
- Task ที่มี priority สูงจะได้รันก่อน  
- ถ้า priority เท่ากัน → ระบบใช้ **Round-Robin Scheduling**

---