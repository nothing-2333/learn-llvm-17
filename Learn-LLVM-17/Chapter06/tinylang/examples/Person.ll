; ModuleID = './examples/Person.mod'
source_filename = "./examples/Person.mod"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%Person = type { i64, i64 }

define void @_t6Person3Set(ptr nocapture dereferenceable(16) %p) !dbg !2 {
entry:
  call void @llvm.dbg.value(metadata ptr %p, metadata !8, metadata !DIExpression()), !dbg !9
  %0 = getelementptr inbounds %Person, ptr %p, i32 0, i32 1
  store i64 18, ptr %0, align 8, !tbaa !10
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.value(metadata, metadata, metadata) #0

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}

!0 = distinct !DICompileUnit(language: DW_LANG_Modula2, file: !1, producer: "tinylang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "Person.mod", directory: "/home/nothing/learn-llvm-17/Learn-LLVM-17/Chapter06/tinylang/./examples")
!2 = distinct !DISubprogram(name: "Set", linkageName: "_t6Person3Set", scope: !1, file: !1, line: 9, type: !3, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!3 = !DISubroutineType(types: !4)
!4 = !{!5, !6}
!5 = !DIBasicType(tag: DW_TAG_unspecified_type, name: "void")
!6 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: null, size: 1024, align: 8)
!7 = !{}
!8 = !DILocalVariable(name: "p", arg: 1, scope: !2, file: !1, line: 9)
!9 = !DILocation(line: 9, column: 19, scope: !2)
!10 = !{!"=\DD\D3`\05\00\00\00\A7c\83\22\F6O\F8\C1", !11, i64 0, !11, i64 8}
!11 = !{!"INTEGER", !12, i64 0}
!12 = !{!"Simple tinylang TBAA"}
