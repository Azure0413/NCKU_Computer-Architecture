#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

// Split the linked list into two parts
void splitList(Node *head, Node **firstHalf, Node **secondHalf)
{
    asm volatile(
        /*
        Block A (splitList), which splits the linked list into two halves
        */
        "   mv      t0, %[head]         \n"  // t0 = slow = head
        "   mv      t1, %[head]         \n"  // t1 = fast = head
        "1: ld      t2, 8(t1)           \n"  // t2 = fast->next
        "   beqz    t2, 2f              \n"  // if fast->next is NULL, exit loop
        "   ld      t3, 8(t2)           \n"  // t3 = fast->next->next
        "   beqz    t3, 2f              \n"  // if fast->next->next is NULL, exit loop
        "   ld      t0, 8(t0)           \n"  // slow = slow->next
        "   mv      t1, t3              \n"  // fast = fast->next->next
        "   j       1b                  \n"  // continue loop
        "2: sd      %[head], 0(%[first])\n"  // *firstHalf = head
        "   ld      t4, 8(t0)           \n"  // t4 = slow->next
        "   sd      t4, 0(%[second])    \n"  // *secondHalf = slow->next
        "   sd      zero, 8(t0)         \n"  // slow->next = NULL
        : [first]"+r"(firstHalf), [second]"+r"(secondHalf)
        : [head]"r"(head)
        : "t0", "t1", "t2", "t3", "t4", "memory"
    );
}

// Merge two sorted linked lists
Node *mergeSortedLists(Node *a, Node *b)
{
    Node *result = NULL;
    Node *tail = NULL;

    asm volatile(
        /*
        Block B (mergeSortedList), which merges two sorted lists into one
        */
        "1:                             \n"
        "  beqz %[a], 2f                \n"  // if a is NULL, go to 2
        "  beqz %[b], 3f                \n"  // if b is NULL, go to 3
        "  ld t0, 0(%[a])               \n"  // t0 = a->data
        "  ld t1, 0(%[b])               \n"  // t1 = b->data
        "  bge t0, t1, 4f               \n"  // if a->data >= b->data, go to 4
        "  mv t2, %[a]                  \n"  // t2 = current = a
        "  ld %[a], 8(%[a])             \n"  // a = a->next
        "  j 5f                         \n"  // go to 5
        "4:                             \n"
        "  mv t2, %[b]                  \n"  // t2 = current = b
        "  ld %[b], 8(%[b])             \n"  // b = b->next
        "5:                             \n"
        "  beqz %[result], 6f           \n"  // if result is NULL, go to 6
        "  sd t2, 8(%[tail])            \n"  // tail->next = current
        "  j 7f                         \n"  // go to 7
        "6:                             \n"
        "  mv %[result], t2             \n"  // result = current
        "7:                             \n"
        "  mv %[tail], t2               \n"  // tail = current
        "  j 1b                         \n"  // continue loop
        "2:                             \n"
        "  beqz %[result], 8f           \n"  // if result is NULL, go to 8
        "  sd %[b], 8(%[tail])          \n"  // tail->next = b
        "  j 9f                         \n"  // go to 9
        "3:                             \n"
        "  beqz %[result], 8f           \n"  // if result is NULL, go to 8
        "  sd %[a], 8(%[tail])          \n"  // tail->next = a
        "  j 9f                         \n"  // go to 9
        "8:                             \n"
        "  mv %[result], %[b]           \n"  // result = b (or a)
        "9:                             \n"  // end
        : [result]"+r"(result), [tail]"+r"(tail), [a]"+r"(a), [b]"+r"(b)
        :
        : "t0", "t1", "t2", "memory"
    );

    return result;
}

// Merge Sort function for linked list
Node *mergeSort(Node *head)
{
    if (!head || !head->next)
        return head; // Return directly if there is only one node

    Node *firstHalf, *secondHalf;
    splitList(head, &firstHalf,
              &secondHalf); // Split the list into two sublists

    firstHalf = mergeSort(firstHalf);   // Recursively sort the left half
    secondHalf = mergeSort(secondHalf); // Recursively sort the right half

    return mergeSortedLists(firstHalf, secondHalf); // Merge the sorted sublists
}

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
    int list_size;
    fscanf(input, "%d", &list_size);
    Node *head = (list_size > 0) ? (Node *)malloc(sizeof(Node)) : NULL;
    Node *cur = head;
    for (int i = 0; i < list_size; i++) {
        fscanf(input, "%d", &(cur->data));
        if (i + 1 < list_size)
            cur->next = (Node *)malloc(sizeof(Node));
        cur = cur->next;
    }
    fclose(input);

    // Linked list sort
    head = mergeSort(head);

    cur = head;
    while (cur) {
        printf("%d ", cur->data);
        asm volatile(
            /*
            Block C (Move to the next node), which updates the pointer to
            traverse the linked list
            */
            "ld      %[cur], 8(%[cur])  \n"  // cur = cur->next
            : [cur]"+r"(cur)
            :
            : "memory"
        );
    }
    
    printf("\n");
    return 0;
}
