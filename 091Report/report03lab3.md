# 🧪 03Lab3: Queue Sets Implementation

## 🎯 วัตถุประสงค์
- เข้าใจการใช้งาน Queue Sets ใน FreeRTOS  
- เรียนรู้การรอข้อมูลจากหลาย Queue พร้อมกัน  
- ฝึกการจัดการ Multiple Input Sources  
- เข้าใจ Event-driven Programming Pattern  

---

## 📊 การสังเกตและบันทึกผล

### ตารางผลการทดลอง

| ทดลอง | Sensor | User | Network | Timer | Total | สังเกต |
|-------|--------|------|---------|-------|-------|---------|
| 1 (ปกติ) | 15 | 10 | 8 | 5 | 38 | ทุกคิวส่งข้อมูลสลับกัน Processor จัดการได้เรียงลำดับดี ไม่มี Delay |
| 2 (ไม่มี Sensor) | 0 | 12 | 9 | 5 | 26 | Processor ประมวลผลเฉพาะ Queue ที่เหลือ ระบบยังคงเสถียร |
| 3 (Network เร็ว) | 12 | 9 | 18 | 5 | 44 | Network ส่งถี่มาก ทำให้ Queue Network เต็มบ่อย Processor รับถี่ขึ้น แต่ไม่ค้าง |

---

### คำถามสำหรับการทดลอง

**1. Processor Task รู้ได้อย่างไรว่าข้อมูลมาจาก Queue ไหน?**  
> ใช้ฟังก์ชัน `xQueueSelectFromSet()` ที่จะคืนค่า handle ของ Queue หรือ Semaphore ที่มีข้อมูลเข้ามา จากนั้นโปรแกรมจะเปรียบเทียบว่าเป็น Queue ไหน เช่น `if (xActivatedMember == xSensorQueue)`  

**2. เมื่อหลาย Queue มีข้อมูลพร้อมกัน เลือกประมวลผลอันไหนก่อน?**  
> FreeRTOS จะเลือกตามลำดับ Queue ที่อยู่ใน Set ก่อน (FIFO ของ Queue Set) ไม่ใช่ตาม Priority ของ Task เว้นแต่เราจะออกแบบเพิ่มเอง เช่น มีการจัด Priority Handling เพิ่มเติมใน Processor  

**3. Queue Sets ช่วยประหยัด CPU อย่างไร?**  
> โดยปกติหากไม่มี Queue Sets เราต้องใช้ `xQueueReceive()` ตรวจแต่ละคิวแบบ polling ซึ่งกิน CPU มาก แต่ Queue Sets จะให้ Task รอเพียง event เดียว โดย block อยู่ใน `xQueueSelectFromSet()` ทำให้ไม่ต้องวนเช็คทุกคิวและลดการใช้ CPU ลงมาก  

---

## 📋 สรุปผลการทดลอง

### สิ่งที่เรียนรู้:
- [x] การใช้งาน Queue Sets เพื่อรวมหลายคิวเข้าด้วยกัน  
- [x] Event-driven Programming ที่ใช้รอเหตุการณ์จากหลายแหล่งพร้อมกัน  
- [x] การจัดการ Multiple Input Sources ให้ประมวลผลใน Task เดียว  
- [x] การ Multiplex ข้อมูลแบบไม่ต้อง polling  
- [x] การประหยัด CPU เมื่อระบบมีหลาย Input  

---

### APIs ที่ใช้:
| API | หน้าที่ |
|------|----------|
| `xQueueCreateSet()` | สร้าง Queue Set เพื่อรวมหลายคิวเข้าด้วยกัน |
| `xQueueAddToSet()` | เพิ่ม Queue/Semaphore เข้าไปใน Queue Set |
| `xQueueSelectFromSet()` | รอข้อมูลจาก Queue Set ใด ๆ แบบ blocking |
| `xQueueRemoveFromSet()` | ลบ Queue/Semaphore ออกจาก Set เมื่อไม่ใช้ |

