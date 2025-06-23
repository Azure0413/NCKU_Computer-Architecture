# HW3 P76134082  

## Exercise1  
### 記憶體存取流程，當 RISC-V 程式執行記憶體讀寫指令時，Spike 會呼叫 access 方法：  
### 此方法的工作流程是：  
### 增加相應的存取計數器（read_accesses 或 write_accesses），呼叫 check_tag 方法檢查是否快取命中  
### 如果命中：對於寫操作，設置 DIRTY 標記返回，完成存取  
### 如果未命中：增加相應的缺失計數器（read_misses 或 write_misses），呼叫 victimize 方法選擇一個替換的區塊處理被替換的區塊（如果是髒的，需要寫回）從較低級記憶體讀取新區塊  
### 快取標籤檢查機制  
### check_tag 方法用於判斷特定地址是否在快取中：根據地址計算索引值：idx = (addr >> idx_shift) & (sets-1)，計算標籤值：tag = (addr >> idx_shift) | VALID，檢查該索引下所有路徑（way）中是否有匹配的標籤，如果找到匹配的標籤，返回指向該標籤的指針；否則返回 NULL  
### FIFO  
### FIFO（First-In-First-Out）替換策略通過以下機制工作：  
### 1. 對於組相聯快取（Set-Associative Cache）：使用時間戳記錄每個快取區塊的進入順序，當需要替換時，從同一組中選擇時間戳最小（最早進入）的區塊，way_timestamps 陣列和 next_timestamp 向量用於實現此機制  
### 2. 對於全相聯快取（Fully-Associative Cache）：使用雙向鏈結串列 fifo_entry_order 維護標籤的進入順序，當需要替換時，取出串列前端（最早進入）的標籤，將新標籤添加到串列末端，表示它是最後進入的  
### cachesim.h：在 cache_sim_t 類別中，加入能夠追蹤每個快取區塊進入順序的資料結構  
### uint64_t* way_timestamps;  // 新增：追蹤每個way的進入時間  
### std::vector<uint64_t> next_timestamp;  // 新增：每個set的下一個timestamp  
### std::vector<std::queue<size_t>> fifo_queues;  // 新增：為每個set維護一個FIFO佇列  
### std::list<uint64_t> fifo_entry_order;  // 新增：追蹤tag進入順序的佇列  

### cachesim.cc：需要修改 cache_sim_t::victimize 和 fa_cache_sim_t::victimize 這兩個函式
  
## Exercise2-1  
### 我實作的矩陣轉置演算法採用區塊分割(Block Tiling)技術，這是一種針對快取架構優化的關鍵策略。目前的演算法有以下缺點：  
### 1. 列式存取，源矩陣(src)的存取方式是沿著列進行的，對於行主序(row-major)的記憶體布局，這會導致快取線性能力差  
### 2. 跨越式存取：當矩陣非常大時(如1000×1000)，列式存取會跨越整個快取線，導致局部性極差  
### 3. 高快取缺失率：結果是頻繁的快取缺失，每次缺失都需要100個週期的懲罰  
### 選擇16×16的區塊大小，快取配置相符在8組、4路組相聯的32位元組快取線配置下，16×16的區塊需要1024位元組，且單一區塊快取效益最大化，區塊足夠小，能有效保留在快取中，同時又足夠大，可以顯著減少對主記憶體的存取  
### 優化策略顯著降低了快取缺失率，從而減少了記憶體存取開銷。讀取存取優化：減少了讀取缺失，使之更加接近空間局部性模式。寫入效率提高：目標矩陣的寫入也變得更有效率，因為寫入現在以更好的空間局部性進行  

### Origin：dst[y + x * n] (或寫成 dst[y*n + x]): 這是問題所在。當 y 遞增時，程式寫入 dst 的位置是 dst[0*n + x], dst[1*n + x], dst[2*n + x]...。這些記憶體位置並不連續，而是每次跳躍 n 個元素的距離。  
### New：採用了區塊化 (Blocking) 技術，外層迴圈 (i, j): 用來選擇要處理的區塊。它們以 blockSize 為步長移動。 內層迴圈 (bi, bj): 用來遍歷該區塊內的所有元素，並執行轉置。這種作法的最大好處是，在處理一個小區塊時（例如 16x16），所有需要讀取的 src 元素和需要寫入的 dst 元素都集中在一個相對較小的記憶體範圍內。這個小範圍的資料很有可能被完整地載入到 CPU 快取中（例如 L1 或 L2 Cache）並一直保留，直到這個區塊的轉置全部完成  

