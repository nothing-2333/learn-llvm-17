; ModuleID = './examples/Person.mod'
source_filename = "./examples/Person.mod"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%Person = type { i64, i64 }

define void @_t6Person3Set(ptr nocapture dereferenceable(16) %p) {
entry:
  %0 = getelementptr inbounds %Person, ptr %p, i32 0, i32 1
  store i64 18, ptr %0, align 8, !tbaa !0
  ret void
}

!0 = !{!"m\83\826\06\00\00\00\00\A2\8Ef\7F\C9:n", !1, i64 0, !1, i64 8}
!1 = !{!"INTEGER", !2, i64 0}
!2 = !{!"Simple tinylang TBAA"}
