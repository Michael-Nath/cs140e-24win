# Answers to `2: use gcc to figure out assembly.`

## 1. What registers holds a pointer (not integer) return value?

code:

```c
int* test(int arr[]) {
    return arr + 1;
}
```

answer: r0

## 2. What register holds the third pointer argument to a routine?

code:

```c
int test(int arg1, int arg2, int arg3) {
    return arg3;
}
```

answer: r2

## 3. If register r1 holds a memory address (a pointer), what instruction do you use to store an 8-bit integer to that location?

code:

```c
__UINT8_TYPE__* test(__UINT8_TYPE__* arr) {
    arr[1] = 5;
    return arr+1;
}
```

answer: strb

## 4. Same as (3) but for load?

code:

```c
__UINT8_TYPE__* test(__UINT8_TYPE__* arr) {
    arr[1] = arr[0];
    return arr+1;
}
```

answer: ldrb

## 5: Load/store 16-bit?

code:

```c
__UINT16_TYPE__* test(__UINT16_TYPE__* arr) {
    arr[1] = arr[0];
    return arr+1;
}
```

answer -  load: ldrh, store: strh

## 6: Load/store 32-bit?

code:

```c
int* test(int* arr) {
    arr[1] = arr[0];
    return arr+1;
}
```

answer - load: ldr, store: str

## 7: Write some C code that will cause the compiler to emit a `bx` instruction that is not a function return.

answer:

```c
int foo(int* (f) (int)) {
    int y = (f)(5);
    return y;
}
```

## 8: What does an infinite loop look like? Why?

code:

```c
int foo() {
    while (1);
}
```

answer:

```asm
00000000 <foo>:
   0:   eafffffe        b       0 <foo>
```

## 9: How do you call a routine whose address is in a register rather than a constant? (You'll use this in the threads lab.)

answer:

```c
void bar() {
    printf("Hi guys\n");
}

void foo(int* (f) (int)) {
    void (*f)(void) = &bar;
    (f)();
}
```

## GET32

```asm
GET32:
ldr r0, [r0]
bx lr
```

## GET16

```asm
GET16:
ldrh r0, [r0]
bx lr
```

## GET8

```asm
GET8:
ldrb r0, [r0]
bx lr
```

## PUT32

```asm
PUT32:
str r1, [r0]
bx lr
```

## PUT16

```asm
PUT16:
strh r1, [r0]
bx lr
```

## PUT8

```asm
PUT8:
strb r1, [r0]
bx lr
```

# Answers to 3.1: `Questions about example-volatile`

## 1. Give two different fixes for 2-wait.c and check that they work.

Fix 1: prepend the `volatile` qualifier to the declaration of the mailbox struct:
`volatile static mailbox_t *mbox =  (void*)0x2000B880;`

This works because of the nature of `volatile` refetching data upon loads.

Fix 2: load the status member upon each iteration of the while loop:

```c
while(status & MAILBOX_FULL) {
    status = mbox->status;
}
```

This works because preivously the compiler could just consider the mbox->status check to happen once 
(since no stores are being issued in between iterations). Now, the `status` variable serves as a proxy
where store are being issued in between iterations. 

## 2. Which of the two files `4-fb.c` and `5-fb.c` (or: both, neither) have a problem? What is it? if one does and not the other, why? Give a fix.

`4-fb.c` has the problem of an infinite loop occurring because the compiler transforms the source
while loop to instead be a if statement that does only one check of the mailbox status.

# Answers to 3: Observability

## 1. Which (if any) assignments can the compiler remove from the code in the file?

answer: the compiler can remove lines 2 and 4. 

## 2. The compiler analyzes foo in isolation, can it reorder or remove any of the following assignments?

answer: the compiler can reorder lines 1 and 2 (or 2 and 3) since they are independent with respect to each other. Additionally, the compiler can remove line 1 because line 3 performs an overwriting store at the same address.

## 3. How much of this code can gcc remove? (Give your intuition!)

answer: gcc can remove the call to `malloc` and the subsequent store to the address pointed to by 
`*p`. My intuition is that the equivalency idea would come into play and since an user would never
see the effects of the malloc + store (cause the pointer is not being returned), it's as if those operations never happened! Thus, those lines can be removed. However, the return 0 must be present because it on the
other hand *is* is an effect that the user sees.

