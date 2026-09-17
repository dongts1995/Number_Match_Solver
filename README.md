# Number Match Solver

CLI solver C++17 cho bang Number Match 9 cot.

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run

Input la chuoi cac so lien nhau tu `1` den `9`. Khong gioi han so luong so ban dau; neu input co nhieu hon so cho phep trong mot hang, moi lan bam `+` se them toi da mot hang tiep theo gom 9 so tu phan con lai.

```powershell
.\build\Release\number_match_solver.exe 11265493...
```

Hoac pipe input:

```powershell
"11265493..." | .\build\Release\number_match_solver.exe
```

Output la danh sach step xoa, gom toa do 1-based, gia tri, huong, khoang cach va viec co day hang hay khong. Solver thu nuoc di theo thu tu greedy: cheo, xa, khong day hang; sau do backtrack de tim loi giai thang co it buoc xoa nhat. Du lieu random co the khong co solution; khi do solver se bao khong tim thay trong gioi han 5 lan bam `+`.