---

### ข้อดีของ Queue Sets:
1. **Efficiency** – ประหยัด CPU เพราะไม่ต้องวนเช็คทุกคิว (polling)  
2. **Scalability** – เพิ่ม/ลบ input source ได้ง่าย โดยไม่ต้องสร้าง Task ใหม่  
3. **Simplicity** – โค้ดอ่านง่าย ไม่ซับซ้อนเมื่อมีหลาย event source  
4. **Responsiveness** – ตอบสนองต่อเหตุการณ์ได้รวดเร็วขึ้นทันทีเมื่อมีข้อมูล  

---

## 🚀 ความท้าทายเพิ่มเติม

| ลำดับ | ความท้าทาย | สิ่งที่ปรับเพิ่ม | ผลการทำงานที่สังเกตได้ |
|--------|--------------|--------------------|--------------------------|
| 1️⃣ | **Priority Handling** | จัดลำดับความสำคัญของ Queue ให้ Network มี Priority สูงกว่า | เมื่อ Network และ User มีข้อมูลพร้อมกัน ระบบจะประมวลผล Network ก่อน |
| 2️⃣ | **Dynamic Queue Management** | สร้าง/ลบ Queue ระหว่างทำงาน เช่น ปิด Network Queue ชั่วคราว | Processor สามารถทำงานต่อได้โดยไม่ crash |
| 3️⃣ | **Load Balancing** | เพิ่ม Processor ที่ 2 เพื่อแบ่งงาน Network Priority สูง | ลดภาระ Processor หลัก ระบบเสถียรแม้โหลดสูง |
| 4️⃣ | **Event Filtering** | ข้าม event ที่ priority ต่ำกว่า 2 | ลดการใช้ CPU และลด delay ของ event สำคัญ |
| 5️⃣ | **Performance Metrics** | วัดเวลาตอบสนองแต่ละ event (Response Time) | ค่าตอบสนองเฉลี่ย ~1.3ms แสดงประสิทธิภาพของระบบ |




I (1000) QUEUESETS_ADV: Starting Advanced Queue Set System...
🌐 Network queued: Alert message (P:5)
🔘 User queued: Button 3 pressed
📊 Sensor queued: Temp: 33.2 C
🔬 [CPU1] Sensor processed: Temp: 33.2 C (1.12ms)
🧍 [CPU1] User processed: Button 3 pressed (1.44ms)
⚡ Redirecting high P(5) to Processor 2
🧠 [CPU2] handled Network: Alert message (lat: 1.83ms)
📊 SYSTEM STATS
Sensor:12  User:8  Network:6  Total:26
Average latency: 1.49 ms
Queues → Sensor:0 User:0 Network:0
...
🧩 Removed Network Queue dynamically!


---

## 🔚 สรุปภาพรวม

> จากการทดลอง **Queue Sets** เป็นแนวทางที่มีประสิทธิภาพในการรวมการรับข้อมูลหลายแหล่งเข้าด้วยกันในระบบ FreeRTOS  
> Task เดียวสามารถจัดการ Event ได้หลากหลายชนิด ทำให้ระบบตอบสนองเร็วขึ้นและใช้ทรัพยากรน้อยลง  
> เมื่อผนวกกับ **Priority Handling, Load Balancing และ Performance Metrics** ระบบสามารถประมวลผลได้อย่างมีประสิทธิภาพ เหมาะกับการพัฒนา IoT, Sensor Gateway และ Edge Computing  

---

📘 **แหล่งอ้างอิง**
- [FreeRTOS Queue Sets Documentation](https://www.freertos.org/RTOS-queue-sets.html)  
- [ESP-IDF FreeRTOS Examples](https://github.com/espressif/esp-idf/tree/master/examples/system/freertos)  
- [Event-driven Architecture Overview](https://en.wikipedia.org/wiki/Event-driven_programming)
