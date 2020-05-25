### A tiny complier and code simulater with C-Minus

Usage:

* make

* make TM

* ./cminus -i \<filename> -o \<filename> [-t]
  * -i filename: the path of source code file
  * -o filename: the path of target .tm file
  * -t(optional): if presenting, it will stop adding trace information to the .tm file
  when it's running, these files will be generated for debugging:
  
  * delCommont.txt:deleted the comments

  * scanner.txt: the result of the scanner

  * Token_Tree.out: the result of parser

* ./tiny \<filename> 

  this will run the code with TM machine

    the usage of TM machine is:

    * s(tep \<n>      Execute n (default 1) TM instructions

    * g(o            Execute TM instructions until HALT

    * r(egs          Print the contents of the registers

    * i(Mem \<b \<n>>  Print n iMem locations starting at b

    * d(Mem \<b \<n>>  Print n dMem locations starting at b

    * t(race         Toggle instruction trace

    * p(rint         Toggle print of total instructions executed ('go' only)

    * c(lear         Reset simulator for new execution of program

    * h(elp          Cause this list of commands to be printed

    * q(uit          Terminate the simulation


