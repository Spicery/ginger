(Extracted from a letter to Phil Allen, Mar 2013)

The files in the instruction_set folder are mangled by a python script to generate several different interpreter classes, all of which derive from the Machine class. This was the very first part of the system I wrote, 16 years ago, in the evenings while I was on holiday - and there are some rather peculiar early decisions that I have not yet refactored.

The file extension is a description of the layout of the instruction. The first letter is always "i" for instruction. This is because the first word of an instruction is the instruction pointer/opcode. 

The subsequent letters are "r", "c" or "v" meaning "raw", "constant" or "valof pointer". These indicate the meaning of the words following the instruction word.

In other words, a GVM instruction is a variable number of words in size, with the constants planted inline. That means the garbage collector has to be able to scan compiled code instruction by instruction looking for embedded pointers. (I am not defending these decisions btw! I mostly can but this is not the place.)

Raw words are ignored by the garbage collector. Constants are tagged words and scanned/forwarded. Valof pointers are the C++ objects that hold global variables. Global variables are subject to garbage collection & this mark allows them to be correctly reclaimed.
