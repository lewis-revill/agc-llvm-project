# RUN: llvm-mc -triple agc -show-encoding < %s \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s

# CHECK-INST: index
# CHECK: encoding: [0xa0,0x64]
index r50

# CHECK-INST: ca r50
# CHECK: encoding: [0x60,0x64]
ca r50

# CHECK-INST: extend
# CHECK-INST: dca rd50
# CHECK: encoding: [0x00,0x0d,0x60,0x64]
extend
dca rd50

# CHECK-INST: cs r50
# CHECK: encoding: [0x80,0x65]
cs r50

# CHECK-INST: extend
# CHECK-INST: dcs rd50
# CHECK: encoding: [0x00,0x0d,0x80,0x65]
extend
dcs rd50

# CHECK-INST: ad r50
# CHECK: encoding: [0xc0,0x64]
ad r50

# CHECK-INST: extend
# CHECK-INST: su r50
# CHECK: encoding: [0x00,0x0d,0xc0,0x64]
extend
su r50

# CHECK-INST: das rd50
# CHECK: encoding: [0x40,0x65]
das rd50

# CHECK-INST: extend
# CHECK-INST: msu r50
# CHECK: encoding: [0x00,0x0d,0x40,0x65]
extend
msu r50

# CHECK-INST: mask r50
# CHECK: encoding: [0xe0,0x65]
mask r50

# CHECK-INST: extend
# CHECK-INST: mp r50
# CHECK: encoding: [0x00,0x0d,0xe0,0x65]
extend
mp r50

# CHECK-INST: extend
# CHECK-INST: dv rd50
# CHECK: encoding: [0x00,0x0d,0x20,0x65]
extend
dv rd50

# CHECK-INST: incr r50
# CHECK: encoding: [0x50,0x64]
incr r50

# CHECK-INST: extend
# CHECK-INST: aug r50
# CHECK: encoding: [0x00,0x0d,0x50,0x64]
extend
aug r50

# CHECK-INST: ads r50
# CHECK: encoding: [0x58,0x65]
ads r50

# CHECK-INST: extend
# CHECK-INST: dim r50
# CHECK: encoding: [0x00,0x0d,0x58,0x65]
extend
dim r50

# CHECK-INST: ts r50
# CHECK: encoding: [0xb0,0x65]
ts r50

# CHECK-INST: xch r50
# CHECK: encoding: [0xb8,0x64]
xch r50

# CHECK-INST: lxch r50
# CHECK: encoding: [0x48,0x64]
lxch r50

# CHECK-INST: extend
# CHECK-INST: qxch r50
# CHECK: encoding: [0x00,0x0d,0x48,0x64]
extend
qxch r50

# CHECK-INST: dxch rd50
# CHECK: encoding: [0xa8,0x65]
dxch rd50

# CHECK-INST: tc 0
# CHECK: encoding: [0x00,0x01]
tc   0
# CHECK-INST: tc .lbl
# CHECK: encoding: [0x00,0x01]
tc   .lbl
# CHECK-INST: tc 4095
# CHECK: encoding: [0x1f,0xff]
tc 07777

# CHECK-INST: tcf 0
# CHECK: encoding: [0x20,0x00]
tcf  0
# CHECK-INST: tcf .lbl
# CHECK: encoding: [0x20,0x00]
tcf  .lbl
# CHECK-INST: tcf 4095
# CHECK: encoding: [0x3f,0xfe]
tcf  07777

# CHECK-INST: extend
# CHECK-INST: bzf 1024
# CHECK: encoding: [0x00,0x0d,0x28,0x01]
extend
bzf  02000
# CHECK-INST: extend
# CHECK-INST: bzf .lbl
# CHECK: encoding: [0x00,0x0d,0x28,0x01]
extend
bzf  .lbl
# CHECK-INST: extend
# CHECK-INST: bzf 4095
# CHECK: encoding: [0x00,0x0d,0x3f,0xfe]
extend
bzf  07777

