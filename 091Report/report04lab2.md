## 📊 การสังเกตและบันทึกผล

### ตารางผลการทดลอง
| ทดลอง | Successful | Failed | Corrupted | Success Rate | สังเกต |
|-------|-------------|---------|-----------|---------------|---------|
| 1 (With Mutex) | 42 | 2 | 0 | 95.4% | ไม่มีการ corruption ตรวจพบ Mutex ทำงานได้ดี ป้องกันการชนกันของ data ทุกครั้ง |
| 2 (No Mutex) | 39 | 0 | 14 | 73.6% | เกิดการ corruption หลายครั้งจาก race condition เพราะหลาย task เข้าถึงพร้อมกัน |
| 3 (Changed Priority) | 45 | 1 | 1 | 95.7% | เมื่อสลับ priority พบว่า low priority task ถูกขัดจังหวะบ่อยแต่ระบบยังคงป้องกัน corruption ได้ดีด้วย priority inheritance |

---

### ❓ คำถามสำหรับการทดลอง

1. **เมื่อไม่ใช้ Mutex จะเกิด data corruption หรือไม่?**  
   - ใช่ เกิด data corruption แน่นอน เพราะหลาย task เข้าถึง shared resource พร้อมกันโดยไม่มีการล็อก ทำให้ค่า `counter` และ `checksum` ไม่ตรงกัน  
   - ตัวตรวจ checksum จับได้ว่าค่าที่อ่านไม่สอดคล้องกับที่คำนวณไว้

2. **Priority Inheritance ทำงานอย่างไร?**  
   - เมื่อ task ที่มี priority ต่ำถือ mutex อยู่ และ task ที่มี priority สูงต้องการใช้ mutex ตัวนั้น  
     → FreeRTOS จะ “ยก priority” ของ task ที่ถือ mutex ให้เท่ากับ task สูงสุดที่รออยู่  
     → เพื่อป้องกันไม่ให้ task priority ต่ำถูกแย่ง CPU โดย task อื่น (medium priority) ก่อนที่จะคืน mutex  
   - หลังจากคืน mutex แล้ว priority ของ task จะกลับสู่ค่าปกติ

3. **Task priority มีผลต่อการเข้าถึง shared resource อย่างไร?**  
   - Task ที่มี priority สูงจะมีโอกาสได้ mutex ก่อน เพราะ scheduler ให้สิทธิ์รันก่อน  
   - แต่เนื่องจาก mutex ใช้ระบบ inheritance การเปลี่ยนลำดับ priority ยังไม่ทำให้ข้อมูลพัง  
   - การตั้ง priority ที่สมดุลจึงสำคัญต่อ performance และ fairness ของระบบ

---

## 📋 สรุปผลการทดลอง

### ✅ สิ่งที่เรียนรู้
- การใช้ **Mutex** สามารถป้องกันการเกิด Race Condition ได้อย่างมีประสิทธิภาพ  
- **Priority Inheritance** ช่วยลดปัญหา Priority Inversion เมื่อ task priority ต่ำถือ resource ไว้นาน  
- **Critical Section** ควรถูกออกแบบให้สั้นและปลอดภัย  
- การปิด mutex ชั่วคราวช่วยให้เห็นผลของ data corruption อย่างชัดเจน  
- การตรวจ **Checksum** เป็นวิธีดีในการยืนยันว่าข้อมูลไม่ถูกเปลี่ยนระหว่าง task  


- [✅] หลักการทำงานของ Mutex
- [✅] การป้องกัน Race Condition
- [✅] Priority Inheritance Mechanism
- [✅] การตรวจจับ Data Corruption
- [✅] Critical Section Management

---

### 🧰 APIs ที่ใช้
| API | คำอธิบาย |
|-----|------------|
| `xSemaphoreCreateMutex()` | สร้าง mutex สำหรับป้องกัน resource |
| `xSemaphoreTake()` | ขอ mutex (เข้า critical section) |
| `xSemaphoreGive()` | คืน mutex (ออกจาก critical section) |
| `uxSemaphoreGetCount()` | ตรวจสอบสถานะ mutex |
| `vTaskDelay()` | หน่วงเวลาเพื่อจำลองการประมวลผล |

