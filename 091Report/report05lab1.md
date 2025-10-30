# 🧪 Lab 1: Basic Software Timers

## 🎯 วัตถุประสงค์
- เข้าใจหลักการทำงานของ FreeRTOS Software Timers
- เรียนรู้การสร้างและจัดการ One-shot และ Auto-reload Timers
- ฝึกการใช้ Timer Callbacks และ Timer Command Queue
- เข้าใจความแตกต่างระหว่าง Hardware Timers และ Software Timers

---

## 🧩 เนื้อหาทดลอง
ในโค้ดได้ใช้ Software Timers เพื่อควบคุม LED หลายดวงและจำลอง heartbeat/status ของระบบ โดยมี:
- Timer แบบ Auto-reload ทำงานซ้ำ (Blink, Heartbeat, Status)
- Timer แบบ One-shot ทำงานครั้งเดียว (Oneshot, Dynamic)
- การปรับ period แบบสุ่ม
- การสร้าง/ลบ timer แบบ dynamic
![alt text](image.png)
---

## ⚙️ การตั้งค่าใน menuconfig
Component config → FreeRTOS → Kernel
[*] Enable FreeRTOS timers
(3) Timer task priority
(2048) Timer task stack depth
(10) Timer queue length

yaml
คัดลอกโค้ด

---

## 📊 สรุปผลการทดลอง

### 🔹 สิ่งที่เรียนรู้
- ✅ การสร้างและจัดการ **Software Timers**
- ✅ ความแตกต่างระหว่าง **One-shot** และ **Auto-reload timers**
- ✅ การทำงานของ **Timer callbacks และ context**
- ✅ การสร้างและลบ **Dynamic timer** ขณะรัน
- ✅ การทำงานของ **Timer Service Task**

---

### 🔹 APIs ที่ใช้
| ฟังก์ชัน | หน้าที่ |
|-----------|----------|
| `xTimerCreate()` | สร้าง timer |
| `xTimerStart()` | เริ่ม timer |
| `xTimerStop()` | หยุด timer |
| `xTimerChangePeriod()` | เปลี่ยนระยะเวลา |
| `xTimerDelete()` | ลบ timer |
| `xTimerReset()` | รีเซ็ต timer |

---

## 🚀 ความท้าทายเพิ่มเติม (Challenge Extensions)

### 1. **Timer Synchronization**
ซิงค์การทำงานของหลาย Timer ให้ทำงานพร้อมกันโดยใช้ Event Group หรือ Queue

### 2. **Performance Analysis**
วัดความแม่นยำของ Timer โดยเทียบเวลาจริงกับเวลาที่ตั้งไว้ และตรวจสอบ jitter

### 3. **Error Handling**
จำลองสถานการณ์ที่ Timer Command Queue เต็ม และจัดการ retry

### 4. **Complex Scheduling**
สร้าง Pattern การทำงานของ Timer แบบซ้อน เช่น Timer A เรียก Timer B เมื่อครบรอบ

### 5. **Resource Management**
วิเคราะห์จำนวน Timer ที่สามารถสร้างพร้อมกัน และการคืนหน่วยความจำเมื่อ delete


I (0) TIMER_CHALLENGE: 🚀 Advanced FreeRTOS Timer Challenge Starting...
I (0) TIMER_CHALLENGE: ✅ All timers started successfully!
I (500) TIMER_CHALLENGE: 🔄 Sync Timer #1 | Interval: 500 ticks | Avg: 500.00
I (1000) TIMER_CHALLENGE: 🔄 Sync Timer #2 | Interval: 500 ticks | Avg: 500.00
I (3000) TIMER_CHALLENGE: 📊 Performance Analysis...
I (3000) TIMER_CHALLENGE: ✅ Timer stable (avg interval 500.00 ticks)
I (7000) TIMER_CHALLENGE: 🧭 Scheduler Pattern #1
I (7000) TIMER_CHALLENGE: 🟡 Slowing Down Sync
I (9000) TIMER_CHALLENGE: ⚠️ Simulating processing delay...
I (9500) TIMER_CHALLENGE: 🔄 Sync Timer #12 | Interval: 700 ticks | Avg: 520.25
I (10000) TIMER_CHALLENGE: 📊 Performance Analysis...
I (10000) TIMER_CHALLENGE: ⏱️ Timer drift detected! Avg interval = 520.25
I (15000) TIMER_CHALLENGE: 🧭 Scheduler Pattern #0
I (15000) TIMER_CHALLENGE: 🟢 Increasing Sync Speed
I (20000) TIMER_CHALLENGE: 💥 Simulated Timer Error Detected!
I (20000) TIMER_CHALLENGE: 🔁 Attempting Recovery...
I (20500) TIMER_CHALLENGE: 🔄 Sync Timer #25 | Interval: 300 ticks | Avg: 490.00


---

## 📷 ผลลัพธ์ที่คาดหวัง

I (1000) TIMER: Blink LED
I (2000) TIMER: Heartbeat LED
I (5000) TIMER: Status update #1
I (7000) TIMER: Adjusting blink period to 300ms
I (10000) TIMER: One-shot trigger -> Dynamic Timer created
I (13000) TIMER: Dynamic Timer event, all LEDs flash

yaml
คัดลอกโค้ด

---