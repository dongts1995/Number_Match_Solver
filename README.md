# Number Match Solver

CLI solver C++17 cho bang Number Match 9 cot.

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run

Input la chuoi cac so lien nhau tu `1` den `9`. 44 so dau tao bang ban dau va duoc phep hoan toan random; khong co yeu cau so luong cua cac nhom `(1,9)`, `(2,8)`, `(3,7)`, `(4,6)` hoac `5` phai chan. Neu input co nhieu hon 44 so, moi lan bam `+` se them toi da mot hang tiep theo gom 9 so tu phan con lai.

```powershell
.\build\Release\number_match_solver.exe 11265493...
```

Hoac pipe input:

```powershell
"11265493..." | .\build\Release\number_match_solver.exe
```

Output la danh sach step xoa, gom toa do 1-based, gia tri, huong, khoang cach va viec co day hang hay khong. Solver thu nuoc di theo thu tu greedy: cheo, xa, khong day hang; sau do backtrack de tim loi giai thang co it buoc xoa nhat. Du lieu random co the khong co solution; khi do solver se bao khong tim thay trong gioi han 5 lan bam `+`.