# CHECK-INST: extend
# CHECK-INST: bzmf 1024
# CHECK: encoding: [0x00,0x0d,0xc8,0x00]
extend
bzmf 02000
# CHECK-INST: extend
# CHECK-INST: bzmf .lbl
# CHECK: encoding: [0x00,0x0d,0xc8,0x00]
extend
bzmf .lbl
# CHECK-INST: extend
# CHECK-INST: bzmf 4095
# CHECK: encoding: [0x00,0x0d,0xdf,0xff]
extend
bzmf 07777

# CHECK-INST: extend
# CHECK-INST: read 0
# CHECK: encoding: [0x00,0x0d,0x00,0x01]
extend
read 0
# CHECK-INST: extend
# CHECK-INST: read kc
# CHECK: encoding: [0x00,0x0d,0x00,0x01]
extend
read kc
# CHECK-INST: extend
# CHECK-INST: read 511
# CHECK: encoding: [0x00,0x0d,0x03,0xfe]
extend
read 0777

# CHECK-INST: extend
# CHECK-INST: write 0
# CHECK: encoding: [0x00,0x0d,0x04,0x00]
extend
write 0
# CHECK-INST: extend
# CHECK-INST: write kc
# CHECK: encoding: [0x00,0x0d,0x04,0x00]
extend
write kc
# CHECK-INST: extend
# CHECK-INST: write 511
# CHECK: encoding: [0x00,0x0d,0x07,0xff]
extend
write 0777

# CHECK-INST: extend
# CHECK-INST: rand 0
# CHECK: encoding: [0x00,0x0d,0x08,0x00]
extend
rand 0
# CHECK-INST: extend
# CHECK-INST: rand kc
# CHECK: encoding: [0x00,0x0d,0x08,0x00]
extend
rand kc
# CHECK-INST: extend
# CHECK-INST: rand 511
# CHECK: encoding: [0x00,0x0d,0x0b,0xff]
extend
rand 0777

# CHECK-INST: extend
# CHECK-INST: wand 0
# CHECK: encoding: [0x00,0x0d,0x0c,0x01]
extend
wand 0
# CHECK-INST: extend
# CHECK-INST: wand kc
# CHECK: encoding: [0x00,0x0d,0x0c,0x01]
extend
wand kc
# CHECK-INST: extend
# CHECK-INST: wand 511
# CHECK: encoding: [0x00,0x0d,0x0f,0xfe]
extend
wand 0777

# CHECK-INST: extend
# CHECK-INST: ror 0
# CHECK: encoding: [0x00,0x0d,0x10,0x00]
extend
ror  0
# CHECK-INST: extend
# CHECK-INST: ror kc
# CHECK: encoding: [0x00,0x0d,0x10,0x00]
extend
ror  kc
# CHECK-INST: extend
# CHECK-INST: ror 511
# CHECK: encoding: [0x00,0x0d,0x13,0xff]
extend
ror  0777

# CHECK-INST: extend
# CHECK-INST: wor 0
# CHECK: encoding: [0x00,0x0d,0x14,0x01]
extend
wor  0
# CHECK-INST: extend
# CHECK-INST: wor kc
# CHECK: encoding: [0x00,0x0d,0x14,0x01]
extend
wor  kc
# CHECK-INST: extend
# CHECK-INST: wor 511
# CHECK: encoding: [0x00,0x0d,0x17,0xfe]
extend
wor  0777

# CHECK-INST: extend
# CHECK-INST: rxor 0
# CHECK: encoding: [0x00,0x0d,0x18,0x01]
extend
rxor 0
# CHECK-INST: extend
# CHECK-INST: rxor kc
# CHECK: encoding: [0x00,0x0d,0x18,0x01]
extend
rxor kc
# CHECK-INST: extend
# CHECK-INST: rxor 511
# CHECK: encoding: [0x00,0x0d,0x1b,0xfe]
extend
rxor 0777
