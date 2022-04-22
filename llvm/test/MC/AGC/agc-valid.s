# RUN: llvm-mc -triple agc -show-encoding < %s \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc -triple agc -filetype=obj < %s | llvm-objdump -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-INST %s

# CHECK-INST: index
# CHECK: encoding: [0xa0,0x64]
index r50

# CHECK-INST: ca r50
# CHECK: encoding: [0x60,0x64]
ca r50
# CHECK-INST: ca
# CHECK: encoding: [0x60,0x01]
ca k
# CHECK-INST: ca 4095
# CHECK: encoding: [0x7f,0xff]
ca 07777

# CHECK-INST: extend
# CHECK-INST: dca rd50
# CHECK: encoding: [0x00,0x0d,0x60,0x64]
extend
dca rd50
# CHECK-INST: extend
# CHECK-INST: dca
# CHECK: encoding: [0x00,0x0d,0x60,0x01]
extend
dca k
# CHECK-INST: extend
# CHECK-INST: dca 4094
# CHECK: encoding: [0x00,0x0d,0x7f,0xfc]
extend
dca 07776

# CHECK-INST: cs r50
# CHECK: encoding: [0x80,0x65]
cs r50
# CHECK-INST: cs
# CHECK: encoding: [0x80,0x00]
cs k
# CHECK-INST: cs 4095
# CHECK: encoding: [0x9f,0xfe]
cs 07777

# CHECK-INST: extend
# CHECK-INST: dcs rd50
# CHECK: encoding: [0x00,0x0d,0x80,0x65]
extend
dcs rd50
# CHECK-INST: extend
# CHECK-INST: dcs
# CHECK: encoding: [0x00,0x0d,0x80,0x00]
extend
dcs k
# CHECK-INST: extend
# CHECK-INST: dcs 4094
# CHECK: encoding: [0x00,0x0d,0x9f,0xfd]
extend
dcs 07776

# CHECK-INST: ad r50
# CHECK: encoding: [0xc0,0x64]
ad r50
# CHECK-INST: ad
# CHECK: encoding: [0xc0,0x01]
ad k
# CHECK-INST: ad 4095
# CHECK: encoding: [0xdf,0xff]
ad 07777

# CHECK-INST: extend
# CHECK-INST: su r50
# CHECK: encoding: [0x00,0x0d,0xc0,0x64]
extend
su r50
# CHECK-INST: extend
# CHECK-INST: su
# CHECK: encoding: [0x00,0x0d,0xc0,0x01]
extend
su k
# CHECK-INST: extend
# CHECK-INST: su 1023
# CHECK: encoding: [0x00,0x0d,0xc7,0xff]
extend
su 01777

# CHECK-INST: das rd50
# CHECK: encoding: [0x40,0x65]
das rd50
# CHECK-INST: das
# CHECK: encoding: [0x40,0x00]
das k
# CHECK-INST: das 1022
# CHECK: encoding: [0x47,0xfd]
das 01776

# CHECK-INST: extend
# CHECK-INST: msu r50
# CHECK: encoding: [0x00,0x0d,0x40,0x65]
extend
msu r50
# CHECK-INST: extend
# CHECK-INST: msu
# CHECK: encoding: [0x00,0x0d,0x40,0x00]
extend
msu k
# CHECK-INST: extend
# CHECK-INST: msu 1023
# CHECK: encoding: [0x00,0x0d,0x47,0xfe]
extend
msu 01777

# CHECK-INST: mask r50
# CHECK: encoding: [0xe0,0x65]
mask r50
# CHECK-INST: mask
# CHECK: encoding: [0xe0,0x00]
mask k
# CHECK-INST: mask 4095
# CHECK: encoding: [0xff,0xfe]
mask 07777

# CHECK-INST: extend
# CHECK-INST: mp r50
# CHECK: encoding: [0x00,0x0d,0xe0,0x65]
extend
mp r50
# CHECK-INST: extend
# CHECK-INST: mp
# CHECK: encoding: [0x00,0x0d,0xe0,0x00]
extend
mp k
# CHECK-INST: extend
# CHECK-INST: mp 4095
# CHECK: encoding: [0x00,0x0d,0xff,0xfe]
extend
mp 07777

# CHECK-INST: extend
# CHECK-INST: dv rd50
# CHECK: encoding: [0x00,0x0d,0x20,0x65]
extend
dv rd50
# CHECK-INST: extend
# CHECK-INST: dv
# CHECK: encoding: [0x00,0x0d,0x20,0x00]
extend
dv k
# CHECK-INST: extend
# CHECK-INST: dv 1022
# CHECK: encoding: [0x00,0x0d,0x27,0xfd]
extend
dv 01776

