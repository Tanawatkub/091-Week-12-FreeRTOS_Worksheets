## 🔍 Objective
เพื่อศึกษาการทำงานของระบบ **Software Timer** บน FreeRTOS โดยรวมหลายระบบย่อยเข้าด้วยกัน ได้แก่  
- Watchdog Monitoring  
- LED Pattern Evolution  
- Sensor Adaptive Sampling  
- System Health Monitoring  

---

## ⚙️ System Overview
| Component | Function | Period (ms) | Type |
|------------|-----------|-------------|------|
| Watchdog Timer | ตรวจสอบการทำงานของระบบหลัก | 5000 | One-shot |
| Feed Timer | ป้อนค่า watchdog เพื่อป้องกัน timeout | 2000 | Auto-reload |
| Pattern Timer | ควบคุมรูปแบบไฟกระพริบ LED | 500 | Auto-reload |
| Sensor Timer | อ่านค่า ADC และส่งข้อมูลเข้า queue | 1000 (adaptive) | Auto-reload |
| Status Timer | แสดงสถานะระบบทุกช่วงเวลา | 3000 | Auto-reload |

---

## 🧪 การทดลอง

### 🧩 ทดลองที่ 1: Watchdog System
**ขั้นตอน:**
1. สังเกตการทำงานของ Watchdog ปกติ  
2. ในรอบที่ 15 ระบบจะ “simulate hang” โดยหยุด feed timer ชั่วคราว  
3. Watchdog Timer จะหมดเวลาและแสดง “🚨 WATCHDOG TIMEOUT”  
4. หลังจาก 8 วินาที Recovery Timer จะทำงานและระบบกลับมาปกติ  

**ผลการสังเกต:**
- มีการ Flash ไฟ `WATCHDOG_LED` อย่างรวดเร็วเมื่อ Timeout  
- Log แสดงการ Feed, Timeout และ Recovery ถูกต้องตามลำดับ  
- ค่า `health_stats.watchdog_timeouts` เพิ่มขึ้น 1 ครั้ง  

---

### 💡 ทดลองที่ 2: LED Pattern Evolution
**ขั้นตอน:**
1. สังเกต Pattern ที่เปลี่ยนทุก 50 cycles  
2. ตรวจสอบการทำงานของ pattern ต่าง ๆ  
3. ดูการตอบสนองเมื่อ sensor เปลี่ยนค่า  

**ผลการสังเกต:**
| Pattern | ลักษณะการกระพริบ | ความถี่โดยประมาณ |
|----------|-------------------|--------------------|
| OFF | ดับทั้งหมด | - |
| SLOW_BLINK | LED1 ช้า 1s | 1Hz |
| FAST_BLINK | LED2 เร็ว 0.2s | 5Hz |
| HEARTBEAT | LED3 double pulse | 10 steps |
| SOS | ... --- ... | Morse cycle |
| RAINBOW | LED1-3 สลับสีวน | 8 steps |

**Pattern sequence:**  
`OFF → SLOW_BLINK → FAST_BLINK → HEARTBEAT → SOS → RAINBOW → (repeat)`

---

### 🌡️ ทดลองที่ 3: Sensor Adaptive Sampling
**ขั้นตอน:**
1. แตะหรือเปลี่ยนแรงดัน ADC Pin  
2. สังเกต Sampling Rate ที่เปลี่ยนแปลง  
3. ตรวจสอบ Warning log เมื่อค่าเกิน threshold  

**ผลการสังเกต:**
| อุณหภูมิจำลอง (°C) | Sampling Period | หมายเหตุ |
|----------------------|-----------------|-----------|
| < 25°C | 2000 ms | Low activity |
| 25–40°C | 1000 ms | Normal |
| > 40°C | 500 ms | High alert |

**ผลลัพธ์:**
- ค่าเฉลี่ยคำนวณทุก 10 ตัวอย่าง  
- แสดง Warning `"🔥 High temperature warning!"` เมื่อค่าเฉลี่ย > 35°C  
- แสดง `"🧊 Low temperature warning!"` เมื่อค่าเฉลี่ย < 15°C  

