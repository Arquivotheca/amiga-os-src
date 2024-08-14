#!/bin/perl
while(<>)
{
	s/[ \t](bcc)\.s[ \t]/ \1 /i;
	s/[ \t](bcs)\.s[ \t]/ \1 /i;
	s/[ \t](beq)\.s[ \t]/ \1 /i;
	s/[ \t](bge)\.s[ \t]/ \1 /i;
	s/[ \t](bgt)\.s[ \t]/ \1 /i;
	s/[ \t](bhi)\.s[ \t]/ \1 /i;
	s/[ \t](ble)\.s[ \t]/ \1 /i;
	s/[ \t](bls)\.s[ \t]/ \1 /i;
	s/[ \t](blt)\.s[ \t]/ \1 /i;
	s/[ \t](bmi)\.s[ \t]/ \1 /i;
	s/[ \t](bne)\.s[ \t]/ \1 /i;
	s/[ \t](bpl)\.s[ \t]/ \1 /i;
	s/[ \t](bvc)\.s[ \t]/ \1 /i;
	s/[ \t](bvs)\.s[ \t]/ \1 /i;
	s/[ \t](blo)\.s[ \t]/ \1 /i;
	s/[ \t](bhs)\.s[ \t]/ \1 /i;
	print;
}
