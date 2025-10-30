# 🧠 Week 12: ESP-IDF Basic Project and Environment Setup  
**รายงานโดย:** Tanawat Putta (67030091)  
**รายวิชา:** Microcontroller Applications  

# Lab 1: Task Priority และ Scheduling — Report

## ชื่อโครงการ
**lab1_task_priority_scheduling**

---

## 🔍 คำถามวิเคราะห์

### 1. Priority ไหนทำงานมากที่สุด? เพราะอะไร?
> **Priority 5 (High)** ทำงานมากที่สุด เพราะ FreeRTOS ใช้ **Preemptive Scheduling** ทำให้ Task ที่มี priority สูงสุดจะได้สิทธิ์ CPU ก่อนเสมอ

---

### 2. เกิด Priority Inversion หรือไม่? จะแก้ไขได้อย่างไร?
> เกิดได้เมื่อ Low Priority ถือ resource ที่ High Priority ต้องใช้ → High ต้องรอ  
> **วิธีแก้:** ใช้ **Mutex พร้อม Priority Inheritance** เพื่อให้ Low Priority ถูกยกระดับชั่วคราว

---

### 3. Tasks ที่มี priority เดียวกันทำงานอย่างไร?
> ใช้ระบบ **Round-Robin Scheduling** โดย FreeRTOS จะสลับให้แต่ละ task ได้ใช้ CPU เท่า ๆ กันในช่วงเวลา tick เดียวกัน

---

### 4. การเปลี่ยน Priority แบบ Dynamic ส่งผลอย่างไร?
> Task สามารถถูกเลื่อนลำดับการทำงานได้ขณะรันจริง เช่นเพิ่ม priority ชั่วคราว เพื่อให้ task นั้น preempt task อื่นทันที

---

### 5. CPU utilization ของแต่ละ priority เป็นอย่างไร?
| Priority | Usage (%) | ลักษณะการทำงาน |
|-----------|------------|----------------|
| 5 (High) | ~40–50% | ทำงานถี่มาก |
| 3 (Medium) | ~25–35% | ถูก preempt บ่อย |
| 1 (Low) | ~15–25% | ทำงานเมื่อ CPU ว่าง |

---

## 🧠 สรุปผลการทดลอง

1. **Priority-based Scheduling:** Task ที่ priority สูงกว่าได้ CPU ก่อน  
2. **Preemptive Nature:** RTOS จะขัดจังหวะ task ที่ต่ำกว่า  
3. **Round-Robin:** Tasks ที่ priority เท่ากันจะถูกสลับรัน  
4. **Priority Inversion:** ปัญหาที่ต้องป้องกันด้วย Mutex  
5. **Dynamic Priority:** สามารถควบคุมลำดับการทำงานขณะระบบรันอยู่ได้

---

**สรุปใจความสำคัญ:**  
> การจัดการ priority อย่างมีเหตุผลช่วยให้ระบบ FreeRTOS ทำงานมีประสิทธิภาพ ไม่เกิด starvation และหลีกเลี่ยงปัญหา deadlock หรือ inversion
