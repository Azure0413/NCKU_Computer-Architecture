void matrix_multiply(int *a, int *b, int *output, int i,
                           int k, int j) {
    // Inplement your code here
    int blockSize = 8;

    for (int x = 0; x < i; x++) {
        for (int y = 0; y < j; y++) {
            output[x * j + y] = 0;
        }
    }
    
    for (int bi = 0; bi < i; bi += blockSize) {
        for (int bk = 0; bk < k; bk += blockSize) {
            for (int bj = 0; bj < j; bj += blockSize) {
                for (int x = bi; x < bi + blockSize && x < i; x++) {
                    for (int z = bk; z < bk + blockSize && z < k; z++) {
                        int a_val = a[x * k + z];
                        for (int y = bj; y < bj + blockSize && y < j; y++) {
                            output[x * j + y] += a_val * b[z * j + y];
                        }
                    }
                }
            }
        }
    }
}