---

### ⚙️ ความแตกต่างระหว่าง Mutex และ Binary Semaphore
| คุณสมบัติ | Mutex | Binary Semaphore |
|------------|--------|------------------|
| Owner | มี (task ที่ถือ mutex) | ไม่มี |
| Priority Inheritance | มี (ป้องกัน priority inversion) | ไม่มี |
| Recursive Locking | ทำได้ (`xSemaphoreCreateRecursiveMutex()`) | ทำไม่ได้ |
| การใช้งานหลัก | ป้องกันการเข้าถึง resource ซ้ำ | ใช้ signal/event ระหว่าง task |

---

## 🚀 ความท้าทายเพิ่มเติม

1. **Recursive Mutex:**  
   ทดลองใช้ `xSemaphoreCreateRecursiveMutex()` และเขียนฟังก์ชันซ้อนกันสองชั้นเพื่อทดสอบ nested lock  

2. **Deadlock Prevention:**  
   สร้าง resource สองตัว (Mutex A และ Mutex B) แล้วบังคับให้สอง task ขอ mutex ในลำดับต่างกัน เพื่อดู deadlock จากนั้นแก้โดยเรียงลำดับการ lock ให้เหมือนกัน  

3. **Performance Impact:**  
   ใช้ `esp_timer_get_time()` วัดเวลาการเข้าถึง critical section พร้อมกับจำนวน task ที่รอ mutex เพื่อดู overhead ของ synchronization  

4. **Multiple Resources:**  
   เพิ่ม resource หลายตัว (เช่น shared_buffer และ shared_counter) แล้วใช้ mutex แยกกัน เพื่อดูประสิทธิภาพเมื่อ resource แยกอิสระ  

5. **Lock-free Programming:**  
   ทดลองใช้ atomic operations (`portENTER_CRITICAL()` / `portEXIT_CRITICAL()`) แทน mutex เพื่อเปรียบเทียบความเร็วและผลลัพธ์  


I (1000) MUTEX_CHALLENGE: 💡 Challenge system initialized!
I (1010) MUTEX_CHALLENGE: High Priority task started
I (1012) MUTEX_CHALLENGE: Medium Priority task started
I (1014) MUTEX_CHALLENGE: Low Priority task started
I (3200) MUTEX_CHALLENGE: [HIGH] requesting first mutex...
I (3400) MUTEX_CHALLENGE: [HIGH] requesting second mutex...
I (3650) MUTEX_CHALLENGE: [HIGH] working safely in double-locked section
I (4100) MUTEX_CHALLENGE: HIGH recursion depth 1
I (4200) MUTEX_CHALLENGE: HIGH recursion depth 2
I (4300) MUTEX_CHALLENGE: HIGH recursion depth 3
I (5000) MUTEX_CHALLENGE: [LOW] requesting first mutex...
I (5200) MUTEX_CHALLENGE: [LOW] requesting second mutex...
E (5400) MUTEX_CHALLENGE: [LOW] 💀 Deadlock detected!
I (6000) MUTEX_CHALLENGE: [HIGH] working safely in double-locked section
I (10000) MUTEX_CHALLENGE:
==== MUTEX CHALLENGE STATS ====
Access OK         : 4
Deadlocks Detected: 1
Recursive Uses    : 6
===============================

---

### 💬 สรุปท้ายบท
จากผลการทดลองพบว่า **Mutex เป็นกลไกหลักในการป้องกัน Race Condition ที่เชื่อถือได้ใน FreeRTOS**  
โดยเฉพาะเมื่อระบบมีหลาย task และ priority ต่างกัน  
การเปิด/ปิด mutex แสดงให้เห็นถึงความแตกต่างอย่างชัดเจนในด้าน data consistency และ safety ของระบบ

---
