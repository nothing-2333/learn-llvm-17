## 命令
```bash
clang -g outofbounds.c -o outofbounds -fsanitize=address
clang -g useafterfree.c -o useafterfree -fsanitize=address
clang -g memory.c -o memory -fsanitize=memory
clang -g thread.c -o thread -fsanitize=thread
# 运行可执行程序时关闭地址随机化
setarch --addr-no-randomize ./thread
```
