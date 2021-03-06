; RUN: llc -mtriple=aarch64-linux %s               -o - | \
; RUN:   FileCheck %s --check-prefix=ASM
; RUN: llc -mtriple=aarch64-linux %s -filetype=obj -o - |  \
; RUN:   llvm-readelf --notes - | FileCheck %s --check-prefix=OBJ

define dso_local i32 @f() {
entry:
  ret i32 0
}

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 8, !"branch-target-enforcement", i32 1}
!1 = !{i32 8, !"sign-return-address", i32 1}
!2 = !{i32 8, !"sign-return-address-all", i32 0}
!3 = !{i32 8, !"sign-return-address-with-bkey", i32 0}

; Both attribute present
; ASM:	    .word	3221225472
; ASM-NEXT:	.word	4
; ASM-NEXT:	.word	3

; OBJ: Properties: aarch64 feature: BTI, PAC
