# --- INITIALIZATION ---
810A # LIZ $r1, 10    (r1 = 10)
8205 # LIZ $r2, 5     (r2 = 5)

# --- ARITHMETIC ---
0328 # ADD $r3, $r1, $r2  (r3 = 10 + 5 = 15)
7060 # PUT $r3            (EXPECT OUTPUT: 15)
0C28 # SUB $r4, $r1, $r2  (r4 = 10 - 5 = 5)
7080 # PUT $r4            (EXPECT OUTPUT: 5)

# --- MEMORY ---
8000 # LIZ $r0, 0         (r0 = 0, our memory address)
480C # SW  $r3, $r0       (Store the 15 from r3 into Memory Address 0)
4500 # LW  $r5, $r0       (Load from Memory Address 0 into r5)
70A0 # PUT $r5            (EXPECT OUTPUT: 15)

# --- BRANCHING ---
0E74 # SUB $r6, $r3, $r5  (r6 = 15 - 15 = 0)
BE0D # BZ  $r6, 13        (If r6 == 0, jump to instruction 13. Imm8=13)

# --- SKIPPED INSTRUCTION ---
8763 # LIZ $r7, 99        (If the branch works, this NEVER executes. r7 stays 0)

# --- TARGET & HALT ---
70C0 # PUT $r6            (EXPECT OUTPUT: 0. This is instruction index 13!)
6800 # HALT