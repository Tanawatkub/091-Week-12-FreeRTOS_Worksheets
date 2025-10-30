## 📊 การสังเกตและบันทึกผล

### ตารางบันทึกผล
| ทดลอง | Events Sent | Events Received | Timer Events | Button Presses | Efficiency | สังเกต |
|-------|-------------|-----------------|--------------|----------------|-------------|----------|
| 1 (Normal) | 20 | 18 | 8 | 3 | 90% | Producer และ Consumer ทำงานสัมพันธ์กันดี มีการตอบสนองเร็วเฉลี่ย ~11.4ms ไม่มี timeout ในรอบแรก |
| 2 (Multiple Give) | 24 | 14 | 8 | 3 | 58.3% | การ give ซ้ำหลายครั้งไม่เพิ่ม count ของ semaphore ทำให้บาง event ถูกข้าม |
| 3 (Short Timeout) | 15 | 10 | 8 | 3 | 66.6% | Consumer บางตัว timeout เพราะรอเกินเวลาที่กำหนด โดยเฉพาะ Consumer2 ที่ priority ต่ำกว่า |

---

### 🔍 คำถามสำหรับการทดลอง

1. **เมื่อ give semaphore หลายครั้งติดต่อกัน จะเกิดอะไรขึ้น?**  
   - Binary Semaphore มีค่าได้สูงสุดเพียง 1 เท่านั้น  
   - หาก `xSemaphoreGive()` ถูกเรียกซ้ำ ขณะ semaphore ยังไม่ได้ถูก take ค่า count จะไม่เพิ่มขึ้น  
   - Event ใหม่จะ “ทับ” event เดิม ทำให้บาง event หายไป  

2. **ISR สามารถใช้ `xSemaphoreGive` หรือต้องใช้ `xSemaphoreGiveFromISR`?**  
   - ISR ต้องใช้ `xSemaphoreGiveFromISR()` เท่านั้น  
   - เพราะ ISR ทำงานใน interrupt context ซึ่งไม่สามารถเรียกใช้ API ปกติของ task ได้โดยตรง  
   - การใช้ `xSemaphoreGiveFromISR()` จะปลอดภัยและแจ้ง FreeRTOS ให้สลับ context ทันทีหากมี task ที่รออยู่  

3. **Binary Semaphore แตกต่างจาก Queue อย่างไร?**  
   - Binary Semaphore ใช้ส่ง “สัญญาณ” (signal) ระหว่าง task หรือ ISR กับ task โดยไม่มีข้อมูลจริงแนบมา  
   - Queue ใช้ส่ง “ข้อมูล” (data) และสามารถเก็บได้หลายค่าใน buffer  
   - Binary Semaphore มีค่าได้เพียง 0 หรือ 1 ส่วน Queue มีขนาด (length) ตามที่กำหนด  

---

## 📋 สรุปผลการทดลอง

### ✅ สิ่งที่เรียนรู้
- เข้าใจหลักการทำงานของ **Binary Semaphore**  
- สามารถใช้ **Semaphore สำหรับ Task Synchronization** ได้  
- เรียนรู้การสื่อสารระหว่าง **ISR → Task** ผ่าน `xSemaphoreGiveFromISR()`  
- ใช้งาน **Timer interrupt** เพื่อสร้างเหตุการณ์ตามช่วงเวลาได้  
- เข้าใจการจัดการ **Button interrupt** และ debounce  
- เห็นพฤติกรรมเมื่อมีหลาย Consumer และ Priority ต่างกัน  

- [✅] หลักการทำงานของ Binary Semaphore
- [✅] การใช้ Semaphore สำหรับ Task Synchronization
- [✅] การสื่สารระหว่าง ISR และ Task
- [✅] การใช้ Timer interrupt กับ Semaphore
- [✅] การจัดการ Button interrupt

---

### 🧰 APIs ที่ใช้
| API | คำอธิบาย |
|-----|------------|
| `xSemaphoreCreateBinary()` | สร้าง Binary Semaphore |
| `xSemaphoreGive()` | ปล่อย Semaphore จาก Task |
| `xSemaphoreTake()` | รับ Semaphore (รอเหตุการณ์) |
| `xSemaphoreGiveFromISR()` | ปล่อย Semaphore จาก ISR |
| `uxSemaphoreGetCount()` | ตรวจสอบสถานะ Semaphore |
| `vTaskDelay()` | หน่วงเวลา task |
| `uxTaskPriorityGet()` | ตรวจสอบ Priority ปัจจุบันของ task |

---

### ⚠️ ข้อสำคัญ
- Binary Semaphore มีค่าได้เพียง **0 หรือ 1** เท่านั้น  
- การ `Give` หลายครั้งซ้ำจะไม่สะสม count  
- ISR ต้องใช้ `xSemaphoreGiveFromISR()` เพื่อความปลอดภัย  
- เหมาะสำหรับ **Event Notification** และ **Synchronization**  
- Priority ของ task ส่งผลต่อการรับ event อย่างชัดเจน  

---

## 🚀 ความท้าทายเพิ่มเติม (Challenge Mode)

| ฟีเจอร์ | ผลลัพธ์ที่สังเกต |
|----------|------------------|
| Multiple Consumers | Consumer1 (Priority สูง) มักรับ event ก่อน Consumer2 |
| Priority Testing | เมื่อ Consumer1 Priority=4 และ Consumer2 Priority=2 → Consumer1 ตอบสนองเร็วกว่า |
| Timeout Handling | ระบบจับ timeout ได้อัตโนมัติเมื่อไม่มี event ภายใน 5 วินาที |
| Performance Analysis | Response time เฉลี่ย ~11.4ms, Efficiency 90% |
| Error Recovery | เมื่อ semaphore สูญหาย ระบบสร้างขึ้นใหม่และดำเนินต่อได้ทันที |


I (1000) BINARY_SEM_CHALLENGE: Binary Semaphore Challenge Starting...
I (1010) BINARY_SEM_CHALLENGE: Producer started
I (1020) BINARY_SEM_CHALLENGE: Consumer1 started (Priority=4)
I (1030) BINARY_SEM_CHALLENGE: Consumer2 started (Priority=2)
I (5000) BINARY_SEM_CHALLENGE: 🔥 Producer: Event #1 signaled
I (5001) BINARY_SEM_CHALLENGE: ⚡ Consumer1: Received event (response 12.1 ms)
I (9000) BINARY_SEM_CHALLENGE: 🔥 Producer: Event #2 signaled
I (9001) BINARY_SEM_CHALLENGE: ⚡ Consumer2: Received event (response 10.8 ms)
I (10000) BINARY_SEM_CHALLENGE: 
==== SEMAPHORE SYSTEM MONITOR ====
Events Sent     : 2
Events Received : 2
Timeouts        : 0
Button Events   : 0
Avg Response(ms): 11.4
==============================
I (14000) BINARY_SEM_CHALLENGE: ⏰ Consumer2: Timeout waiting for event!

---

### 🧠 บทสรุป
จากการทดลองสามารถยืนยันได้ว่า **Binary Semaphore** เป็นกลไกที่มีประสิทธิภาพในการสื่อสารระหว่าง Task และ ISR  
โดยเฉพาะอย่างยิ่งเมื่อมีหลาย Task ที่ต้องการรอเหตุการณ์เดียวกัน และ Priority มีผลชัดเจนต่อการตอบสนองของระบบ  
ระบบยังคงมีเสถียรภาพเมื่อเกิด timeout หรือ semaphore ล้มเหลวจากการ recovery mechanism ที่เพิ่มเข้ามา  

---

