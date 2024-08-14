echo off
for %%i in (*.asm) do ext %%i %1
for %%i in (*.inc) do ext %%i %1
for %%i in (*.c) do ext %%i %1


