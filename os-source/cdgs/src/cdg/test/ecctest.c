					if(chars == (ULONG)'e')
					{
						testit[1] ^= 0x37;
						testit[19] ^= 0x35;
						trashme++;
						if(trashme > 18) trashme = 0;

						CDGTest((ULONG)&testit[0]);
						temp = &testit[0];
						for(x=0;x<24;x++)
						{
							kprintf("$%lx ",(ULONG)*temp);
							temp++;
						}
						kprintf("\n");
					}
					if(chars == (ULONG)'1')
					{
						vererror=FALSE;

						for(byte1=0;byte1<24;byte1++)
						{
						  kprintf(".");
						  for(eormask=1;eormask<64;eormask++)
						  {
							testit[byte1] ^= eormask;
							if(CDGTest((ULONG)&testit[0]))
							{

								for(verify=0;verify<24;verify++)
								{
									if(testit[verify] != master[verify])
									{
										vererror = TRUE;
									}
								}
							}
							else
							{
								kprintf("CDGTest() FAIL\n");
								vererror = TRUE;
							}
							if(vererror)
							{
								kprintf("1 BYTE ERROR FAILED\n");
								kprintf("Byte %ld Mask %lx\n",(ULONG)byte1,(ULONG)eormask);
								byte1 = 24;
							}
						  }
						}
						kprintf("Done\n");
					}
					if(chars == (ULONG)'2')
					{
						vererror=FALSE;

						for(byte1=0;byte1<24;byte1++)
						{
						  kprintf("Byte1=%ld\n",(ULONG)byte1);
						  for(eormask=0;eormask<64;eormask += 3)
						  {
						   for(byte2=0;byte2<24;byte2++)
						   {

						    if(byte2==0) kprintf("Byte2=%ld\n",(ULONG)byte2);
						     for(eor2=0;eor2<64;eor2 += 3)
						     {

							testit[byte1] ^= eormask;
							testit[byte2] ^= eor2;

							if(CDGTest((ULONG)&testit[0]))
							{

								for(verify=0;verify<24;verify++)
								{
									if(testit[verify] != master[verify])
									{
										vererror = TRUE;
									}
								}
							}
							else
							{
								kprintf("CDGTest() FAIL\n");
								vererror = TRUE;
							}
							if(vererror)
							{
								kprintf("2 BYTE ERROR FAILED\n");
								kprintf("Byte1 %ld Mask1 %lx\n",(ULONG)byte1,(ULONG)eormask);
								kprintf("Byte2 %ld Mask2 %lx\n",(ULONG)byte2,(ULONG)eor2);
								byte1 = 24;
								byte2 = 24;
								eormask=64;
								eor2=64;

							}
						    }
						   }
						  }
						}
						kprintf("Done\n");
					}
