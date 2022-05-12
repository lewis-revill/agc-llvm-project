# RUN: llvm-mc -triple agc -show-encoding < %s \
# RUN:     | FileCheck -check-prefixes=CHECK %s
# RUN: llvm-mc -triple agc -filetype=obj < %s | llvm-objdump -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-REL %s

# CHECK: ca %cpi(1)
# CHECK: encoding: [0b011AAAAA,0bAAAAAAA1]
# CHECK: fixup A - offset: 0, value: %cpi(1), kind: fixup_agc_cpi12
# CHECK-REL: R_AGC_CPI12 *ABS*+0x1
ca %cpi(1)

# CHECK: ca %banks(sym)
# CHECK: encoding: [0b011AAAAA,0bAAAAAAA1]
# CHECK: fixup A - offset: 0, value: %banks(sym), kind: fixup_agc_banks12
# CHECK-REL: R_AGC_BANKS12 sym
ca %banks(sym)

# CHECK: tc %lo12(sym)
# CHECK: encoding: [0b000AAAAA,0bAAAAAAA1]
# CHECK: fixup A - offset: 0, value: %lo12(sym), kind: fixup_agc_lo12
# CHECK-REL: R_AGC_LO12 sym
tc %lo12(sym)
