"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi t1, x0, 0\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi t2, x0, 1\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi t3, x0, 1\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"fcvt.s.w f1, t2\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"fcvt.s.w f0, x0\n\t"
"1:\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"fcvt.s.w f2, t3\n\t"
"addi %[fdiv_cnt], %[fdiv_cnt], 1\n\t"
"fdiv.s f3, f1, f2\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"andi t4, t1, 1\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"beq t4, x0, 2f\n\t"
"addi %[fsub_cnt], %[fsub_cnt], 1\n\t"
"fsub.s f0, f0, f3\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"jal x0, 3f\n\t"
"2:\n\t"
"addi %[fadd_cnt], %[fadd_cnt], 1\n\t"
"fadd.s f0, f0, f3\n\t"
"3:\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi t1, t1, 1\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi t3, t3, 2\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"blt t1, %[N], 1b\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"fmv.s %[pi], f0\n\t"
