; ModuleID = 'examples/Gcd.mod'
source_filename = "examples/Gcd.mod"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i64 @_t3Gcd3GCD(i64 %a, i64 %b) {
entry:
  %0 = icmp eq i64 %b, 0
  br i1 %0, label %if.body, label %after.if

if.body:                                          ; preds = %entry
  ret i64 %a

after.if:                                         ; preds = %entry
  br label %while.cond

while.cond:                                       ; preds = %while.body, %after.if
  %1 = phi i64 [ %2, %while.body ], [ %a, %after.if ]
  %2 = phi i64 [ %4, %while.body ], [ %b, %after.if ]
  %3 = icmp ne i64 %2, 0
  br i1 %3, label %while.body, label %after.while

while.body:                                       ; preds = %while.cond
  %4 = srem i64 %1, %2
  br label %while.cond

after.while:                                      ; preds = %while.cond
  ret i64 %1
}
