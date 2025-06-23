# Exercise 1. Fast Fourier Transform Calculation  
### 在實作快速傅立葉轉換 (Fast Fourier Transform, FFT) 演算法。FFT 是一種高效計算離散傅立葉轉換 (Discrete Fourier Transform, DFT) 的方法，廣泛應用於訊號處理、影像處理、多項式乘法等領域。此練習的目標是理解 FFT 的原理，並透過程式碼實作，驗證其相較於 O(N^2) 的直接 DFT 計算所具有的 O(N \log N) 效率優勢  
### 對於complex add, sub, mul 其實作法很單純，查看 C code 即可明瞭計算方法  
### log2 的部分，主要是將 N 反覆除以 2 (取整數部分或進行位元右移)，直到 N 變成 0 或 1 為止，這個過程中除法的執行次數  
### 而bit_reverse 和 pi 的部分相對複雜一點，bit_reverse的部分主要是給定一個索引 b 和位元寬度 m，，位元反轉會將 b 的最低 m 個位元的順序顛倒，例如，如果 N=8，則 m = 3。索引 b=3 的二進位表示是 011。反轉後變成 110，對應的十進位數是 6。pi的部分，使用了格雷戈里-萊布尼茲級數 (Gregory-Leibniz series) 來逼近。原理是計算級數 π 4 = 1 − 1/3 + 1/5 − 1/7 + … 。組合語言程式碼透過一個迴圈，重複計算形式為 1 奇數的項，並根據項數是偶數或奇數，交替地將該項加到或減去累計總和。執行固定次數 (iter) 的迭代後，累計的總和會近似於 π / 4 ，最後程式會將此結果乘以 4，得到最終的 π 近似值。  
### 這題我處理最久的是instruction counting的部分，因為有些指令不小心寫在for loop外面導致計算錯誤，還有add和other的instruction數算錯，debug了很久。  

# Exercise 2-1. Array Multiplication without V Extension  
### 核心邏輯是一個迴圈，遍歷從索引 0 到 arr_size - 1 的所有元素。 在迴圈的每一次迭代中（假設當前索引為 i）：  
### 載入 (Load): 從記憶體中分別讀取 h[i] 和 x[i] 的值到浮點數暫存器。 這會使用 flw (floating-point load word) 指令。  
### 乘法 (Multiply): 將讀取的 h[i] 和 x[i] 進行浮點數乘法運算。 這會使用 fmul.s (floating-point multiply, single-precision) 指令。  
### 加法 (Add): 將上一步得到的乘積結果與預先載入的 id 值進行浮點數加法運算。 這會使用 fadd.s (floating-point add, single-precision) 指令。  
### 儲存 (Store): 將最終的計算結果 (h[i] * x[i] + id) 寫回到記憶體中 y[i] 的位置。 這會使用 fsw (floating-point store word) 指令。  
### 這個過程會重複 arr_size 次，每次處理陣列中的一個元素，直到所有元素都計算完成。  
### 這一題解題主要就是跟著註解c code的邏輯撰寫，但要注意要使用float instructions來做，在instruction counting的時候也要換成 float 的 cnt。  

# Exercise 2-2. Array Multiplication with V Extension  
### 在每次迴圈迭代中：  
### 設定向量長度 (vsetvl): 使用 vsetvl t0, %[arr_size], e32, m1 指令。這個關鍵指令會根據硬體向量暫存器長度 (VLEN=128 位元) 、設定的元素寬度 (e32，即 32 位元 float)、向量暫存器分組 (m1) 以及剩餘未處理的元素數量 (%[arr_size]) 來決定這次迭代實際要處理多少個元素 (vl)，並將 vl 的值存入 t0。  
### 向量載入 (vle32.v): 使用 vle32.v v0, (%[h]) 和 vle32.v v1, (%[x]) 指令。這兩條指令分別從記憶體位址 h 和 x 開始，一次載入 vl 個 float 元素到向量暫存器 v0 和 v1。  
### 接著會進行向量的加法和乘法，最後使用 vse32.v v0, (%[y]) 指令進行向量儲存。另外，迴圈判斷: 檢查 arr_length (%[arr_size]) 是否為零 (beq %[arr_size], x0, loop_end)，若不為零則跳回 (jal x0, loop_start) 繼續處理下一批元素。  
### 此題困難的部分是 V 擴充指令集相當豐富，需要選擇正確的指令，並理解其操作對象（暫存器或記憶體）、元素寬度 (e32) 和操作類型（.vv 代表 vector-vector, .vf 代表 vector-scalar, .vi 代表 vector-immediate）。  

# Exercise 3-1. Single Floating-point Multiplication  
### 此題具體來說，會讀取兩個輸入陣列 h 和 x，計算它們對應元素的乘積 (h[i] * x[i])，然後將所有這些逐元素乘積結果再全部乘起來，得到一個最終的純量 (scalar) 結果 single_floating_result。  
### 此題與Exercise 2-1 類似，需要精確地管理記憶體指標。每次迭代後，h 和 x 的指標都需要正確地增加 sizeof(float)（即 4 位元組）以指向下一個元素。  
### 另外，雖然實作本身不難，但單精度浮點數 (float) 的精度有限（約 7 位十進制有效數字）。當連續乘以多個浮點數時（尤其是在 arr_size 變大時），捨入誤差會不斷累積。如果中間乘積非常大或非常小，還可能出現溢出 (overflow) 或下溢 (underflow) 的問題。這也是為什麼會有 Exercise 3-2 使用雙精度 (double) 來進行比較的原因，以觀察精度差異。

# Exercise 3-2. Double Floating-point Multiplication  
### 這個題目的目標與 Exercise 3-1 非常相似，都是實作陣列的乘積歸約 (product reduction)。不同之處在於，Exercise 3-2 使用 雙精度浮點數 (double) 來儲存輸入陣列 u 和 v 的數據以及最終的計算結果 double_floating_result。演算法會讀取這兩個 double 陣列，計算對應元素的乘積 (u[i] * v[i])，然後將所有這些逐元素乘積結果再全部乘起來，得到一個最終的純量 double 結果。  
### 我認為容易出錯的地方是處理 8 位元組的 double 數據。載入/儲存指令要用對 (fld/fsd)，並且在迴圈中更新指標時，每次必須增加 8 位元組 (addi %[h], %[h], 8) 而不是 4。這個部分也是後來才意識到的。  

# macro_define.h  
### cycle count主要就是將riscv中每種指令的cnt與cycle相乘並加總，而cpu time而cpu time就跟課本一樣是cycle time * cycle count，最後 "Ratio" 的目的是用來判斷一個程式是 CPU 密集型 (CPU-bound) 還是記憶體密集型 (Memory-bound)。  