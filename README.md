# shellgen

## What is shellgen?

shellgen is a command line utiity that lets you create [shellcode](https://en.wikipedia.org/wiki/Shellcode) without NULL-Bytes.  
Perfect for stack smashing.  

All the removing of nullbytes and some of the optimization is taken care of by the utility.  

## Why the heck would i need to generate shellcode in the first place?

Ofcourse you probably won't need it in your day to day life but if you like playing CTFs as much  
as I do, chances are high that there will be lots of challenges that require you to inject some kind of shellcode.

## What kind of shellcode can shellgen create?

shellgen is currently capable of generating 32bit and 64bit shellcode, both, little and big endian.  


## Sounds good, how can I use it, tough?

Installing is an easy part, you just have to clone the reposity and  
run the Makefile using the `make` command from your terminal.

When executing shellgen itself without any arguments, it should print you a neat little help  
specifying the syntax that you have to use.  

Hope you like it :)