"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi t0, x0, 0\n\t"                

"loop_start:\n\t" 
"addi %[others_cnt], %[others_cnt], 1\n\t"
"bge t0, %[arr_size], loop_end\n\t"  

"addi %[others_cnt], %[others_cnt], 1\n\t"
"slli t1, t0, 2\n\t"        

"addi %[add_cnt], %[add_cnt], 1\n\t"
"add t2, %[h], t1\n\t"                
"addi %[flw_cnt], %[flw_cnt], 1\n\t"
"flw f0, 0(t2)\n\t"            

"addi %[add_cnt], %[add_cnt], 1\n\t"
"add t2, %[x], t1\n\t"             
"addi %[flw_cnt], %[flw_cnt], 1\n\t"
"flw f1, 0(t2)\n\t"                

"addi %[fmul_cnt], %[fmul_cnt], 1\n\t"
"fmul.s f0, f0, f1\n\t"              

"addi %[fadd_cnt], %[fadd_cnt], 1\n\t"
"fadd.s f0, f0, %[id]\n\t"           

"addi %[add_cnt], %[add_cnt], 1\n\t"
"add t2, %[y], t1\n\t"               
"addi %[fsw_cnt], %[fsw_cnt], 1\n\t"
"fsw f0, 0(t2)\n\t"         
"addi %[fsw_cnt], %[fsw_cnt], 1\n\t"
"fsw f0, 0(t2)\n\t"                      

"addi %[add_cnt], %[add_cnt], 1\n\t"
"addi t0, t0, 1\n\t"         

"addi %[others_cnt], %[others_cnt], 1\n\t"
"jal x0, loop_start\n\t"            

"loop_end:\n\t"          