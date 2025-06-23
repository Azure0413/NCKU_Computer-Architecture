"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi t0, x0, 0\n\t"

"loop_start:\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"beq %[arr_size], x0, loop_end\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"vsetvli t0, %[arr_size], e32, m1\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"vfmv.v.f v2, %[id]\n\t"

"addi %[lw_cnt], %[lw_cnt], 1\n\t"
"vle32.v v0, (%[h])\n\t"

"addi %[lw_cnt], %[lw_cnt], 1\n\t"
"vle32.v v1, (%[x])\n\t"

"addi %[fmul_cnt], %[fmul_cnt], 1\n\t"
"vfmul.vv v0, v0, v1\n\t"

"addi %[fadd_cnt], %[fadd_cnt], 1\n\t"
"vfadd.vv v0, v0, v2\n\t"

"addi %[sw_cnt], %[sw_cnt], 1\n\t"
"vse32.v v0, (%[y])\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"slli t1, t0, 2\n\t"

"addi %[add_cnt], %[add_cnt], 1\n\t"
"add %[h], %[h], t1\n\t"

"addi %[add_cnt], %[add_cnt], 1\n\t"
"add %[x], %[x], t1\n\t"

"addi %[add_cnt], %[add_cnt], 1\n\t"
"add %[y], %[y], t1\n\t"

"addi %[sub_cnt], %[sub_cnt], 1\n\t"
"sub %[arr_size], %[arr_size], t0\n\t"

"addi %[others_cnt], %[others_cnt], 1\n\t"
"jal x0, loop_start\n\t"

"loop_end:\n\t"