---

### 📊 ทดลองที่ 4: System Health Monitoring
**ขั้นตอน:**
1. สังเกต Log จาก Status Timer ทุก 3 วินาที  
2. ตรวจสอบค่าของ metrics ที่เปลี่ยนแปลง  
3. วิเคราะห์ Performance และ Heap Memory  

**ตัวอย่าง Log:**
═══════ SYSTEM STATUS ═══════
Uptime: 63 seconds
System Health: ✅ HEALTHY
Watchdog Feeds: 30
Watchdog Timeouts: 1
Pattern Changes: 6
Sensor Readings: 85
Current Pattern: 4
Timer States:
Watchdog: ACTIVE
Feed: ACTIVE
Pattern: ACTIVE
Sensor: ACTIVE
Status: ACTIVE
════════════════════════════

yaml
คัดลอกโค้ด

---

## 📈 การวิเคราะห์ผล

### Performance Metrics (Expected)
| Metric | ค่าที่คาดหวัง | ค่าที่ได้ |
|---------|----------------|------------|
| Watchdog Feeds | ~30 feeds/min | ✅ 30–31 |
| Pattern Changes | ~6 changes/min | ✅ ตรงตามรอบ |
| Sensor Readings | 60–120 readings/min | ✅ Adaptive |
| System Uptime | ต่อเนื่อง | ✅ ต่อเนื่อง |

### System Analysis
- การใช้หลาย Timer พร้อมกันทำงานได้ราบรื่น (ไม่มี drift)  
- การใช้ Queue ช่วยจัดการ Sensor Data โดยไม่สูญหาย  
- Adaptive Sampling ทำให้ระบบใช้พลังงานมีประสิทธิภาพมากขึ้น  

---

## 💬 Post-Lab Questions

| คำถาม | คำตอบ |
|--------|--------|
| **1. ทำไมต้องใช้ separate timer สำหรับ feeding watchdog?** | เพื่อป้องกันไม่ให้ Task หลักหรือ Timer อื่นที่ค้างส่งผลให้ watchdog หยุดทำงาน ระบบ feed แยกจึงมั่นใจได้ว่า watchdog ถูกรีเซ็ตสม่ำเสมอ |
| **2. อธิบายการเลือก Timer Period สำหรับแต่ละ pattern** | กำหนดตามลักษณะของการกระพริบ เช่น slow blink 1000 ms, fast blink 200 ms เพื่อให้เห็นความต่างชัดเจน |
| **3. ประโยชน์ของ Adaptive Sampling Rate คืออะไร?** | ลดการใช้พลังงานและเพิ่มความละเอียดเมื่อ sensor มีการเปลี่ยนแปลงอย่างรวดเร็ว |
| **4. Metrics ใดบ้างที่ควรติดตามในระบบจริง?** | Watchdog feed/timeout count, average sensor reading rate, heap memory usage, uptime, และ system health flag |

---

## 🚀 ความท้าทายเพิ่มเติม
- เพิ่ม Pattern ที่ซับซ้อนแบบ fading หรือ breathing  
- เพิ่ม Sensor หลายตัวพร้อม Priority Queue  
- เชื่อมต่อ Network Watchdog เพื่อส่ง Heartbeat ผ่าน WiFi  
- ใช้ Machine Learning เพื่อเรียนรู้ pattern การเปลี่ยนแปลงของ sensor  

---

## 📚 Key Learning Points
✅ การใช้งาน Watchdog Timer แบบซ้อนระบบ  
✅ การสร้างและควบคุม Complex LED Pattern  
✅ การเปลี่ยนแปลง Sampling Rate แบบ Adaptive  
✅ การจัดการ Multi-Timer ด้วย FreeRTOS  
✅ การ Monitor และ Log สถานะระบบ  
✅ การจำลองสถานการณ์จริงที่มีการ hang และ recovery  

---