## Exercise2-2  
### 實作的矩陣乘法優化利用了區塊分割(Block Tiling)技術並結合多重迴圈重排序和資料重用策略，針對快取架構進行了多方面優化。  
### 目前的演算法有幾個問題，資料重用率低：每次內部迴圈(z迴圈)執行時，都會遍歷整個矩陣的不同部分、快取衝突頻繁：當矩陣大小超過快取容量時，會導致大量快取替換。  
### 我優化的方向是  
### 1. 將大矩陣計算分解成小型的區塊計算，確保處理的資料子集能有效地留在快取中，提高快取利用率  
### 2. 迴圈巢狀重構，這種順序確保資料存取盡可能地連續  
### 3. 資料重用增強，將矩陣的元素存入暫存變數，在內層迴圈中重複使用，減少記憶體存取次數  
### 總而言之，空間局部性增強：連續存取同一區塊內的資料，充分利用每一快取線的所有資料、時間局部性顯著提升：最內層迴圈充分重用已載入快取的資料，A矩陣元素的重複使用減少了額外的記憶體讀取、快取衝突顯著減少：區塊大小的精心選擇減少了組間衝突，區塊內的運算最大化了快取命中率  

### Origin：B[z * j + y]: 這是主要的效能瓶頸。當 z 遞增時，程式存取的位置是 B[y], B[1][y], B[2][y], ...。因為矩陣是「列主序」儲存，這些元素在記憶體中的位置是分散的，每次存取都跳躍了 j 個元素的距離。如果 j 很大，幾乎每次讀取 B 的元素都會導致一次快取失誤 (Cache Miss)。資料重複使用率低 (Poor Temporal Locality): 矩陣 A 的每個元素雖然會被重複使用 j 次，但要等到內層的 y 迴圈跑完一輪後才會再次用到。矩陣 B 的元素同樣重複使用率很低。在這段時間內，這些資料很可能已經被踢出快取，下次使用時需要重新從主記憶體載入。
### New：| 1  2 | 3  4 |  
###      | 5  6 | 7  8 |  
###      |------+------|  
###      | 9 10 |9+2 12|  
###      | 13 14| 15 16|  
### 每一格block中的數值也會存下來在output[x * j + y] = 0;，後續計算的值可以累加上去

## Others  
## HW1  

## Exercise 1. Array Bubble Sort  
利用 lw 和 sw 指令讀取並寫回記憶體，來進行 arr[j] 和 arr[j+1] 的比較與交換。並透過 blt 來決定是否跳過交換。

Bubble Sort 的核心邏輯是如果 arr[j] > arr[j+1]，就要交換。所以就使用了 blt 來進行判斷和操作，如果符合順序就跳過，沒有就用 sw 來交換。

1: (標籤) 這是 blt 指令的跳轉目標，如果 t1 < t2，則直接跳到這裡，不執行交換
f 代表 "forward"（向前搜尋），即程式會往後方的程式碼尋找 1: 標籤


## Exercise 2. Array Search  
我主要實作一個線性搜尋（Linear Search），目標是找到陣列中第一個等於 target 的元素，並回傳其索引值（index），如果找不到則回傳 -1。
(其中 %0 對應 p_a、 %1 對應 result、 %2 對應 arr_size、 %3 對應 target)

設計流程
設定搜尋變數 (t0=arr_size, t1=target, t2=起始指標, t3=索引)。
進入迴圈： 讀取當前元素到 t4。 如果 t4 == target，跳出迴圈並回傳索引。 
若沒找到，繼續搜尋（指標前進，索引加一）。 若遍歷完畢仍未找到，返回 -1。 
最後回傳結果。