# CHECK-INST: incr r50
# CHECK: encoding: [0x50,0x64]
incr r50
# CHECK-INST: incr
# CHECK: encoding: [0x50,0x01]
incr k
# CHECK-INST: incr 1023
# CHECK: encoding: [0x57,0xff]
incr 01777

# CHECK-INST: extend
# CHECK-INST: aug r50
# CHECK: encoding: [0x00,0x0d,0x50,0x64]
extend
aug r50
# CHECK-INST: extend
# CHECK-INST: aug
# CHECK: encoding: [0x00,0x0d,0x50,0x01]
extend
aug k
# CHECK-INST: extend
# CHECK-INST: aug 1023
# CHECK: encoding: [0x00,0x0d,0x57,0xff]
extend
aug 01777

# CHECK-INST: ads r50
# CHECK: encoding: [0x58,0x65]
ads r50
# CHECK-INST: ads
# CHECK: encoding: [0x58,0x00]
ads k
# CHECK-INST: ads 1023
# CHECK: encoding: [0x5f,0xfe]
ads 01777

# CHECK-INST: extend
# CHECK-INST: dim r50
# CHECK: encoding: [0x00,0x0d,0x58,0x65]
extend
dim r50
# CHECK-INST: extend
# CHECK-INST: dim
# CHECK: encoding: [0x00,0x0d,0x58,0x00]
extend
dim k
# CHECK-INST: extend
# CHECK-INST: dim 1023
# CHECK: encoding: [0x00,0x0d,0x5f,0xfe]
extend
dim 01777

# CHECK-INST: ts r50
# CHECK: encoding: [0xb0,0x65]
ts r50
# CHECK-INST: ts
# CHECK: encoding: [0xb0,0x00]
ts k
# CHECK-INST: ts 1023
# CHECK: encoding: [0xb7,0xfe]
ts 01777

# CHECK-INST: xch r50
# CHECK: encoding: [0xb8,0x64]
xch r50
# CHECK-INST: xch
# CHECK: encoding: [0xb8,0x01]
xch k
# CHECK-INST: xch 1023
# CHECK: encoding: [0xbf,0xff]
xch 01777

# CHECK-INST: lxch r50
# CHECK: encoding: [0x48,0x64]
lxch r50
# CHECK-INST: lxch
# CHECK: encoding: [0x48,0x01]
lxch k
# CHECK-INST: lxch 1023
# CHECK: encoding: [0x4f,0xff]
lxch 01777

# CHECK-INST: extend
# CHECK-INST: qxch r50
# CHECK: encoding: [0x00,0x0d,0x48,0x64]
extend
qxch r50
# CHECK-INST: extend
# CHECK-INST: qxch
# CHECK: encoding: [0x00,0x0d,0x48,0x01]
extend
qxch k
# CHECK-INST: extend
# CHECK-INST: qxch 1023
# CHECK: encoding: [0x00,0x0d,0x4f,0xff]
extend
qxch 01777

# CHECK-INST: dxch rd50
# CHECK: encoding: [0xa8,0x65]
dxch rd50
# CHECK-INST: dxch
# CHECK: encoding: [0xa8,0x00]
dxch k
# CHECK-INST: dxch 1022
# CHECK: encoding: [0xaf,0xfd]
dxch 01776

# CHECK-INST: tc 0
# CHECK: encoding: [0x00,0x01]
tc   0
# CHECK-INST: tc
# CHECK: encoding: [0x00,0x01]
tc   .lbl
# CHECK-INST: tc 4095
# CHECK: encoding: [0x1f,0xff]
tc 07777

# CHECK-INST: tcf 0
# CHECK: encoding: [0x20,0x00]
tcf  0
# CHECK-INST: tcf
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
# CHECK-INST: bzf
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
# CHECK-INST: bzmf
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
# CHECK-INST: read
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
# CHECK-INST: write
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
# CHECK-INST: rand
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
# CHECK-INST: wand
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
# CHECK-INST: ror
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
# CHECK-INST: wor
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
# CHECK-INST: rxor
# CHECK: encoding: [0x00,0x0d,0x18,0x01]
extend
rxor kc
# CHECK-INST: extend
# CHECK-INST: rxor 511
# CHECK: encoding: [0x00,0x0d,0x1b,0xfe]
extend
rxor 0777
