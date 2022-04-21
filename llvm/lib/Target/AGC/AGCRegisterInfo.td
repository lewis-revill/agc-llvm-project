//===-- AGCRegisterInfo.td - AGC Register defs -------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Declarations that describe the AGC register files
//
//===----------------------------------------------------------------------===//

let Namespace = "AGC" in {
// AGCReg - The base register for all internal AGC registers.
class AGCReg<bits<12> enc, string n> : Register<n> {
  let HWEncoding{11-0} = enc;
  let RegAltNameIndices = [NoRegAltName];
}

// AGCRegWithSubRegs - The base register for AGC registers composed of
// sub-registers.
class AGCRegWithSubRegs<bits<12> enc, string n, list<Register> subregs>
    : RegisterWithSubRegs<n, subregs> {
  let HWEncoding{11-0} = enc;
  let RegAltNameIndices = [NoRegAltName];
}

// Define upper/lower subreg indices.
def subreg_upper : SubRegIndex<16>;
def subreg_lower : SubRegIndex<16>;
} // Namespace = "AGC"

// Note that all values in registers are treated as 15 bit ones complement,
// unless documented otherwise, and the instruction using the register knows
// about how to handle the register.

//===----------------------------------------------------------------------===//
// AGC registers
//===----------------------------------------------------------------------===//

// Accumulator register. Used to store the result of operations done by the CPU.
// Consists of 16 bits due to the addition of an overflow/underflow bit.
def R0 : AGCReg<0, "a">, DwarfRegNum<[0]>;

// Lower accumulator register. Used to store the lower half of the result of
// operations done by the CPU when these operations are 'double precision'. Only
// 15 bits used in contrast to the standard accumulator.
def R1 : AGCReg<1, "l">, DwarfRegNum<[1]>;

// Return address register. Used to store the address that a procedure should
// return to. The AGC has no physical stack and so this register is the only
// available option for calls, implying recursion is not well supported.
// Consists of 16 bits due to the addition of an overflow/underflow bit.
def R2 : AGCReg<2, "q">, DwarfRegNum<[2]>;

// Erasable memory bank select register. Used to switch the contents present in
// the memory locations 0x300 to 0x3FF between the 8 erasable memory banks. Only
// 3 bits of this register are needed and are arranged as follows:
//
// X000 0EEE 0000 0000
//
// The both banks select register aliases these bits.
def R3 : AGCReg<3, "eb">, DwarfRegNum<[3]>;

// Fixed memory bank select register. Used to switch the contents present in the
// memory locations 0x400 to 0x7FF between the 36 fixed memory banks. Only 5
// bits of this register are needed and are arranged as follows:
//
// XFFF FF00 0000 0000
//
// The both banks select register aliases these bits
def R4 : AGCReg<4, "fb">, DwarfRegNum<[4]>;

// Program counter. Used to store 12 bits of the address of the next
// instruction. The full address depends on the contents of the memory bank
// select registers to access the full range. The bits are arranged as follows:
//
// X000 AAAA AAAA AAAA
def R5 : AGCReg<5, "z">, DwarfRegNum<[5]>;

// Both banks select register. Aliases both the erasable memory bank select and
// the fixed memory bank select registers. The bits are arranged as follows:
//
// XFFF FF00 0000 0EEE
def R6 : AGCReg<6, "bb">, DwarfRegNum<[6]>;

// Fixed zeroes register. This register has a fixed contents of all zeroes.
def R7 : AGCReg<7, "zero">, DwarfRegNum<[7]>;

// Location reserved for storing the state of the accumulator register under an
// interrupt. It is not automatically written, so the interrupt routine must do
// so manually.
def R8  : AGCReg<8, "arupt">, DwarfRegNum<[8]>;

// Location reserved for storing the state of the lower accumulator register
// under an interrupt. It is not automatically written, so the interrupt routine
// must do so manually.
def R9  : AGCReg<9, "lrupt">, DwarfRegNum<[9]>;

// Location reserved for storing the state of the return address register under
// an interrupt. It is not automatically written, so the interrupt routine must
// do so manually.
def R10 : AGCReg<10,"qrupt">, DwarfRegNum<[10]>;

// Copies of the TIME2 and TIME1 registers respectively.
def R11 : AGCReg<11,"samptime2">, DwarfRegNum<[11]>;
def R12 : AGCReg<12,"samptime1">, DwarfRegNum<[12]>;

// Interrupt return address register. Used to store the address that an
// interrupt routine should return to.
def R13 : AGCReg<13,"zrupt">, DwarfRegNum<[13]>;

