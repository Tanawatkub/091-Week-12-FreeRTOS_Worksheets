# Lab 3: Stack Monitoring และ Debugging — Report

## 🔍 คำถามวิเคราะห์

1. **Task ไหนใช้ stack มากที่สุด? เพราะอะไร?**  
   → Heavy Task ใช้ stack มากที่สุด เพราะมีการประกาศ local array ขนาดใหญ่ภายใน loop (1024 + 512 bytes)

2. **การใช้ heap แทน stack มีข้อดีอย่างไร?**  
   → ช่วยลดการใช้หน่วยความจำ stack ของแต่ละ task และสามารถจัดการขนาดได้ยืดหยุ่นกว่า

3. **Stack overflow เกิดขึ้นเมื่อไหร่ และทำอย่างไรป้องกัน?**  
   → เกิดเมื่อ task ใช้หน่วยความจำเกิน stack ที่จัดสรรไว้  
   → ป้องกันโดย:
   - ใช้ heap สำหรับข้อมูลขนาดใหญ่  
   - ตรวจสอบ stack เหลือ (`uxTaskGetStackHighWaterMark`)  
   - เปิดใช้งาน `CONFIG_FREERTOS_CHECK_STACKOVERFLOW=2`

4. **การตั้งค่า stack size ควรพิจารณาจากอะไร?**  
   → จากจำนวนตัวแปร local, ฟังก์ชันซ้อนลึก (recursion depth) และความซับซ้อนของโค้ด

5. **Recursion ส่งผลต่อ stack usage อย่างไร?**  
   → ใช้ stack มากขึ้นในแต่ละระดับการเรียก เพราะต้องเก็บ return address และ local variables ทุกชั้น

---

## 📊 ผลการทดลองที่คาดหวัง

| Task | Stack Size | Usage | Warning |
|------|------------|--------|---------|
| Light | 1024 | ~200–400 bytes | Safe |
| Medium | 2048 | ~600–800 bytes | Safe |
| Heavy | 2048 | ~1500–1800 bytes | Warning |
| Recursion | 3072 | เพิ่มตาม depth | Monitor |

---

## 💡 บทสรุป

**สิ่งที่ได้เรียนรู้จาก Lab นี้:**
1. เข้าใจการจัดการ stack memory ใน FreeRTOS  
2. สามารถตรวจสอบ stack usage ได้จริงใน runtime  
3. รู้จักการใช้ heap แทน stack เพื่อ optimization  
4. รู้วิธีป้องกันและ debug stack overflow  
5. สามารถใช้ LED และ Serial log เป็นเครื่องมือช่วยสังเกตได้

**Key Takeaways:**
- ออกแบบ stack size อย่างรอบคอบ  
- ใช้ heap สำหรับข้อมูลใหญ่  
- เปิดใช้การตรวจสอบ stack overflow (`CONFIG_FREERTOS_CHECK_STACKOVERFLOW=2`)  
- ตรวจสอบ stack usage เป็นประจำในการพัฒนา embedded systems