## Exercise 3. Linked-List Merge Sort 
整體的設計邏輯： 
Block A (splitList)： 使用 快慢指標法 (Slow and Fast Pointers) 來找到鏈結串列的中點，並將串列拆成兩部分。 
Block B (mergeSortedLists)： 透過 兩條指標 (a, b) 遍歷兩個已排序的鏈結串列，將它們合併為一個有序鏈結串列。 
Block C (main)： 透過組合語言來遍歷排序後的鏈結串列並輸出結果。

針對 splitList 我主要設計了t0 (slow) 和 t1 (fast) 兩個指標，其中 fast 指標每次走兩步 (fast = fast->next->next)，而slow 只走一步 (slow = slow->next)。 
當 fast 到達尾端時，slow 會停在中點，左半部分就是從 head 到 slow，而右半部分就是從 slow->next 開始。另外，拆分 firstHalf 指向 headsecondHalf 指向 slow->next ，斷開 slow->next (讓 slow->next = NULL)，拆成兩條鏈結串列。

針對 mergeSortedLists 我主要透過兩個指標 a 和 b，去掃兩個已排序的鏈結串列，並選擇較小的節點加入，業就是說當哪一個指標的值較小就選擇他加入，並將該指標往下一個走，最後當如果 result == NULL，代表 result 還沒有初始值，則 result = t2。 否則，把 tail->next 指向 t2，再更新 tail。
同時我覺得這個部分是最困難的，因為這部分我卡住最久，因為對 riscv 還不夠熟悉 ld,sd 和 lw, sw 好像會不同，在更新指標的時候要確保 tail 指向的節點正確連接，最後還要確保 result == NULL 的特殊處理。

最後， block c 的部分就是 cur 指向當前節點，然後 cur = cur->next，直到 NULL 結束遍歷。從檔案讀入 linked list、呼叫 mergeSort，並印出排序後的結果。

## HW2  

# Exercise 1. Fast Fourier Transform Calculation  
### 在實作快速傅立葉轉換 (Fast Fourier Transform, FFT) 演算法。FFT 是一種高效計算離散傅立葉轉換 (Discrete Fourier Transform, DFT) 的方法，廣泛應用於訊號處理、影像處理、多項式乘法等領域。此練習的目標是理解 FFT 的原理，並透過程式碼實作，驗證其相較於 O(N^2) 的直接 DFT 計算所具有的 O(N \log N) 效率優勢  
### 對於complex add, sub, mul 其實作法很單純，查看 C code 即可明瞭計算方法  
### log2 的部分，主要是將 N 反覆除以 2 (取整數部分或進行位元右移)，直到 N 變成 0 或 1 為止，這個過程中除法的執行次數  
### 而bit_reverse 和 pi 的部分相對複雜一點，bit_reverse的部分主要是給定一個索引 b 和位元寬度 m，，位元反轉會將 b 的最低 m 個位元的順序顛倒，例如，如果 N=8，則 m = 3。索引 b=3 的二進位表示是 011。反轉後變成 110，對應的十進位數是 6。pi的部分，使用了格雷戈里-萊布尼茲級數 (Gregory-Leibniz series) 來逼近。原理是計算級數 π 4 = 1 − 1/3 + 1/5 − 1/7 + … 。組合語言程式碼透過一個迴圈，重複計算形式為 1 奇數的項，並根據項數是偶數或奇數，交替地將該項加到或減去累計總和。執行固定次數 (iter) 的迭代後，累計的總和會近似於 π / 4 ，最後程式會將此結果乘以 4，得到最終的 π 近似值。  
### 這題我處理最久的是instruction counting的部分，因為有些指令不小心寫在for loop外面導致計算錯誤，還有add和other的instruction數算錯，debug了很久。  
addi t1, x0, 0           # t1 = 0，迴圈 index：k
addi t2, x0, 1           # t2 = 1
addi t3, x0, 1           # t3 = 2k + 1，用來當分母
fcvt.s.w f1, t2          # f1 = float(1) → 分子
fcvt.s.w f0, x0          # f0 = 累積結果 (初始 = 0)

