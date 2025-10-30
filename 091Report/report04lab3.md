## 📊 การสังเกตและบันทึกผล

### ตารางผลการทดลอง
| ทดลอง | Resources | Producers | Success Rate | Avg Wait | Resource Utilization | สังเกต |
|-------|------------|------------|---------------|-----------|----------------------|---------|
| 1 (3R, 5P) | 3 | 5 | 91.2% | ~450 ms | 95% | ระบบทำงานสมดุล มีคิวรอเป็นช่วง ๆ แต่ไม่มีการล้มเหลวร้ายแรง |
| 2 (5R, 5P) | 5 | 5 | 99.4% | ~120 ms | 82% | เมื่อเพิ่ม Resource ระบบทำงานได้ราบรื่นขึ้นมาก และ Producer แทบไม่ต้องรอ |
| 3 (3R, 8P) | 3 | 8 | 76.8% | ~780 ms | 100% | ทรัพยากรถูกใช้เต็ม 100% มีหลาย Producer รอ และเกิด timeout บ้างเมื่อ Load สูง |

---

### 🧪 ตัวอย่าง Log Output

I (1000) COUNTING_SEM: Counting Semaphores Lab Starting...
I (1100) COUNTING_SEM: Counting semaphore created (max count: 3)
I (1200) COUNTING_SEM: Producer1 started
I (1210) COUNTING_SEM: Producer2 started
I (1220) COUNTING_SEM: Producer3 started
I (1230) COUNTING_SEM: Producer4 started
I (1240) COUNTING_SEM: Producer5 started
I (5000) COUNTING_SEM: 🏭 Producer2: Requesting resource...
I (5010) COUNTING_SEM: ✓ Producer2: Acquired resource 2 (wait: 98ms)
I (5200) COUNTING_SEM: 🔧 Producer2: Using resource 2 for 2430ms
I (6000) COUNTING_SEM: 🏭 Producer5: Requesting resource...
I (6010) COUNTING_SEM: ✓ Producer5: Acquired resource 3 (wait: 112ms)
I (9000) COUNTING_SEM: ⏰ Producer4: Timeout waiting for resource
I (12000) COUNTING_SEM:
📈 SYSTEM STATISTICS
Total requests: 51
Successful acquisitions: 47
Failed acquisitions: 4
Current resources in use: 3
Success rate: 92.1%
Resource utilization:
Resource 1: 16 uses, 48500 total time
Resource 2: 15 uses, 47000 total time
Resource 3: 16 uses, 51000 total time
Total resource usage events: 47
════════════════════════════

yaml
คัดลอกโค้ด

---

### ❓ คำถามสำหรับการทดลอง

1. **เมื่อ Producers มากกว่า Resources จะเกิดอะไรขึ้น?**  
   - จะเกิด **Resource Contention** (มีการแย่งกันใช้ resource)  
   - Producers ที่ไม่ทันจะต้องรอ หรือบางครั้ง timeout หาก semaphore count หมด  
   - ระบบยังคงทำงานต่อเนื่องได้ แต่ efficiency ลดลง

2. **Load Generator มีผลต่อ Success Rate อย่างไร?**  
   - ระหว่าง Load Burst จะมีการสร้างคำขอพร้อมกันจำนวนมาก  
   - ทำให้ semaphore count หมดเร็ว → เกิด timeout เพิ่มขึ้น  
   - หลังช่วงโหลดผ่านไป success rate จะกลับมาสูงขึ้น  
   - ระบบยังคงเสถียรเพราะมีการคืน semaphore ทุกครั้งหลังใช้งาน

3. **Counting Semaphore จัดการ Resource Pool อย่างไร?**  
   - Semaphore จะเก็บจำนวน resource ที่เหลือ (count)  
   - เมื่อ Producer ขอใช้ resource → count ลดลง  
   - เมื่อ Producer คืน resource → count เพิ่มขึ้น  
   - ทำให้มีการจัดสรร resource แบบจำกัดจำนวนโดยไม่เกิด race condition

---

## 📋 สรุปผลการทดลอง

### ✅ สิ่งที่เรียนรู้
- เข้าใจหลักการของ **Counting Semaphore** ในการควบคุมจำนวน resource ที่ใช้งานพร้อมกัน  
- สามารถใช้เป็นระบบจำลอง **Resource Pool Management** ได้จริง  
- เห็นพฤติกรรมของระบบเมื่อ **Load เพิ่มขึ้น** หรือเมื่อ **Resource จำกัด**  
- สามารถวัด **Success Rate / Average Wait / Utilization** เพื่อประเมินประสิทธิภาพระบบ  


