# HW1 by P76134082 陳冠言  

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

最後， block c 的部分就是 cur 指向當前節點，然後 cur = cur->next，直到 NULL 結束遍歷。