// Location reserved for storing the state of the both banks select register
// under an interrupt. It is not automatically written, so the interrupt routine
// must do so manually.
def R14 : AGCReg<14,"bbrupt">, DwarfRegNum<[14]>;

// Interrupt return instruction register. Used to store the actual instruction
// at the interrupt return address. Used as the first instruction executed after
// the interrupt routine returns - used for the purpose of storing the modified
// version of an instruction preceeded by INDEX.
def R15 : AGCReg<15,"brupt">, DwarfRegNum<[15]>;

// Cycle right register. Upon writing to this register (including write-backs),
// its 15-bit contents are cycled right through one bit.
def R16 : AGCReg<16,"cyr">, DwarfRegNum<[16]>;

// Shift right register. Upon writing to this register (including write-backs),
// its 15-bit contents are shifted right one bit.
def R17 : AGCReg<17,"sr">, DwarfRegNum<[17]>;

// Cycle left register. Upon writing to this register (including write-backs),
// its 15-bit contents are cycled left through one bit.
def R18 : AGCReg<18,"cyl">, DwarfRegNum<[18]>;

// Unused editing register which is intended to behave similar to the ones
// above.
def R19 : AGCReg<19,"edop">, DwarfRegNum<[19]>;

// Counters which are automatically incremented. TIME1 is incremented every 10ms
// and upon overflow of the 15-bit ones complement count, TIME2 is automatically
// incremented. Thus TIME2 and TIME1 together form a 28-bit counter when used as
// a combined register.
def R20 : AGCReg<20,"time2">, DwarfRegNum<[20]>;
def R21 : AGCReg<21,"time1">, DwarfRegNum<[21]>;

// Counters which are also automatically incremented every 10ms, but with
// various phases relative to TIME1. TIME3 is in phase, TIME4 is 7.5ms delayed,
// TIME5 is 5ms delayed, and TIME6 is 2.5ms delayed.
def R22 : AGCReg<22,"time3">, DwarfRegNum<[22]>;
def R23 : AGCReg<23,"time4">, DwarfRegNum<[23]>;
def R24 : AGCReg<24,"time5">, DwarfRegNum<[24]>;
def R25 : AGCReg<25,"time6">, DwarfRegNum<[25]>;

// The following registers are all for interactions with the Apollo hardware for
// control and communication. Their specific behaviour is not really relevant to
// the compiler other than the fact that they should never be allocated and only
// used explicitly via builtins.
def R26 : AGCReg<26,"cdux">, DwarfRegNum<[26]>;
def R27 : AGCReg<27,"cduy">, DwarfRegNum<[27]>;
def R28 : AGCReg<28,"cduz">, DwarfRegNum<[28]>;
def R29 : AGCReg<29,"opty">, DwarfRegNum<[29]>;
def R30 : AGCReg<30,"optz">, DwarfRegNum<[30]>;
def R31 : AGCReg<31,"pipax">, DwarfRegNum<[31]>;
def R32 : AGCReg<32,"pipay">, DwarfRegNum<[32]>;
def R33 : AGCReg<33,"pipaz">, DwarfRegNum<[33]>;
def R34 : AGCReg<34,"rhcp">, DwarfRegNum<[34]>;
def R35 : AGCReg<35,"rhcy">, DwarfRegNum<[35]>;
def R36 : AGCReg<36,"rhcr">, DwarfRegNum<[36]>;
def R37 : AGCReg<37,"inlink">, DwarfRegNum<[37]>;
def R38 : AGCReg<38,"rnrad">, DwarfRegNum<[38]>;
def R39 : AGCReg<39,"gyrocmd">, DwarfRegNum<[39]>;
def R40 : AGCReg<40,"cduxcmd">, DwarfRegNum<[40]>;
def R41 : AGCReg<41,"cduycmd">, DwarfRegNum<[41]>;
def R42 : AGCReg<42,"cduzcmd">, DwarfRegNum<[42]>;
def R43 : AGCReg<43,"optycmd">, DwarfRegNum<[43]>;
def R44 : AGCReg<44,"optzcmd">, DwarfRegNum<[44]>;
def R45 : AGCReg<45,"thrust">, DwarfRegNum<[45]>;
def R46 : AGCReg<46,"lemonm">, DwarfRegNum<[46]>;
def R47 : AGCReg<47,"outlink">, DwarfRegNum<[47]>;
def R48 : AGCReg<48,"altm">, DwarfRegNum<[48]>;

// Stack pointer. This register has no special implementation so it is just one
// of a number of general purpose registers we have added to those defined as
// special purpose registers.
def R49 : AGCReg<49, "sp">, DwarfRegNum<[49]>;

