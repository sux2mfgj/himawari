#define ASM_FILE
.text
.code64

.globl trap_entry, trap_exit

trap_entry:

    push %r15
    push %r14
    push %r13
    push %r12
    push %r11
    push %r10
    push %r9
    push %r8
    push %rdi
    push %rsi
    push %rbp
    push %rdx
    push %rcx
    push %rbx
    push %rax

    mov %rsp, %rdi
    call trap

trap_exit:
    pop %rax
    pop %rbx
    pop %rcx
    pop %rdx
    pop %rbp
    pop %rsi
    pop %rdi
    pop %r8
    pop %r9
    pop %r10
    pop %r11
    pop %r12
    pop %r13
    pop %r14
    pop %r15

    # ignore error code and trap number
    add $16, %rsp
    iretq

//TODO
//trap_exit:


#define exception_not_error_code(name, number) \
        .text;\
        .globl name;\
        name:;\
            push $0;\
            push $number;\
            jmp trap_entry;\
        .data;\
            .quad name


#define exception_error_code(name, number) \
        .text;\
        .globl name;\
            name:;\
            push $number;\
            jmp trap_entry;\
        .data;\
            .quad name

.data 
.globl vectors
vectors:

exception_not_error_code(divide_error_exception, 0)
exception_not_error_code(debug_exception, 1)
exception_not_error_code(nmi_interrupt, 2)
exception_not_error_code(breakpoint_exception, 3)
exception_not_error_code(overflow_exception, 4)
exception_not_error_code(bound_range_exceeded_exception, 5)
exception_not_error_code(invalid_opecode_exception, 6)
exception_not_error_code(device_not_availabe_exception, 7)
exception_error_code(double_fault_exceptin, 8)
exception_not_error_code(coprocessor_segment_overrun, 9)
exception_error_code(invalid_tss_exception, 10)
exception_error_code(segment_not_present, 11)
exception_error_code(stack_fault_exception, 12)
exception_error_code(general_protection_exception, 13)
exception_error_code(page_fault_exception, 14)
exception_not_error_code(not_set_15, 15)
exception_not_error_code(fpu_floating_point_error, 16)
exception_error_code(alignment_check_exception, 17)
exception_not_error_code(machine_check_exception, 18)
exception_not_error_code(simd_floating_point_exception, 19)
exception_not_error_code(virtualization_exception, 20)
exception_not_error_code(not_set_21, 21)
exception_not_error_code(not_set_22, 22)
exception_not_error_code(not_set_23, 23)
exception_not_error_code(not_set_24, 24)
exception_not_error_code(not_set_25, 25)
exception_not_error_code(not_set_26, 26)
exception_not_error_code(not_set_27, 27)
exception_not_error_code(not_set_28, 28)
exception_not_error_code(not_set_29, 29)
exception_not_error_code(not_set_30, 30)
exception_not_error_code(not_set_31, 31)
exception_not_error_code(not_set_32, 32)
exception_not_error_code(not_set_33, 33)
exception_not_error_code(not_set_34, 34)
exception_not_error_code(not_set_35, 35)
exception_not_error_code(not_set_36, 36)
exception_not_error_code(not_set_37, 37)
exception_not_error_code(not_set_38, 38)
exception_not_error_code(not_set_39, 39)
exception_not_error_code(not_set_40, 40)
exception_not_error_code(not_set_41, 41)
exception_not_error_code(not_set_42, 42)
exception_not_error_code(not_set_43, 43)
exception_not_error_code(not_set_44, 44)
exception_not_error_code(not_set_45, 45)
exception_not_error_code(not_set_46, 46)
exception_not_error_code(not_set_47, 47)
exception_not_error_code(not_set_48, 48)
exception_not_error_code(not_set_49, 49)
exception_not_error_code(not_set_50, 50)
exception_not_error_code(not_set_51, 51)
exception_not_error_code(not_set_52, 52)
exception_not_error_code(not_set_53, 53)
exception_not_error_code(not_set_54, 54)
exception_not_error_code(not_set_55, 55)
exception_not_error_code(not_set_56, 56)
exception_not_error_code(not_set_57, 57)
exception_not_error_code(not_set_58, 58)
exception_not_error_code(not_set_59, 59)
exception_not_error_code(not_set_60, 60)
exception_not_error_code(not_set_61, 61)
exception_not_error_code(not_set_62, 62)
exception_not_error_code(not_set_63, 63)
exception_not_error_code(not_set_64, 64)
exception_not_error_code(not_set_65, 65)
exception_not_error_code(not_set_66, 66)
exception_not_error_code(not_set_67, 67)
exception_not_error_code(not_set_68, 68)
exception_not_error_code(not_set_69, 69)
exception_not_error_code(not_set_70, 70)
exception_not_error_code(not_set_71, 71)
exception_not_error_code(not_set_72, 72)
exception_not_error_code(not_set_73, 73)
exception_not_error_code(not_set_74, 74)
exception_not_error_code(not_set_75, 75)
exception_not_error_code(not_set_76, 76)
exception_not_error_code(not_set_77, 77)
exception_not_error_code(not_set_78, 78)
exception_not_error_code(not_set_79, 79)
exception_not_error_code(not_set_80, 80)
exception_not_error_code(not_set_81, 81)
exception_not_error_code(not_set_82, 82)
exception_not_error_code(not_set_83, 83)
exception_not_error_code(not_set_84, 84)
exception_not_error_code(not_set_85, 85)
exception_not_error_code(not_set_86, 86)
exception_not_error_code(not_set_87, 87)
exception_not_error_code(not_set_88, 88)
exception_not_error_code(not_set_89, 89)
exception_not_error_code(not_set_90, 90)
exception_not_error_code(not_set_91, 91)
exception_not_error_code(not_set_92, 92)
exception_not_error_code(not_set_93, 93)
exception_not_error_code(not_set_94, 94)
exception_not_error_code(not_set_95, 95)
exception_not_error_code(not_set_96, 96)
exception_not_error_code(not_set_97, 97)
exception_not_error_code(not_set_98, 98)
exception_not_error_code(not_set_99, 99)
exception_not_error_code(not_set_100, 100)
exception_not_error_code(not_set_101, 101)
exception_not_error_code(not_set_102, 102)
exception_not_error_code(not_set_103, 103)
exception_not_error_code(not_set_104, 104)
exception_not_error_code(not_set_105, 105)
exception_not_error_code(not_set_106, 106)
exception_not_error_code(not_set_107, 107)
exception_not_error_code(not_set_108, 108)
exception_not_error_code(not_set_109, 109)
exception_not_error_code(not_set_110, 110)
exception_not_error_code(not_set_111, 111)
exception_not_error_code(not_set_112, 112)
exception_not_error_code(not_set_113, 113)
exception_not_error_code(not_set_114, 114)
exception_not_error_code(not_set_115, 115)
exception_not_error_code(not_set_116, 116)
exception_not_error_code(not_set_117, 117)
exception_not_error_code(not_set_118, 118)
exception_not_error_code(not_set_119, 119)
exception_not_error_code(not_set_120, 120)
exception_not_error_code(not_set_121, 121)
exception_not_error_code(not_set_122, 122)
exception_not_error_code(not_set_123, 123)
exception_not_error_code(not_set_124, 124)
exception_not_error_code(not_set_125, 125)
exception_not_error_code(not_set_126, 126)
exception_not_error_code(not_set_127, 127)
exception_not_error_code(not_set_128, 128)
exception_not_error_code(not_set_129, 129)
exception_not_error_code(not_set_130, 130)
exception_not_error_code(not_set_131, 131)
exception_not_error_code(not_set_132, 132)
exception_not_error_code(not_set_133, 133)
exception_not_error_code(not_set_134, 134)
exception_not_error_code(not_set_135, 135)
exception_not_error_code(not_set_136, 136)
exception_not_error_code(not_set_137, 137)
exception_not_error_code(not_set_138, 138)
exception_not_error_code(not_set_139, 139)
exception_not_error_code(not_set_140, 140)
exception_not_error_code(not_set_141, 141)
exception_not_error_code(not_set_142, 142)
exception_not_error_code(not_set_143, 143)
exception_not_error_code(not_set_144, 144)
exception_not_error_code(not_set_145, 145)
exception_not_error_code(not_set_146, 146)
exception_not_error_code(not_set_147, 147)
exception_not_error_code(not_set_148, 148)
exception_not_error_code(not_set_149, 149)
exception_not_error_code(not_set_150, 150)
exception_not_error_code(not_set_151, 151)
exception_not_error_code(not_set_152, 152)
exception_not_error_code(not_set_153, 153)
exception_not_error_code(not_set_154, 154)
exception_not_error_code(not_set_155, 155)
exception_not_error_code(not_set_156, 156)
exception_not_error_code(not_set_157, 157)
exception_not_error_code(not_set_158, 158)
exception_not_error_code(not_set_159, 159)
exception_not_error_code(not_set_160, 160)
exception_not_error_code(not_set_161, 161)
exception_not_error_code(not_set_162, 162)
exception_not_error_code(not_set_163, 163)
exception_not_error_code(not_set_164, 164)
exception_not_error_code(not_set_165, 165)
exception_not_error_code(not_set_166, 166)
exception_not_error_code(not_set_167, 167)
exception_not_error_code(not_set_168, 168)
exception_not_error_code(not_set_169, 169)
exception_not_error_code(not_set_170, 170)
exception_not_error_code(not_set_171, 171)
exception_not_error_code(not_set_172, 172)
exception_not_error_code(not_set_173, 173)
exception_not_error_code(not_set_174, 174)
exception_not_error_code(not_set_175, 175)
exception_not_error_code(not_set_176, 176)
exception_not_error_code(not_set_177, 177)
exception_not_error_code(not_set_178, 178)
exception_not_error_code(not_set_179, 179)
exception_not_error_code(not_set_180, 180)
exception_not_error_code(not_set_181, 181)
exception_not_error_code(not_set_182, 182)
exception_not_error_code(not_set_183, 183)
exception_not_error_code(not_set_184, 184)
exception_not_error_code(not_set_185, 185)
exception_not_error_code(not_set_186, 186)
exception_not_error_code(not_set_187, 187)
exception_not_error_code(not_set_188, 188)
exception_not_error_code(not_set_189, 189)
exception_not_error_code(not_set_190, 190)
exception_not_error_code(not_set_191, 191)
exception_not_error_code(not_set_192, 192)
exception_not_error_code(not_set_193, 193)
exception_not_error_code(not_set_194, 194)
exception_not_error_code(not_set_195, 195)
exception_not_error_code(not_set_196, 196)
exception_not_error_code(not_set_197, 197)
exception_not_error_code(not_set_198, 198)
exception_not_error_code(not_set_199, 199)
exception_not_error_code(not_set_200, 200)
exception_not_error_code(not_set_201, 201)
exception_not_error_code(not_set_202, 202)
exception_not_error_code(not_set_203, 203)
exception_not_error_code(not_set_204, 204)
exception_not_error_code(not_set_205, 205)
exception_not_error_code(not_set_206, 206)
exception_not_error_code(not_set_207, 207)
exception_not_error_code(not_set_208, 208)
exception_not_error_code(not_set_209, 209)
exception_not_error_code(not_set_210, 210)
exception_not_error_code(not_set_211, 211)
exception_not_error_code(not_set_212, 212)
exception_not_error_code(not_set_213, 213)
exception_not_error_code(not_set_214, 214)
exception_not_error_code(not_set_215, 215)
exception_not_error_code(not_set_216, 216)
exception_not_error_code(not_set_217, 217)
exception_not_error_code(not_set_218, 218)
exception_not_error_code(not_set_219, 219)
exception_not_error_code(not_set_220, 220)
exception_not_error_code(not_set_221, 221)
exception_not_error_code(not_set_222, 222)
exception_not_error_code(not_set_223, 223)
exception_not_error_code(not_set_224, 224)
exception_not_error_code(not_set_225, 225)
exception_not_error_code(not_set_226, 226)
exception_not_error_code(not_set_227, 227)
exception_not_error_code(not_set_228, 228)
exception_not_error_code(not_set_229, 229)
exception_not_error_code(not_set_230, 230)
exception_not_error_code(not_set_231, 231)
exception_not_error_code(not_set_232, 232)
exception_not_error_code(not_set_233, 233)
exception_not_error_code(not_set_234, 234)
exception_not_error_code(not_set_235, 235)
exception_not_error_code(not_set_236, 236)
exception_not_error_code(not_set_237, 237)
exception_not_error_code(not_set_238, 238)
exception_not_error_code(not_set_239, 239)
exception_not_error_code(not_set_240, 240)
exception_not_error_code(not_set_241, 241)
exception_not_error_code(not_set_242, 242)
exception_not_error_code(not_set_243, 243)
exception_not_error_code(not_set_244, 244)
exception_not_error_code(not_set_245, 245)
exception_not_error_code(not_set_246, 246)
exception_not_error_code(not_set_247, 247)
exception_not_error_code(not_set_248, 248)
exception_not_error_code(not_set_249, 249)
exception_not_error_code(not_set_250, 250)
exception_not_error_code(not_set_251, 251)
exception_not_error_code(not_set_252, 252)
exception_not_error_code(not_set_253, 253)
exception_not_error_code(not_set_254, 254)
exception_not_error_code(not_set_255, 255)

