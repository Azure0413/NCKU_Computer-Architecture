"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi t0, x0, 0\n\t"        

"loop_start:\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"bge t0, %[arr_size], loop_end\n\t"

"addi %[flw_cnt], %[flw_cnt], 1\n\t"
"flw f0, 0(%[h])\n\t" 

"addi %[flw_cnt], %[flw_cnt], 1\n\t"
"flw f1, 0(%[x])\n\t" 

"addi %[fmul_cnt], %[fmul_cnt], 1\n\t"
"fmul.s f2, f0, f1\n\t"     
"addi %[fmul_cnt], %[fmul_cnt], 1\n\t"
"fmul.s %[result], %[result], f2\n\t" 

"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi %[h], %[h], 4\n\t"    
"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi %[x], %[x], 4\n\t" 


"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi t0, t0, 1\n\t"
"addi %[others_cnt], %[others_cnt], 1\n\t"
"blt t0, %[arr_size], loop_start\n\t"

"loop_end:\n\t"
"addi %[add_cnt], %[add_cnt], 1\n\t"
"add x0, x0, x0\n\t" 