// General purpose registers. These registers are actually no different from any
// erasable memory location, but as part of our memory model to help compilation
// we define these as extra registers that can be allocated, whereas the rest of
// memory is treated more like actual memory.
def R50 : AGCReg<50, "r50">, DwarfRegNum<[50]>;
def R51 : AGCReg<51, "r51">, DwarfRegNum<[51]>;
def R52 : AGCReg<52, "r52">, DwarfRegNum<[52]>;
def R53 : AGCReg<53, "r53">, DwarfRegNum<[53]>;
def R54 : AGCReg<54, "r54">, DwarfRegNum<[54]>;
def R55 : AGCReg<55, "r55">, DwarfRegNum<[55]>;
def R56 : AGCReg<56, "r56">, DwarfRegNum<[56]>;
def R57 : AGCReg<57, "r57">, DwarfRegNum<[57]>;
def R58 : AGCReg<58, "r58">, DwarfRegNum<[58]>;
def R59 : AGCReg<59, "r59">, DwarfRegNum<[59]>;
def R60 : AGCReg<60, "r60">, DwarfRegNum<[60]>;
def R61 : AGCReg<61, "r61">, DwarfRegNum<[61]>;
def R62 : AGCReg<62, "r62">, DwarfRegNum<[62]>;
def R63 : AGCReg<63, "r63">, DwarfRegNum<[63]>;



// All combinations of double registers. Some are documented as special pairs
// but most pairs are arbitrary due to their order.
let SubRegIndices = [subreg_upper, subreg_lower] in {

// Double accumulator register. Composed of both the upper and lower accumulator
// registers. The effective value in this register is the concatenation of the
// lower 14 bits of both registers, with the sign bits in each register set to
// match each other. The overflow bit in the upper accumulator is set
// accordingly.
def RD0 : AGCRegWithSubRegs<0, "al", [R0, R1]>, DwarfRegNum<[64]>;

foreach Index = 1-10 in {
  let SubRegIndices = [subreg_upper, subreg_lower] in
  def RD#Index : AGCRegWithSubRegs<Index, "rd"#Index,
                                   [!cast<AGCReg>("R"#Index),
                                    !cast<AGCReg>("R"#!add(Index, 1))]>,
                 DwarfRegNum<[!add(Index, 64)]>;
}

// Double SAMPTIME register. Composed of SAMPTIME2 and SAMPTIME1.
def RD11 : AGCRegWithSubRegs<11, "samptime", [R11, R12]>,
           DwarfRegNum<[75]>;

foreach Index = 12-19 in {
  let SubRegIndices = [subreg_upper, subreg_lower] in
  def RD#Index : AGCRegWithSubRegs<Index, "rd"#Index,
                                   [!cast<AGCReg>("R"#Index),
                                    !cast<AGCReg>("R"#!add(Index, 1))]>,
                 DwarfRegNum<[!add(Index, 64)]>;
}

// Double TIME register. Composed of TIME2 and TIME1.
def RD20 : AGCRegWithSubRegs<20, "time", [R20, R21]>, DwarfRegNum<[84]>;

foreach Index = 21-62 in {
  let SubRegIndices = [subreg_upper, subreg_lower] in
  def RD#Index : AGCRegWithSubRegs<Index, "rd"#Index,
                                   [!cast<AGCReg>("R"#Index),
                                    !cast<AGCReg>("R"#!add(Index, 1))]>,
                 DwarfRegNum<[!add(Index, 64)]>;
}

} // SubRegIndices = [subreg_upper, subreg_lower]

//===----------------------------------------------------------------------===//
// AGC register classes
//===----------------------------------------------------------------------===//

// Define the accumulator as its own register class.
def A : RegisterClass<"AGC", [i16], 16, (add R0)>;

// Define the lower accumulator as its own register class.
def L : RegisterClass<"AGC", [i16], 16, (add R1)>;

// Define the upper/lower accumulator pair as its own register class.
def AL : RegisterClass<"AGC", [i32], 16, (add RD0)>;

// Define the return address register as its own register class.
def Q : RegisterClass<"AGC", [i16], 16, (add R2)>;

// Define the stack pointer as its own register class.
def SP : RegisterClass<"AGC", [i16], 16, (add R49)>;

// Define the full set of 'general purpose' registers in a class.
def GPR : RegisterClass<"AGC", [i16], 16, (add (sequence "R%u", 50, 63),
                                               (sequence "R%u", 0, 49))>;

// Define the full set of 'general purpose' double registers in a class.
def GPRD : RegisterClass<"AGC", [i32], 16, (add (sequence "RD%u", 50, 62),
                                                (sequence "RD%u", 0, 49))>;
