/* libs/pixelflinger/t32cb16_neon.S
 *
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

    .text
    .align

    .global scanline_t32cb16_neon

// r0:  dst ptr
// r1:  src ptr
// r2:  count

scanline_t32cb16_neon:
    cmp         r2, #7
    bhi         count_great_than_8

    // handle count < 8
    mov         r3, #0
    vmov.u8     d31, #1<<7
    mov         r3, r0

    tst         r2, #4
    beq         14f
    vld1.16     {d25}, [r0]!
    vld1.32     {q1}, [r1]!

14:
    tst         r2, #2
    beq         12f
    vld1.32     {d24[1]}, [r0]!
    vld1.32     {d1}, [r1]!

12:
    tst         r2, #1
    beq         11f
    vld1.16     {d24[1]}, [r0]!
    vld1.32     {d0[1]}, [r1]!

11:
    // unzip achieve the same as a vld4 operation
    vuzpq.u16   q0, q1
    vuzp.u8     d0, d1
    vuzp.u8     d2, d3
    // expand 0565 q12 to 8888 {d4-d7}
    vmovn.u16   d4, q12
    vshr.u16    q11, q12, #5
    vshr.u16    q10, q12, #6+5
    vmovn.u16   d5, q11
    vmovn.u16   d6, q10
    vshl.u8     d4, d4, #3
    vshl.u8     d5, d5, #2
    vshl.u8     d6, d6, #3

    vmovl.u8    q14, d31
    vmovl.u8    q13, d31
    vmovl.u8    q12, d31

    // duplicate in 4/2/1 & 8pix vsns
    vmvn.8      d30, d3
    vmlal.u8    q14, d30, d6
    vmlal.u8    q13, d30, d5
    vmlal.u8    q12, d30, d4
    vshr.u16    q8, q14, #5
    vshr.u16    q9, q13, #6
    vaddhn.u16  d6, q14, q8
    vshr.u16    q8, q12, #5
    vaddhn.u16  d5, q13, q9
    vqadd.u8    d6, d6, d0       // moved up
    vaddhn.u16  d4, q12, q8
    // intentionally, don't calculate alpha result in d4-d6

    vqadd.u8    d5, d5, d1
    vqadd.u8    d4, d4, d2

    // pack 8888 {d4-d6} to 0565 q10
    vshll.u8    q10, d6, #8
    vshll.u8    q3, d5, #8
    vshll.u8    q2, d4, #8
    vsri.u16    q10, q3, #5
    vsri.u16    q10, q2, #11

    // store
    tst         r2, #4
    beq         24f
    vst1.16     {d21}, [r3]!

24:
    tst         r2, #2
    beq         22f
    vst1.32     {d20[1]}, [r3]!

22:
    tst         r2, #1
    beq         21f
    vst1.16     {d20[1]}, [r3]!

21:
    bx          lr

# count >= 8
count_great_than_8:
    mov         r3, #0
    ands        ip, r2, #7
    vmov.u8     d31, #1<<7
    vld1.16     {q12}, [r0]
    vld4.8      {d0-d3}, [r1]
    moveq       ip, #8
    mov         r3, r0

    add         r1, r1, ip, LSL#2
    add         r0, r0, ip, LSL#1
    subs        r2, r2, ip
    b           9f

// LOOP
2:
    vld1.16     {q12}, [r0]!
    vld4.8      {d0-d3}, [r1]!
    vst1.16     {q10}, [r3]
    sub         r3, r0, #8*2
    subs        r2, r2, #8
9:
    pld         [r0,#32]
    // expand 0565 q12 to 8888 {d4-d7}
    vmovn.u16   d4, q12
    vshr.u16    q11, q12, #5
    vshr.u16    q10, q12, #6+5
    vmovn.u16   d5, q11
    vmovn.u16   d6, q10
    vshl.u8     d4, d4, #3
    vshl.u8     d5, d5, #2
    vshl.u8     d6, d6, #3

    // duplicate in 4/2/1 & 8pix vsns
    vmovl.u8    q14, d31
    vmovl.u8    q13, d31
    vmovl.u8    q12, d31
    vmvn.8      d30, d3
    vmlal.u8    q14, d30, d6
    vmlal.u8    q13, d30, d5
    vmlal.u8    q12, d30, d4
    vshr.u16    q8, q14, #5
    vshr.u16    q9, q13, #6
    vaddhn.u16  d6, q14, q8      // moved up
    vshr.u16    q8, q12, #5
    vaddhn.u16  d5, q13, q9
    // intentionally, don't calculate alpha result in d4-d6

    vqadd.u8    d6, d6, d0
    vaddhn.u16  d4, q12, q8

    // pack 8888 {d4-d6} to 0565 q10
    vqadd.u8    d5, d5, d1
    vqadd.u8    d4, d4, d2
    vshll.u8    q10, d6, #8
    vshll.u8    q3, d5, #8
    vshll.u8    q2, d4, #8
    vsri.u16    q10, q3, #5
    vsri.u16    q10, q2, #11

    bne         2b

1:
    vst1.16     {q10}, [r3]

    bx    lr
