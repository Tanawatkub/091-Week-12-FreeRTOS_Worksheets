# Lab 2: Task States Demonstration — Report

## 🔍 คำถามวิเคราะห์

### 1. Task อยู่ใน Running state เมื่อไหร่บ้าง?
> เมื่อ Task ได้รับ CPU และกำลังประมวลผลคำสั่ง (ไม่ได้ yield หรือ delay)

### 2. ความแตกต่างระหว่าง Ready และ Blocked state คืออะไร?
> - **Ready:** พร้อมทำงานแต่ยังไม่ได้รับ CPU  
> - **Blocked:** รอ event เช่น semaphore, delay หรือ input

### 3. การใช้ vTaskDelay() ทำให้ task อยู่ใน state ใด?
> ทำให้ Task เข้า **Blocked state** ชั่วคราวตามเวลาที่กำหนด

### 4. การ Suspend task ต่างจาก Block อย่างไร?
> - **Suspend:** หยุดด้วยการสั่งจาก API โดยตรง  
> - **Block:** หยุดเพราะรอ event หรือ resource  
> (Suspend ต้อง resume เอง ส่วน Block จะกลับมาเองเมื่อ event เกิด)

### 5. Task ที่ถูก Delete จะกลับมาได้หรือไม่?
> ไม่ได้ ต้องถูกสร้างใหม่ด้วย `xTaskCreate()` เท่านั้น

---

## 📊 ผลการทดลองที่คาดหวัง

| State | เงื่อนไข | LED | ระยะเวลา |
|-------|-----------|-----|-----------|
| Running | ทำงานจริง | GPIO2 | สั้น |
| Ready | รอ CPU | GPIO4 | สั้นมาก |
| Blocked | รอ semaphore/delay | GPIO5 | ยาว |
| Suspended | กดปุ่ม | GPIO18 | จน resume |

---

## 💡 บทสรุป

การทดลองนี้แสดงให้เห็นว่า:
1. Task ใน FreeRTOS มีวงจรชีวิตหลายสถานะ
2. การเปลี่ยน state เกิดขึ้นอัตโนมัติจากการทำงาน เช่น delay หรือ wait
3. Suspend / Resume ใช้ควบคุม manual state ได้
4. การ monitoring task ช่วยในการ debug ระบบ multitasking

**Key Takeaways:**
- เข้าใจ 5 states: Running, Ready, Blocked, Suspended, Deleted  
- ใช้ LED ช่วยแสดงผลการเปลี่ยน state ได้ชัดเจน  
- สามารถวิเคราะห์การทำงานของ scheduler ได้จาก Serial Log
