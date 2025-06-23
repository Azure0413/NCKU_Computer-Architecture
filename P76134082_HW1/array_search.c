#include <stdio.h>

int arraySearch(int *p_a, int arr_size, int target)
{
    int result = -1;

    asm volatile(
        // Your code
        "mv t0, %2\n\t"         // t0 = arr_size (迴圈計數器)
        "mv t1, %3\n\t"         // t1 = target
        "mv t2, %0\n\t"         // t2 = p_a (指向陣列開頭)
        "li t3, 0\n\t"          // t3 = index = 0

        "1:\n\t"
        "beqz t0, 2f\n\t"       // 如果 t0 (arr_size) == 0，跳到結束
        "lw t4, 0(t2)\n\t"      // t4 = *p_a (讀取當前元素)
        "beq t4, t1, 3f\n\t"    // 如果 t4 == target，跳到找到
        "addi t2, t2, 4\n\t"    // p_a++ (移動到下一個元素)
        "addi t3, t3, 1\n\t"    // index++
        "addi t0, t0, -1\n\t"   // arr_size--
        "j 1b\n\t"              // 跳回迴圈繼續搜尋

        "3:\n\t"
        "mv %1, t3\n\t"         // result = index
        "j 2f\n\t"              // 跳到結束

        "2:\n\t"
        : "+r"(p_a), "+r"(result)
        : "r"(arr_size), "r"(target)
        : "t0", "t1", "t2", "t3", "t4", "memory"
    );

    return result;
}

// Main function to test the implementation
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        fprintf(stderr, "Error opening file: %s\n", argv[1]);
        return 1;
    }
    int arr_size;
    fscanf(input, "%d", &arr_size);
    int arr[arr_size];

    // Read integers from input file into the array
    for (int i = 0; i < arr_size; i++) {
        int data;
        fscanf(input, "%d", &data);
        arr[i] = data;
    }
    int target;
    fscanf(input, "%d", &target);
    fclose(input);

    int *p_a = &arr[0];

    int index = arraySearch(p_a, arr_size, target);

    // Print the result
    printf("%d ", index);
    printf("\n");

    return 0;
}
