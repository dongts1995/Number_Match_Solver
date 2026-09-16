# Interactive Pair Tool

Tool CLI de xem cac cap available trong ma tran Number Match va tu chon cap can xoa.

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run

```powershell
.\build\Release\interactive_pair_tool.exe 27878416851413235132724968341595351527861242
```

Moi luot tool se:

1. In ma tran hien tai.
2. Liet ke cac cap hop le theo thu tu uu tien cheo, xa, khong xoa hang.
3. Cho nguoi dung nhap so thu tu cua cap.
4. Xoa cap da chon va xoa hang neu hang da toan o xam.
5. Tao lai danh sach cap cho ma tran moi.
6. Cho phep nhap `+` de sao chep toan bo so con lai va noi tiep vao cuoi ma tran.

Lenh `+` duoc phep toi da 5 lan. Cac o xam duoc giu nguyen; danh sach cap duoc tinh lai sau moi lan them so.

Nhap `q` de thoat.