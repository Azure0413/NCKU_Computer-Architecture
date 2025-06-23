void matrix_transpose(int n, int *dst, int *src) {
    // Inplement your code here
    int blockSize = 16;
    
    for (int i = 0; i < n; i += blockSize) {
        for (int j = 0; j < n; j += blockSize) {
            for (int bi = i; bi < i + blockSize && bi < n; bi++) {
                for (int bj = j; bj < j + blockSize && bj < n; bj++) {
                    dst[bj + bi * n] = src[bi + bj * n];
                }
            }
        }
    }
    }