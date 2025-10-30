# 🧪 Lab 2: Producer–Consumer System

## 📊 ผลการทดลอง (จากการรันจริง)

| ทดลอง | Producers | Consumers | Produced | Consumed | Dropped | Efficiency |
|--------|------------|------------|-----------|-----------|-----------|-------------|
| 1 | 3 | 2 | 15 | 13 | 0 | 86.7% |
| 2 | 4 | 2 | 40 | 26 | 5 | 65.0% |
| 3 | 3 | 1 | 28 | 12 | 10 | 42.8% |

---

## 🔍 คำถามและคำตอบ

**1. ในทดลองที่ 2 เกิดอะไรขึ้นกับ Queue?**  
Queue เริ่มเต็มบ่อย เพราะ Producer เร็วกว่า Consumer → เกิด backlog และ drop message บางส่วน

**2. ในทดลองที่ 3 ระบบทำงานเป็นอย่างไร?**  
Consumer เหลือแค่ 1 → Queue เต็มตลอดเวลา, Efficiency ต่ำ, มี drop เยอะ

**3. Load Balancer แจ้งเตือนเมื่อไหร่?**  
เมื่อ Queue มีข้อมูลค้างมากกว่า 8 รายการ → แจ้งเตือน “⚠️ HIGH LOAD DETECTED!”

---

## ✅ สรุปสิ่งที่เรียนรู้
- [✅] Producer-Consumer Pattern
- [✅] การจัดการ Multiple Producers/Consumers
- [✅] การใช้ Mutex สำหรับ Synchronized Output
- [✅] การวิเคราะห์ประสิทธิภาพระบบ
- [✅] การตรวจจับ Bottleneck

---


## 🚀 ความท้าทายเพิ่มเติม

1. **Dynamic Load Balancing**: เพิ่ม/ลด Consumer ตามโหลด
2. **Product Categories**: แยก Queue ตามประเภทสินค้า
3. **Quality Control**: เพิ่ม Inspection Task
4. **Batch Processing**: ประมวลผลเป็น batch
5. **Network Integration**: ส่งข้อมูลผ่าน WiFi

ผลการรัน 
I (1000) LAB2_PROD_CONS: 🚀 03Lab2 Producer-Consumer with Challenges Starting...
🍳 Producer 1 started
🍳 Producer 2 started
🔍 QC task started
👷 Consumer 1 started
👷 Consumer 2 started
📊 Stats: Prod=0 | Cons=0 | Drop=0 | QC_Fail=0 | FoodQ=0 | DrinkQ=0

✅ P1 Produced: Product#0 (Food)
🧪 QC Passed: Product#0 (Food)
✅ P2 Produced: Product#0 (Drink)
🧪 QC Passed: Product#0 (Drink)
📦 Consumer 1 processing batch (2 items)
→ Consumed: Product#0 (Food)
→ Consumed: Product#0 (Drink)
📡 Consumer 1 sent batch to server

⚡ High load (9 items), adding extra consumer!
👷 Consumer 3 started
📦 Consumer 3 processing batch (3 items)
→ Consumed: Product#5 (Drink)
→ Consumed: Product#6 (Food)
→ Consumed: Product#7 (Drink)
📡 Consumer 3 sent batch to server

💤 Low load (1 items), removing dynamic consumer.
❌ QC Failed: Product#10 (Food)
📊 Stats: Prod=20 | Cons=18 | Drop=1 | QC_Fail=1 | FoodQ=1 | DrinkQ=0



## ⚙️ สรุปการทำงานโดยรวม
ระบบ Producer–Consumer ช่วยให้แต่ละ Task ทำงานแบบอิสระและไม่ชนกัน  
เมื่อระบบไม่สมดุล จะเห็นได้จาก Queue เต็ม (หรือว่าง) และ Efficiency ลดลงอย่างชัดเจน  
ผลนี้ทำให้เข้าใจแนวคิด **Synchronization + Task Scheduling** ของ FreeRTOS ได้ลึกซึ้ง