# 迴圈開始
1:
fcvt.s.w f2, t3          # f2 = float(2k + 1)
fdiv.s f3, f1, f2        # f3 = 1 / (2k + 1)
andi t4, t1, 1           # 檢查 k 是奇數還偶數
beq t4, x0, 2f           # 如果是偶數（k%2==0），跳到 2f，加上 f3
fsub.s f0, f0, f3        # 奇數：從總和減掉 f3
jal x0, 3f               # 跳過下一行
2:
fadd.s f0, f0, f3        # 偶數：加上 f3
3:
addi t1, t1, 1           # k += 1
addi t3, t3, 2           # 分母 += 2
blt t1, %[N], 1b         # 如果 k < N，回到迴圈

fmv.s %[pi], f0          # 把最終值存到 pi
  

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

## 差異 
### 2-2 執行一種變形的向量積累（累乘）操作，不是加總而是乘積積累。這不是一般內積（dot product）：一般 dot product 是：result += h[i] * x[i]，但你這段是乘起來的結果（積積）：result *= h[i] * x[i]  

# Exercise 3-1. Single Floating-point Multiplication  
### 此題具體來說，會讀取兩個輸入陣列 h 和 x，計算它們對應元素的乘積 (h[i] * x[i])，然後將所有這些逐元素乘積結果再全部乘起來，得到一個最終的純量 (scalar) 結果 single_floating_result。  
### 此題與Exercise 2-1 類似，需要精確地管理記憶體指標。每次迭代後，h 和 x 的指標都需要正確地增加 sizeof(float)（即 4 位元組）以指向下一個元素。  
### 另外，雖然實作本身不難，但單精度浮點數 (float) 的精度有限（約 7 位十進制有效數字）。當連續乘以多個浮點數時（尤其是在 arr_size 變大時），捨入誤差會不斷累積。如果中間乘積非常大或非常小，還可能出現溢出 (overflow) 或下溢 (underflow) 的問題。這也是為什麼會有 Exercise 3-2 使用雙精度 (double) 來進行比較的原因，以觀察精度差異。

# Exercise 3-2. Double Floating-point Multiplication  
### 這個題目的目標與 Exercise 3-1 非常相似，都是實作陣列的乘積歸約 (product reduction)。不同之處在於，Exercise 3-2 使用 雙精度浮點數 (double) 來儲存輸入陣列 u 和 v 的數據以及最終的計算結果 double_floating_result。演算法會讀取這兩個 double 陣列，計算對應元素的乘積 (u[i] * v[i])，然後將所有這些逐元素乘積結果再全部乘起來，得到一個最終的純量 double 結果。  
### 我認為容易出錯的地方是處理 8 位元組的 double 數據。載入/儲存指令要用對 (fld/fsd)，並且在迴圈中更新指標時，每次必須增加 8 位元組 (addi %[h], %[h], 8) 而不是 4。這個部分也是後來才意識到的。  

## 差異
### 項目	ARRAYMUL_DOUBLE（這個版本）	ARRAYMUL_IMPROVED（前一個版本）
### 資料類型	double（64-bit 浮點數）	float（32-bit 浮點數）
### 處理方式	逐一 scalar 處理	向量化（SIMD）一次處理多筆
### 使用寄存器	f0–f2（浮點標量暫存器）	v0–v2（向量寄存器）
### 迴圈控制	純 addi + blt/bge 控制 index	利用 vsetvli 控制一次處理幾筆
### 資料載入	fld 標量載入	vle32.v 向量載入
### 運算	fmul.d, fmul.d	vfmul.vv, vfadd.vv 等
### 性能	每次只能處理一筆，效能受限	一次處理多筆，性能較佳
### 適用場景	適合小陣列、向量化資源不足時使用	適合大陣列、有向量處理單元的硬體

# macro_define.h  
### cycle count主要就是將riscv中每種指令的cnt與cycle相乘並加總，而cpu time而cpu time就跟課本一樣是cycle time * cycle count，最後 "Ratio" 的目的是用來判斷一個程式是 CPU 密集型 (CPU-bound) 還是記憶體密集型 (Memory-bound)。  