del x
echo off
for %%i in (*.asm) do ext %%i %1 >>x
for %%i in (*.inc) do ext %%i %1 >>x
for %%i in (*.c) do ext %%i %1 >>x


