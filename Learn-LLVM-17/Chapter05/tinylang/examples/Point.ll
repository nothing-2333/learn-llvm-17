; ModuleID = 'examples/Point.mod'
source_filename = "examples/Point.mod"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%Point = type { i64, i64 }

@_t5Point1p = private global %Point

define void @_t5Point7AssignX(i64 %a) {
entry:
  store i64 %a, ptr @_t5Point1p, align 8
  ret void
}
