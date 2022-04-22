# RUN: not llvm-mc -triple agc < %s 2>&1 | FileCheck %s

ca 010000   # CHECK: error: invalid operand for instruction
ca          # CHECK: error: too few operands for instruction
extend
ca r50      # CHECK: error: instruction prefixed with EXTEND is not an extracode

extend
dca 010000  # CHECK: error: invalid operand for instruction
extend
dca         # CHECK: error: too few operands for instruction
dca rd50    # CHECK: error: extracode instruction should be prefixed with EXTEND


cs 010000   # CHECK: error: invalid operand for instruction
cs          # CHECK: error: too few operands for instruction
extend
cs r50      # CHECK: error: instruction prefixed with EXTEND is not an extracode

extend
dcs 010000  # CHECK: error: invalid operand for instruction
extend
dcs         # CHECK: error: too few operands for instruction
dcs rd50    # CHECK: error: extracode instruction should be prefixed with EXTEND

ad 010000   # CHECK: error: invalid operand for instruction
ad          # CHECK: error: too few operands for instruction
extend
ad r50      # CHECK: error: instruction prefixed with EXTEND is not an extracode

extend
su 02000    # CHECK: error: invalid operand for instruction
extend
su          # CHECK: error: too few operands for instruction
su r50      # CHECK: error: extracode instruction should be prefixed with EXTEND

das 02000   # CHECK: error: invalid operand for instruction
das         # CHECK: error: too few operands for instruction
extend
das rd50    # CHECK: error: instruction prefixed with EXTEND is not an extracode

extend
msu 02000   # CHECK: error: invalid operand for instruction
extend
msu         # CHECK: error: too few operands for instruction
msu r50     # CHECK: error: extracode instruction should be prefixed with EXTEND

mask 010000 # CHECK: error: invalid operand for instruction
mask        # CHECK: error: too few operands for instruction
extend
mask r50    # CHECK: error: instruction prefixed with EXTEND is not an extracode

extend
mp 010000   # CHECK: error: invalid operand for instruction
extend
mp          # CHECK: error: too few operands for instruction
mp r50      # CHECK: error: extracode instruction should be prefixed with EXTEND

extend
dv 02000    # CHECK: error: invalid operand for instruction
extend
dv          # CHECK: error: too few operands for instruction
dv rd50     # CHECK: error: extracode instruction should be prefixed with EXTEND

incr 02000  # CHECK: error: invalid operand for instruction
incr        # CHECK: error: too few operands for instruction
extend
incr r50    # CHECK: error: instruction prefixed with EXTEND is not an extracode

extend
aug 02000   # CHECK: error: invalid operand for instruction
extend
aug         # CHECK: error: too few operands for instruction
aug r50     # CHECK: error: extracode instruction should be prefixed with EXTEND

ads 02000   # CHECK: error: invalid operand for instruction
ads         # CHECK: error: too few operands for instruction
extend
ads r50     # CHECK: error: instruction prefixed with EXTEND is not an extracode

extend
dim 02000   # CHECK: error: invalid operand for instruction
extend
dim         # CHECK: error: too few operands for instruction
dim r50     # CHECK: error: extracode instruction should be prefixed with EXTEND

ts 02000    # CHECK: error: invalid operand for instruction
ts          # CHECK: error: too few operands for instruction
extend
ts r50      # CHECK: error: instruction prefixed with EXTEND is not an extracode

xch 02000   # CHECK: error: invalid operand for instruction
xch         # CHECK: error: too few operands for instruction
extend
xch r50     # CHECK: error: instruction prefixed with EXTEND is not an extracode

lxch 02000  # CHECK: error: invalid operand for instruction
lxch        # CHECK: error: too few operands for instruction
extend
lxch r50    # CHECK: error: instruction prefixed with EXTEND is not an extracode

extend
qxch 02000  # CHECK: error: invalid operand for instruction
extend
qxch        # CHECK: error: too few operands for instruction
qxch r50    # CHECK: error: extracode instruction should be prefixed with EXTEND

dxch 02000  # CHECK: error: invalid operand for instruction
dxch        # CHECK: error: too few operands for instruction
extend
dxch rd50   # CHECK: error: instruction prefixed with EXTEND is not an extracode

tc 010000   # CHECK: error: address must be an immediate in the range [0, 4095] or a symbol
tc          # CHECK: error: too few operands for instruction
extend
tc 07777    # CHECK: error: instruction prefixed with EXTEND is not an extracode

tcf 010000  # CHECK: error: address must be an immediate in the range [0, 4095] or a symbol
tcf         # CHECK: error: too few operands for instruction
extend
tcf 07777   # CHECK: error: instruction prefixed with EXTEND is not an extracode

extend
bzf 010000  # CHECK: error: address must be an immediate in the range [0, 4095] or a symbol
extend
bzf         # CHECK: error: too few operands for instruction
bzf 07777   # CHECK: error: extracode instruction should be prefixed with EXTEND

extend
bzmf 010000 # CHECK: error: address must be an immediate in the range [0, 4095] or a symbol
extend
bzmf        # CHECK: error: too few operands for instruction
bzmf 07777  # CHECK: error: extracode instruction should be prefixed with EXTEND

extend
read 01000  # CHECK: error: IO channel address must be an immediate in the range [0, 511] or a symbol
extend
read        # CHECK: error: too few operands for instruction
read 0777   # CHECK: error: extracode instruction should be prefixed with EXTEND

extend
write 01000 # CHECK: error: IO channel address must be an immediate in the range [0, 511] or a symbol
extend
write       # CHECK: error: too few operands for instruction
write 0777  # CHECK: error: extracode instruction should be prefixed with EXTEND

extend
rand 01000  # CHECK: error: IO channel address must be an immediate in the range [0, 511] or a symbol
extend
rand        # CHECK: error: too few operands for instruction
rand 0777   # CHECK: error: extracode instruction should be prefixed with EXTEND

extend
wand 01000  # CHECK: error: IO channel address must be an immediate in the range [0, 511] or a symbol
extend
wand        # CHECK: error: too few operands for instruction
wand 0777   # CHECK: error: extracode instruction should be prefixed with EXTEND

extend
ror 01000   # CHECK: error: IO channel address must be an immediate in the range [0, 511] or a symbol
extend
ror         # CHECK: error: too few operands for instruction
ror 0777    # CHECK: error: extracode instruction should be prefixed with EXTEND

extend
wor 01000   # CHECK: error: IO channel address must be an immediate in the range [0, 511] or a symbol
extend
wor         # CHECK: error: too few operands for instruction
wor 0777    # CHECK: error: extracode instruction should be prefixed with EXTEND

extend
rxor 01000  # CHECK: error: IO channel address must be an immediate in the range [0, 511] or a symbol
extend
rxor        # CHECK: error: too few operands for instruction
rxor 0777   # CHECK: error: extracode instruction should be prefixed with EXTEND

fail r50    # CHECK: error: unrecognized instruction mnemonic
