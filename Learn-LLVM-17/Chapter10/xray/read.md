## 命令
```bash
clang -fxray-instrument -fxray-instruction-threshold=1 -g xraydemo.c -o xraydemo
XRAY_OPTIONS="patch_premain=true xray_mode=xray-basic" ./xraydemo
llvm-xray account xray-log.xraydemo.GWYJgE --sort=count --sortorder=dsc --instr_map ./xraydemo
llvm-xray stack xray-log.xraydemo.GWYJgE -instr_map ./xraydemo
# 没下载 flamegraph.pl
llvm-xray stack xray-log.xraydemo.GWYJgE --all-stacks --stack-format=flame --aggregation-type=time --instr_map ./xraydemo
```