# 🧪 การสังเกตและบันทึกผล (Observation & Result Recording)

## 📊 ตารางบันทึกผลการทดลอง

| ทดลอง | Sender Rate | Receiver Rate | Queue Status | สังเกต |
|-------|--------------|---------------|--------------|---------|
| 1 | 2s | 1.5s | Normal | ส่งและรับข้อมูลสอดคล้องกัน Queue ทำงานปกติ ไม่มี Overflow หรือ Underflow |
| 2 | 0.5s | 1.5s | Full | Sender ส่งข้อมูลเร็วกว่า Receiver ทำให้ Queue เต็มและขึ้นข้อความ “Queue full” |
| 3 | 2s | 0.1s | Empty | Receiver อ่านข้อมูลเร็วกว่า Sender ทำให้ Queue ว่างและขึ้นข้อความ “No message received” |

---

## ❓ คำถามสำหรับการทดลอง

### 1. เมื่อ Queue เต็ม การเรียก `xQueueSend()` จะเกิดอะไรขึ้น?
- เมื่อ Queue เต็ม ระบบจะ **รอ (Block)** จนกว่าจะมีช่องว่างใน Queue หรือจนกว่าเวลาที่กำหนดใน timeout จะหมดลง  
- ถ้าครบเวลาแล้วไม่มีที่ว่าง จะคืนค่า `errQUEUE_FULL` และไม่ส่งข้อมูลเข้า Queue  

**ตัวอย่างผลลัพธ์จาก Serial Monitor:**
W (2000) QUEUE_LAB1: Failed to send message (queue full?)

yaml
คัดลอกโค้ด

---

### 2. เมื่อ Queue ว่าง การเรียก `xQueueReceive()` จะเกิดอะไรขึ้น?
- เมื่อไม่มีข้อมูลใน Queue, Task จะ **รอ (Block)** จนกว่าจะมีข้อมูลเข้ามา  
- ถ้าครบเวลา timeout แล้วไม่พบข้อมูล จะคืนค่า `errQUEUE_EMPTY`

**ตัวอย่างผลลัพธ์จาก Serial Monitor:**
W (5000) QUEUE_LAB1: No message received within timeout

yaml
คัดลอกโค้ด

---

### 3. ทำไม LED จึงกะพริบตามการส่งและรับข้อความ?
- LED แต่ละดวงถูกใช้เพื่อแสดงกิจกรรมของแต่ละ Task:
  - **LED2 (Sender):** กะพริบเมื่อส่งข้อความเข้า Queue สำเร็จ  
  - **LED4 (Receiver):** กะพริบเมื่อรับข้อความออกจาก Queue  
- ช่วยให้มองเห็นการทำงานแบบเรียลไทม์โดยไม่ต้องดู Serial Monitor

---

## 🧭 สรุปผลการทดลอง


### สิ่งที่เรียนรู้:
- [✅] การสร้างและใช้งาน Queue
- [✅] การส่งและรับข้อมูลผ่าน Queue
- [✅] การตรวจสอบสถานะ Queue
- [✅] พฤติกรรมเมื่อ Queue เต็มหรือว่าง
- [✅] การใช้ Timeout ในการส่งและรับ

---

## 🚀 ความท้าทายเพิ่มเติม

1. **Priority Queue**: ปรับปรุงให้ข้อความมี Priority
2. **Multiple Senders**: เพิ่ม Sender หลายตัว
3. **Queue Statistics**: เพิ่มการนับ dropped messages
4. **Dynamic Queue Size**: ทดลองขนาด Queue ต่างๆ

I (1234) QUEUE_ADV: ✅ Sender1 Sent: Sender1 -> MSG#0 (P0)
I (1567) QUEUE_ADV: ✅ Sender2 Sent: Sender2 -> MSG#0 (P0)
I (3011) QUEUE_ADV: 🎯 Received: Sender2 -> MSG#0 (P0) | Time=123
I (6020) QUEUE_ADV: ✅ Sender1 Sent: Sender1 -> MSG#3 (P1)
I (6045) QUEUE_ADV: ⚠️ Sender2 failed to send (Queue full)
I (8050) QUEUE_ADV: 🎯 Received: Sender1 -> MSG#3 (P1) | Time=567
📊 Queue Status → Used: 3 / Free: 2
Sent=15 | Received=13 | Dropped=2
Queue Visualization: [■■■□□]



### 🧩 บทวิเคราะห์เพิ่มเติม
- **Queue เต็ม:** ต้องจัดการด้วย Timeout หรือเพิ่มขนาด Queue  
- **Queue ว่าง:** ควรตั้ง Delay หรือ Timeout เพื่อให้ Task ทำงานอื่นระหว่างรอ  
- **LED สองดวง:** ทำให้เข้าใจจังหวะการสื่อสารของแต่ละ Task ได้ง่ายขึ้น  
- ระบบนี้เป็นพื้นฐานของการสื่อสารใน **ระบบหลาย Task (multitasking)**  
  และเป็นรากฐานของระบบใหญ่เช่น Sensor → Processing → Display

---

### 📘 สรุปสุดท้าย:
การทดลองนี้ทำให้เข้าใจการสื่อสารระหว่าง Task ใน FreeRTOS อย่างลึกซึ้ง  
โดย Queue เป็นเครื่องมือสำคัญในการแลกเปลี่ยนข้อมูลระหว่าง Task  
แบบ **ปลอดภัย (Thread-safe)** และควบคุมได้ด้วย Timeout

**ผลลัพธ์โดยรวม:**  
ระบบทำงานถูกต้องตามหลัก FIFO (First In First Out) และแสดงพฤติกรรม  
เมื่อ Queue เต็ม/ว่างอย่างชัดเจนผ่าน LED และ Log Monitor