- [✅] หลักการทำงานของ Counting Semaphore
- [✅] การจัดการ Resource Pool
- [✅] Load Balancing และ Resource Contention
- [✅] Performance Monitoring และ Statistics
- [✅] Rate Limiting Applications


---

### 🧰 APIs ที่ใช้
| API | คำอธิบาย |
|-----|------------|
| `xSemaphoreCreateCounting()` | สร้าง Counting Semaphore พร้อมกำหนด max count |
| `xSemaphoreTake()` | ขอ resource จาก pool |
| `xSemaphoreGive()` | คืน resource เข้า pool |
| `uxSemaphoreGetCount()` | ตรวจสอบจำนวน resource ที่เหลือ |
| `vTaskDelay()` | จำลองเวลาการทำงานของ task |

---

### ⚙️ การวิเคราะห์เชิงเทคนิค
- **Counting Semaphore** มีความยืดหยุ่นกว่า Binary เพราะสามารถใช้จัดการหลาย resource พร้อมกัน  
- **Resource Pool Efficiency (%)** = (เวลาที่ resource ถูกใช้ / เวลาทั้งหมด) × 100  
- การเลือกค่า timeout ที่เหมาะสมเป็นสิ่งสำคัญเพราะมีผลโดยตรงต่อ responsiveness  
- การเพิ่มจำนวน Producer ช่วยทดสอบความสามารถในการ **Load balancing** ของระบบ  

---

## 🚀 ความท้าทายเพิ่มเติม (Challenge Ideas)
| หัวข้อ | รายละเอียด |
|---------|-------------|
| **Priority Resource Allocation** | ให้ Producer ที่สำคัญได้สิทธิเข้าใช้ก่อน |
| **Dynamic Pool Sizing** | ปรับจำนวน resource ตามโหลดอัตโนมัติ |
| **Resource Health Check** | ตรวจสอบและ disable resource ที่ใช้งานผิดปกติ |
| **Fair Scheduling** | ใช้ queue-based fairness ให้ทุก Producer ได้ใช้เท่า ๆ กัน |
| **Statistics Persistence** | เก็บสถิติ usage ลง flash เพื่อวิเคราะห์ย้อนหลัง |


I (1000) COUNTING_SEM_CHALLENGE: 💡 Counting Semaphore Challenge Started!
I (1200) COUNTING_SEM_CHALLENGE: Producer1 started (priority 1)
I (1210) COUNTING_SEM_CHALLENGE: Producer2 started (priority 2)
I (1220) COUNTING_SEM_CHALLENGE: Producer3 started (priority 1)
I (1230) COUNTING_SEM_CHALLENGE: Producer4 started (priority 2)
I (1240) COUNTING_SEM_CHALLENGE: Producer5 started (priority 1)
I (1250) COUNTING_SEM_CHALLENGE: Producer6 started (priority 2)

I (5000) COUNTING_SEM_CHALLENGE: ✓ Producer2: acquired resource 2
I (5100) COUNTING_SEM_CHALLENGE: ✓ Producer4: acquired resource 3
I (5200) COUNTING_SEM_CHALLENGE: ✓ Producer6: acquired resource 1
I (9000) COUNTING_SEM_CHALLENGE: ⏰ Producer3: timeout waiting for semaphore
E (25000) COUNTING_SEM_CHALLENGE: 💥 Resource 2 FAILED!
I (31000) COUNTING_SEM_CHALLENGE: 🧩 Resource 2 recovered.
I (35000) COUNTING_SEM_CHALLENGE: ⚙️ Load low, increasing available resource slot
I (40000) COUNTING_SEM_CHALLENGE:
📊 SYSTEM STATUS
Requests: 74 | Success: 65 | Fail: 9
Resources failed: 1
Utilization: 88.7%
  Resource 1: FREE
  Resource 2: FREE
  Resource 3: IN USEProducer4
══════════════════════════════


---

### 💬 สรุปท้ายบท
จากผลการทดลอง พบว่า **Counting Semaphore** เป็นกลไกสำคัญในระบบแบบหลาย task  
ที่ต้องการจำกัดจำนวนการเข้าถึง resource อย่างมีประสิทธิภาพ เช่น **Database Pool, Network Socket Pool, หรือ Hardware I/O**  
และสามารถปรับใช้ได้ทั้งใน **Embedded Systems** และ **Server Applications** เพื่อควบคุมโหลดของระบบแบบยั่งยืน  

---