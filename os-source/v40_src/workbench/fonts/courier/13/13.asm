;------ included files ------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"libraries/diskfont.i"
	INCLUDE		"13_rev.i"

;------ code to handle execution of this file ------
		moveq	#100,D0
		rts

;------ DiskFontHeader structure ------
		dc.l	0		; ln_Succ
		dc.l	0		; ln_Pred
		dc.b	NT_FONT		; ln_Type
		dc.b	0		; ln_Pri
		dc.l	font13Name	; ln_Name
		dc.w	DFH_ID		; FileID
		dc.w	0		; Revision
		dc.l	0		; Segment
font13Name:
		dc.b	0,'$VER: courier'
		VERS
		dc.b	' ('
		DATE
		dc.b	')',0
		dcb.b	MAXFONTNAME+8-(*-font13Name),0

;------ TextFont structure ------
		dc.b	NT_FONT		; ln_Type
		dc.b	0		; ln_Pri
		dc.l	font13Name	; ln_Name
		dc.l	0		; mn_ReplyPort
		dc.w	0		; (graphics library internal use)
		dc.w	13		; tf_YSize
		dc.b	$00		; tf_Style
		dc.b	$42		; tf_Flags
		dc.w	7		; tf_XSize
		dc.w	9		; tf_Baseline
		dc.w	1		; tf_BoldSmear
		dc.w	0		; tf_Accessors
		dc.b	32		; tf_LoChar
		dc.b	255		; tf_HiChar
		dc.l	font13Data	; tf_CharData
		dc.w	FONT13MODULO	; tf_Modulo
		dc.l	font13Loc	; tf_CharLoc
		dc.l	font13Space	; tf_CharSpace
		dc.l	font13Kern	; tf_CharKern


;;   FONT DISPLAY
;
;   $20   .$21   .$22   .$23   .$24   .$25    .$26   .$27   .$28   .$29   .
;   >     <>     <>     <>     <>     <>     < >     <>     <>     <>     <
;                                                                          
;                                  *                                       
;             *     * *     * *   ***    **      **      *        *  *     
;             *     * *     * *  * * *  *  *    *        *       *    *    
;             *     * *   ****** * *     **  *  *       *        *    *    
;             *            * *    **       **    *   *          *      *   
;             *            * *     **    **     *** *           *      *   
;             *          ******  * * *  *  **  *   *            *      *   
;                         * *    * * *    *  * *   **           *      *   
;             *           * *     ***      **   ***  *           *    *    
;___                               *                             *    *    
;                                                                 *  *     
;                                                                          
;
;   $2a   .$2b   .$2c   .$2d   .$2e   .$2f   .$30   .$31   .$32   .$33   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;                                            *                            
;     * *                                   *   ***     *     ***    ***  
;      *                                    *  *   *  ***    *   *  *   * 
;    *****    *                            *   *   *    *    *   *      * 
;      *      *                            *   *   *    *       *     **  
;     * *   *****         *****           *    *   *    *      *        * 
;             *                           *    *   *    *     *         * 
;             *     *                    *     *   *    *    *   *  *   * 
;                   *             *      *      ***   *****  *****   ***  
;___               *                    *                                 
;                                                                         
;                                                                         
;
;   $34   .$35   .$36   .$37   .$38   .$39   .$3a   .$3b   .$3c   .$3d   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;                                                                         
;       *   *****    ***  *****   ***    ***                              
;      **   *       *     *   *  *   *  *   *                   *         
;     * *   *      *         *   *   *  *   *                  *          
;    *  *   ****   ****      *    ***   *   *    *      *     *     ***** 
;   *   *       *  *   *     *   *   *   ****                *            
;   ******      *  *   *    *    *   *      *                 *     ***** 
;       *  *    *  *   *    *    *   *     *            *      *          
;      ***  ****    ***     *     ***   ***      *      *       *         
;___                                                   *                  
;                                                                         
;                                                                         
;;
;   $3e   .$3f   .$40   .$41   .$42   .$43   .$44   .$45   .$46   .$47   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;                                                                         
;            ***    ***   ***   *****    ***  *****  ****** ******   ***  
;     *     *   *  *   *    *    *   *  *   *  *   *  *   *  *   *  *   * 
;      *        *  *  **   * *   *   *  *   *  *   *  *  *   * * *  *   * 
;       *      *   * * *   * *   ****   *      *   *  ****   ***    *     
;        *    *    * * *  *****  *   *  *      *   *  *  *   * *    *  ** 
;       *     *    *  **  *   *  *   *  *   *  *   *  *      *      *   * 
;      *           *      *   *  *   *  *   *  *   *  *   *  *      *   * 
;     *       *     ***  *** ********    ***  *****  ****** ****     ***  
;___                                                                      
;                                                                         
;                                                                         
;
;   $48   .$49   .$4a   .$4b   .$4c   .$4d   .$4e   .$4f   .$50   .$51   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;                                                                         
;   *** *** *****   **** *** ** ***    **   ****  ***  ***  *****    ***  
;    *   *    *       *   *  *   *      ** **  **  *  *   *  *  **  *   * 
;    *   *    *       *   * *    *      ** **  **  *  *   *  *   *  *   * 
;    *****    *       *   **     *      * * *  * * *  *   *  *  **  *   * 
;    *   *    *       *   ***    *      * * *  * * *  *   *  ****   *   * 
;    *   *    *    *  *   *  *   *   *  * * *  *  **  *   *  *      *   * 
;    *   *    *    *  *   *   *  *   *  *   *  *  **  *   *  *      *   * 
;   *** *** *****   **   ***  ******** *** ******  *   ***  ****     ***  
;___                                                                   ** 
;                                                                         
;                                                                         
;
;   $52   .$53   .$54   .$55   .$56   .$57   .$58   .$59   .$5a   .$5b   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;                                                                         
;   *****    **** ********** ****** ****** ****** ****** *** *****    **  
;    *   *  *   * *  *  * *   *  *   *  *   *  *   *  *   *  *   *    *   
;    *   *  *     *  *  * *   *  *   *  * * *   * *    * *   *  *     *   
;    ****   ***      *    *   *  ** **  * * *    *     * *     *      *   
;    *  *     ***    *    *   *   * *   * * *   * *     *      *      *   
;    *   *      *    *    *   *   * *   ** **   * *     *     *  *    *   
;    *   *  *   *    *    ** **   ***    * *   *   *    *    *   *    *   
;   ***  ** ****   *****   ***     *     * *  *** ***  ***   *****    *   
;___                                                                  *   
;                                                                     **  
;                                                                         
;;
;   $5c   .$5d   .$5e   .$5f   .$60   .$61   .$62   .$63   .$64   .$65   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;    *                           *                                        
;     *      **      *            *           **                **        
;     *       *     * *                        *                 *        
;      *      *    *   *                ****   ****    ***    ****   ***  
;      *      *                             *  *   *  *   *  *   *  *   * 
;       *     *                          ****  *   *  *      *   *  ***** 
;       *     *                         *   *  *   *  *      *   *  *     
;        *    *                         *   *  *   *  *   *  *   *  *   * 
;        *    *                          *** ** ***    ***    *** *  ***  
;___      *   *                                                           
;            **                                                           
;                        *******                                          
;
;   $66   .$67   .$68   .$69   .$6a   .$6b   .$6c   .$6d   .$6e   .$6f   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;                                                                         
;      ***        **        *       *  **      ***                        
;     *            *                    *        *                        
;    *****   *** * * **   ***    ****   *  **    *   *** *  * ***    ***  
;     *     *   *  **  *    *       *   * *      *    * * *  **  *  *   * 
;     *     *   *  *   *    *       *   **       *    * * *  *   *  *   * 
;     *     *   *  *   *    *       *   * *      *    * * *  *   *  *   * 
;     *     *   *  *   *    *       *   *  *     *    * * *  *   *  *   * 
;    ****    **** *** *** *****     *  **   ** ******** * *****  **  ***  
;___            *                   *                                     
;               *                   *                                     
;            ***                 ***                                      
;
;   $70   .$71   .$72   .$73   .$74   .$75   .$76   .$77   .$78   .$79   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;                                                                         
;                                 *                                       
;                                 *                                       
;   * ***    *** * * ***   ***   ***** **  ** *** ****** ****** ****** ***
;    *   *  *   *   *     *   *   *     *   *  *   *  *   *   * *   *   * 
;    *   *  *   *   *      ***    *     *   *  *   *  * * *    *    *   * 
;    *   *  *   *   *         *   *     *   *   * *   * * *   * *    * *  
;    *   *  *   *   *     *   *   *   * *   *   ***    * *   *   *   ***  
;    ****    ****  ****    ***     ***   *** *   *     * *  **   **   *   
;___ *          *                                                     *   
;    *          *                                                     *   
;   ***        ***                                                  **    
;;
;   $7a   .$7b   .$7c   .$7d   .$7e   .$a0   .$a1   .$a2   .$a3   .$a4   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;                                                                         
;              *     *     *                            *     **          
;             *      *      *                           *    *  *  *    * 
;    *****    *      *      *                    *     ***   *      ****  
;    *  *     *      *      *     *  *                * * * *****   *  *  
;      *     *       *       *   * * *                * *     *     *  *  
;     *       *      *      *    *  *            *    * * *   *     ****  
;    *   *    *      *      *                    *     ***   *   * *    * 
;    *****    *      *      *                    *      *   *****         
;___          *      *      *                    *      *                 
;              *     *     *                     *                        
;                                                *                        
;
;   $a5   .$a6   .$a7   .$a8   .$a9   .$aa   .$ab   .$ac   .$ad   .$ae   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;                   ****                                                  
;   *** ***   *     *  *   * *    ***    **                          ***  
;    *   *    *     **           *   *  *  *                        *   * 
;     * *     *    *  *         *  ** *  ***    *  *               * *** *
;    *****    *    *   *        * *   *  * *   *  *   *****        * * * *
;      *            *  *        * *   *       *  *        *  ***** * **  *
;    *****           **         *  ** * ****   *  *       *        * * * *
;      *      *    *  *          *   *          *  *                *   * 
;     ***     *    ****           ***                                ***  
;___          *                                                           
;             *                                                           
;                                                                         
;
;   $af   .$b0   .$b1   .$b2   .$b3   .$b4   .$b5   .$b6   .$b7   .$b8   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;                                          *          *****               
;    *****   **            **     **      *          * * *                
;           *  *          *  *   *  *                * * *                
;           *  *     *      *      *          **  ** * * *                
;            **    *****   *     *  *          *   *  ** *   ***          
;                    *    ****    **           *   *   * *   ***          
;                                              *   *   * *   ***          
;                  *****                       *   *   * *                
;                                              **** * ** **               
;___                                           *                      *   
;                                              *                       *  
;                                              *                     **   
;;
;   $b9   .$ba   .$bb   .$bc    .$bd    .$be    .$bf   .$c0   .$c1   .
;   >     <>     <>     <>     < >     < >     < >     <>     <>     <
;                                                         *       *   
;                                                          *     *    
;      *     **           *       *       **                          
;     **    *  *         **    * **    * *  *  *         ***    ***   
;      *    *  *   *  *   *   *   *   *    *  *     *      *      *   
;      *     **     *  *  *  * *  *  *** *  ** *          * *    * *  
;     ***            *  ***** ** *****  * *** **    *     * *    * *  
;           ****    *  *   * * *   *   *   * * *    *    *****  ***** 
;                  *  *   *  **** *   *   *  ****  *     *   *  *   * 
;                              *     ****      *  *     *** ****** ***
;___                                              *   *               
;                                                  ***                
;                                                                     
;
;   $c2   .$c3   .$c4   .$c5   .$c6   .$c7   .$c8   .$c9   .$ca   .$cb   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;      *     * *            *                    *      *      *          
;     * *   * *     * *    * *                    *    *      * *   * *   
;                           *    *****   ***                              
;    ***    ***    ***    ***     **    *   * ****** ****** ****** ****** 
;      *      *      *      *    * *    *   *  *   *  *   *  *   *  *   * 
;     * *    * *    * *    * *   * ***  *      *  *   *  *   *  *   *  *  
;     * *    * *    * *    * *   * *    *      ****   ****   ****   ****  
;    *****  *****  *****  ***** ****    *   *  *  *   *  *   *  *   *  *  
;    *   *  *   *  *   *  *   * *  *    *   *  *   *  *   *  *   *  *   * 
;   *** ****** ****** ****** ****  ***   ***  ****** ****** ****** ****** 
;___                                      *                               
;                                          *                              
;                                        **                               
;
;   $cc   .$cd   .$ce   .$cf   .$d0   .$d1   .$d2   .$d3   .$d4   .$d5   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;      *       *     *                    * *    *       *     *     * *  
;       *     *     * *    * *           * *      *     *     * *   * *   
;                               *****                                     
;    *****  *****  *****  *****  *   * *** ***  ***    ***    ***    ***  
;      *      *      *      *    *   *  **  *  *   *  *   *  *   *  *   * 
;      *      *      *      *   ***  *  * * *  *   *  *   *  *   *  *   * 
;      *      *      *      *    *   *  * * *  *   *  *   *  *   *  *   * 
;      *      *      *      *    *   *  *  **  *   *  *   *  *   *  *   * 
;      *      *      *      *    *   *  *  **  *   *  *   *  *   *  *   * 
;    *****  *****  *****  ***** *****  ***  *   ***    ***    ***    ***  
;___                                                                      
;                                                                         
;                                                                         
;;
;   $d6   .$d7   .$d8   .$d9   .$da   .$db   .$dc   .$dd   .$de   .$df   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                           *       *     *              *                
;     * *              *     *     *     * *    * *     *                 
;                   ***                                     ***      **   
;     ***          *  ** *** ****** ****** ****** ****** *** *      *  *  
;    *   *  *   *  * * *  *   *  *   *  *   *  *   *  *   *  ****   *  *  
;    *   *   * *   * * *  *   *  *   *  *   *  *   *   * *   *   *  * *   
;    *   *    *    * * *  *   *  *   *  *   *  *   *    *    *   *  *  *  
;    *   *   * *   * * *  *   *  *   *  *   *  *   *    *    ****   *   * 
;    *   *  *   *  **  *  ** **  ** **  ** **  ** **    *    *      * * * 
;     ***           ***    ***    ***    ***    ***    ***  ***    ** **  
;___               *                                                      
;                                                                         
;                                                                         
;
;   $e0   .$e1   .$e2   .$e3   .$e4   .$e5   .$e6   .$e7   .$e8   .$e9   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;      *       *     *      * *           *                   *       *   
;       *     *     * *    * *    * *    * *                   *     *    
;                                         *                               
;    ****   ****   ****   ****   ****   ****   ** **   ***    ***    ***  
;        *      *      *      *      *      *    *  * *   *  *   *  *   * 
;     ****   ****   ****   ****   ****   ****  ****** *      *****  ***** 
;    *   *  *   *  *   *  *   *  *   *  *   * *  *    *      *      *     
;    *   *  *   *  *   *  *   *  *   *  *   * *  *  * *   *  *   *  *   * 
;     *** *  *** *  *** *  *** *  *** *  *** * ** **   ***    ***    ***  
;___                                                    *                 
;                                                        *                
;                                                      **                 
;
;   $ea   .$eb   .$ec   .$ed   .$ee   .$ef   .$f0   .$f1   .$f2   .$f3   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;      *            *       *      *           ** *    * *    *       *   
;     * *    * *     *     *      * *    * *  * **    * *      *     *    
;                                              *  *                       
;     ***    ***   ***    ***    ***    ***     **** * ***    ***    ***  
;    *   *  *   *    *      *      *      *    *   *  **  *  *   *  *   * 
;    *****  *****    *      *      *      *    *   *  *   *  *   *  *   * 
;    *      *        *      *      *      *    *   *  *   *  *   *  *   * 
;    *   *  *   *    *      *      *      *    *   *  *   *  *   *  *   * 
;     ***    ***   *****  *****  *****  *****   ***  ***  **  ***    ***  
;___                                                                      
;                                                                         
;                                                                         
;;
;   $f4   .$f5   .$f6   .$f7   .$f8   .$f9   .$fa   .$fb   .$fc   .$fd   .
;   >     <>     <>     <>     <>     <>     <>     <>     <>     <>     <
;                                                                         
;      *     * *                         *       *      *              *  
;     * *   * *     * *                   *     *      * *    * *     *   
;                                    *                                    
;     ***    ***    ***     *     ***  **  ** **  ** **  ** **  ** *** ***
;    *   *  *   *  *   *         *  **  *   *  *   *  *   *  *   *  *   * 
;    *   *  *   *  *   *  *****  * * *  *   *  *   *  *   *  *   *  *   * 
;    *   *  *   *  *   *         * * *  *   *  *   *  *   *  *   *   * *  
;    *   *  *   *  *   *    *    **  *  *   *  *   *  *   *  *   *   ***  
;     ***    ***    ***           ***    *** *  *** *  *** *  *** *   *   
;___                             *                                    *   
;                                                                     *   
;                                                                   **    
;
;   $fe   .$ff   .$--   .
;   >     <>     <>     <
;                        
;                  ***** 
;   **       * *   *   * 
;    *             *   * 
;    ****  *** *** *   * 
;    *   *  *   *  *   * 
;    *   *  *   *  *   * 
;    *   *   * *   *   * 
;    *   *   ***   *   * 
;    ****     *    *   * 
;___ *        *    *   * 
;    *        *    ***** 
;   ***     **           
;
font13Loc:
		dc.l	$00000000,$00000001,$000c0003,$001a0007
		dc.l	$002c0005,$00310006,$00400007,$00200002
		dc.l	$004f0003,$004d0003,$00600005,$00770005
		dc.l	$009f0002,$00740005,$00c50001,$009f0006
		dc.l	$00d20005,$00c10005,$00e40005,$00e90005
		dc.l	$00f20006,$00690006,$01080005,$011e0005
		dc.l	$00ed0005,$01230005,$00460001,$012c0002
		dc.l	$00be0004,$01360005,$00b50004,$01410005
		dc.l	$00d60005,$00c50007,$014f0006,$00280005
		dc.l	$00230006,$015b0006,$00070006,$01700005
		dc.l	$01490007,$00220005,$007f0005,$01750007
		dc.l	$01610006,$01550007,$010d0007,$00d20005
		dc.l	$01820006,$01880005,$01910007,$019e0005
		dc.l	$000e0007,$00830007,$01130007,$00890007
		dc.l	$00010007,$01190007,$01a30005,$004c0002
		dc.l	$009a0006,$00510002,$005c0005,$01a80007
		dc.l	$00a40002,$01b10006,$01bd0006,$01c20005
		dc.l	$01d30006,$01e40005,$01ef0005,$00570006
		dc.l	$01f40007,$00370005,$020a0004,$020e0007
		dc.l	$02150006,$021b0007,$02220007,$01cf0005
		dc.l	$01df0006,$02050006,$02290005,$01c60005
		dc.l	$00640006,$022d0007,$02400007,$023a0007
		dc.l	$01e90007,$02460007,$02530005,$025a0003
		dc.l	$004c0001,$02580003,$025d0005,$037a0005
		dc.l	$037a0005,$037a0005,$037a0005,$037a0005
		dc.l	$037a0005,$037a0005,$037a0005,$037a0005
		dc.l	$037a0005,$037a0005,$037a0005,$037a0005
		dc.l	$037a0005,$037a0005,$037a0005,$037a0005
		dc.l	$037a0005,$037a0005,$037a0005,$037a0005
		dc.l	$037a0005,$037a0005,$037a0005,$037a0005
		dc.l	$037a0005,$037a0005,$037a0005,$037a0005
		dc.l	$037a0005,$037a0005,$037a0005,$037a0005
		dc.l	$02620000,$02620001,$01320005,$00460006
		dc.l	$02680006,$008f0007,$02730001,$02740005
		dc.l	$00a50003,$02630007,$02790004,$00bb0006
		dc.l	$012e0005,$00740005,$026c0007,$00950005
		dc.l	$01450004,$02850005,$006f0004,$028a0004
		dc.l	$00990002,$024c0007,$00140006,$02950003
		dc.l	$01ae0003,$00720003,$02980004,$00b60006
		dc.l	$029c0008,$02a40008,$028d0008,$01280005
		dc.l	$00cb0007,$017b0007,$01970007,$01b60007
		dc.l	$01d80007,$01fa0007,$01660006,$02ac0005
		dc.l	$00df0006,$02b50006,$02bf0006,$02c50006
		dc.l	$00b10005,$00db0005,$02b10005,$02bb0005
		dc.l	$016b0006,$013b0007,$01040005,$01000005
		dc.l	$00fc0005,$00f80005,$00ac0005,$00b90005
		dc.l	$018c0005,$02d70007,$02d10007,$02dd0007
		dc.l	$02cb0007,$02e30007,$02ea0006,$003b0006
		dc.l	$02f00006,$02f60006,$02fc0006,$03020006
		dc.l	$03080006,$030e0006,$03140007,$02810005
		dc.l	$031f0005,$03290005,$032e0005,$03330005
		dc.l	$03380005,$033d0005,$03420005,$03470005
		dc.l	$00a70006,$034c0007,$00530005,$01cb0005
		dc.l	$02010005,$027d0005,$031b0005,$007a0005
		dc.l	$03530005,$035e0007,$03650007,$036c0007
		dc.l	$03730007,$02340007,$03240006,$03580007
		dc.l	$037a0005

font13Space:
		dc.w	0007,0004,0005,0007,0006,0006,0007,0005
		dc.w	0004,0006,0006,0006,0006,0006,0005,0006
		dc.w	0006,0006,0006,0006,0007,0007,0006,0006
		dc.w	0006,0006,0004,0005,0006,0006,0005,0006
		dc.w	0006,0007,0007,0006,0007,0007,0007,0006
		dc.w	0007,0006,0006,0007,0007,0007,0007,0006
		dc.w	0007,0006,0007,0006,0007,0007,0007,0007
		dc.w	0007,0007,0006,0004,0006,0005,0006,0007
		dc.w	0006,0006,0007,0006,0006,0006,0006,0006
		dc.w	0007,0006,0006,0007,0006,0007,0007,0006
		dc.w	0007,0006,0006,0006,0006,0007,0007,0007
		dc.w	0007,0007,0006,0005,0004,0005,0006,0006
		dc.w	0006,0006,0006,0006,0006,0006,0006,0006
		dc.w	0006,0006,0006,0006,0006,0006,0006,0006
		dc.w	0006,0006,0006,0006,0006,0006,0006,0006
		dc.w	0006,0006,0006,0006,0006,0006,0006,0006
		dc.w	0007,0004,0006,0007,0007,0007,0004,0006
		dc.w	0005,0007,0006,0007,0006,0006,0007,0006
		dc.w	0006,0006,0006,0006,0004,0007,0007,0006
		dc.w	0005,0005,0006,0006,0007,0007,0007,0006
		dc.w	0007,0007,0007,0007,0007,0007,0007,0006
		dc.w	0007,0007,0007,0007,0006,0006,0006,0006
		dc.w	0007,0007,0006,0006,0006,0006,0006,0006
		dc.w	0006,0007,0007,0007,0007,0007,0007,0007
		dc.w	0006,0006,0006,0006,0006,0006,0007,0006
		dc.w	0006,0006,0006,0006,0006,0006,0006,0006
		dc.w	0007,0007,0006,0006,0006,0006,0006,0006
		dc.w	0006,0007,0007,0007,0007,0007,0007,0007
		dc.w	0006

font13Kern:
		dc.w	0000,0003,0002,0000,0001,0001,0000,0002
		dc.w	0003,0001,0001,0001,0001,0001,0002,0001
		dc.w	0001,0001,0001,0001,0000,0000,0001,0001
		dc.w	0001,0001,0003,0002,0001,0001,0002,0001
		dc.w	0001,0000,0000,0001,0000,0000,0000,0001
		dc.w	0000,0001,0001,0000,0000,0000,0000,0001
		dc.w	0000,0001,0000,0001,0000,0000,0000,0000
		dc.w	0000,0000,0001,0003,0001,0002,0001,0000
		dc.w	0001,0001,0000,0001,0001,0001,0001,0001
		dc.w	0000,0001,0001,0000,0001,0000,0000,0001
		dc.w	0000,0001,0001,0001,0001,0000,0000,0000
		dc.w	0000,0000,0001,0002,0003,0002,0001,0001
		dc.w	0001,0001,0001,0001,0001,0001,0001,0001
		dc.w	0001,0001,0001,0001,0001,0001,0001,0001
		dc.w	0001,0001,0001,0001,0001,0001,0001,0001
		dc.w	0001,0001,0001,0001,0001,0001,0001,0001
		dc.w	0000,0003,0001,0000,0000,0000,0003,0001
		dc.w	0002,0000,0001,0000,0001,0001,0000,0001
		dc.w	0001,0001,0001,0001,0003,0000,0000,0001
		dc.w	0002,0002,0001,0001,0000,0000,0000,0001
		dc.w	0000,0000,0000,0000,0000,0000,0000,0001
		dc.w	0000,0000,0000,0000,0001,0001,0001,0001
		dc.w	0000,0000,0001,0001,0001,0001,0001,0001
		dc.w	0001,0000,0000,0000,0000,0000,0000,0000
		dc.w	0001,0001,0001,0001,0001,0001,0000,0001
		dc.w	0001,0001,0001,0001,0001,0001,0001,0001
		dc.w	0000,0000,0001,0001,0001,0001,0001,0001
		dc.w	0001,0000,0000,0000,0000,0000,0000,0000
		dc.w	0001

font13Data:
		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
		dc.w	$0000,$0000,$0000,$1000,$0004,$0002,$2000,$0052
		dc.w	$1200,$0000,$0000,$0002,$8000,$0000,$0000,$0002
		dc.w	$0000,$0020,$0000,$00a0,$0000,$0000,$0000,$0004
		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
		dc.w	$0000,$0000,$0000,$1080,$2000,$0420,$8100,$0000
		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000

		dc.w	$0000,$07c0,$0002,$0000,$0000,$0800,$0000,$0000
		dc.w	$0000,$0020,$08d5,$0800,$0002,$0004,$1000,$00a5
		dc.w	$2100,$0000,$0000,$0005,$0000,$0000,$0000,$0004
		dc.w	$0000,$8050,$0000,$0140,$0004,$0028,$0000,$000a
		dc.w	$1000,$0000,$0000,$0080,$0000,$0000,$0000,$0782
		dc.w	$8000,$0000,$0000,$290a,$5285,$0811,$4200,$2042
		dc.w	$0a00,$8000,$8010,$8041,$0802,$8000,$8081,$003e

		dc.w	$f7fb,$fa85,$7f77,$3046,$30cc,$6402,$543e,$d000
		dc.w	$fdf7,$dfd0,$1560,$0000,$1380,$1dc0,$073b,$8200
		dc.w	$003e,$7df7,$ee00,$0800,$3b77,$f63f,$f1ff,$7760
		dc.w	$3e77,$7c01,$ff00,$0006,$0008,$0300,$0000,$7c04
		dc.w	$2807,$0700,$0000,$0100,$0000,$0088,$0703,$94b5
		dc.w	$001b,$0064,$0407,$0000,$0000,$0000,$0038,$1085
		dc.w	$1451,$400a,$4c21,$4a22,$14a5,$0028,$4102,$8522

		dc.w	$a28a,$4a85,$488a,$c809,$412a,$a005,$2421,$3000
		dc.w	$28a2,$8810,$1097,$7c00,$708e,$223f,$f8c4,$4677
		dc.w	$7743,$28a2,$3100,$081d,$c4a2,$8b68,$a0c8,$8a4e
		dc.w	$1389,$a2e2,$3100,$01c2,$0000,$0170,$0000,$841c
		dc.w	$0001,$0100,$0000,$0000,$0000,$0050,$0884,$5648
		dc.w	$0024,$909c,$2c28,$ffff,$fffd,$f7df,$7dd0,$0000
		dc.w	$0000,$8000,$0400,$0000,$0000,$0100,$0000,$0022

		dc.w	$94aa,$4a9f,$888a,$33c9,$410a,$aee8,$ffa0,$5048
		dc.w	$28aa,$5008,$2078,$9244,$9142,$2264,$8884,$4a88
		dc.w	$8883,$2894,$5120,$1c0c,$84a2,$8b69,$2148,$8a82
		dc.w	$118a,$a222,$1200,$7843,$ddce,$ef11,$7777,$f584
		dc.w	$3bbd,$311d,$2e5e,$6efb,$efbc,$df50,$337b,$b93b
		dc.w	$b909,$2094,$4448,$9224,$8a28,$a28a,$289e,$f3cf
		dc.w	$3cf3,$c6ce,$e7b9,$cee7,$39cb,$8eef,$366c,$d9a2

		dc.w	$88e0,$468a,$0886,$0c4a,$23e9,$3110,$243c,$9040
		dc.w	$28aa,$f808,$2088,$9129,$1145,$22a4,$911b,$9288
		dc.w	$88f2,$ad94,$5107,$ebea,$8b3e,$f2af,$217c,$8305
		dc.w	$138a,$bc53,$8400,$04a2,$2231,$1128,$8894,$864a
		dc.w	$4445,$410a,$9922,$2451,$4514,$5252,$544a,$b8ac
		dc.w	$47e4,$d764,$a4e8,$1244,$9248,$a28a,$2511,$0820
		dc.w	$8208,$2131,$1446,$3121,$0846,$5345,$1224,$48a2

		dc.w	$94a0,$428a,$0883,$3049,$7489,$3110,$5403,$fffe
		dc.w	$28aa,$2004,$4088,$9092,$13e5,$22a4,$f204,$6288
		dc.w	$888a,$a508,$4f20,$280a,$9022,$8aa9,$2148,$9b85
		dc.w	$1e8a,$a450,$e400,$3ca2,$21d1,$1128,$8f88,$844a
		dc.w	$4445,$810a,$9122,$2455,$4514,$4425,$544b,$2484
		dc.w	$411b,$b70f,$6f98,$13c4,$f3c8,$a28a,$2211,$79e7
		dc.w	$9e79,$e7f1,$f47f,$ff21,$0844,$5545,$1224,$48a2

		dc.w	$9480,$42bf,$088a,$cc48,$8889,$3110,$0402,$0041
		dc.w	$28b6,$f804,$4088,$9129,$122f,$a264,$9404,$7f88
		dc.w	$888a,$6508,$8120,$2be9,$9022,$8aa8,$23c8,$8a4f
		dc.w	$908a,$a2f8,$2900,$45f2,$2031,$117c,$8814,$845f
		dc.w	$4445,$410a,$9122,$2295,$28a4,$4854,$b37a,$a37c
		dc.w	$4001,$57f2,$a228,$9244,$9248,$a28a,$221e,$8a28
		dc.w	$a28a,$2911,$0442,$1021,$0844,$5529,$1224,$48a2

		dc.w	$2280,$4294,$088a,$924a,$8d19,$3110,$0442,$0049
		dc.w	$2d94,$2002,$8088,$9244,$9228,$a204,$88c4,$4288
		dc.w	$888a,$6708,$8244,$1c09,$8022,$8a28,$a248,$8a28
		dc.w	$908c,$a28a,$3100,$4512,$2231,$1144,$88a2,$8451
		dc.w	$4445,$210a,$9122,$238a,$38e4,$5150,$2884,$5904
		dc.w	$47c2,$7804,$f448,$9224,$8a2d,$b6db,$6210,$8a28
		dc.w	$a28a,$2931,$1446,$3121,$0844,$5939,$1224,$48a2

		dc.w	$f7e1,$f6d4,$3f77,$0dfb,$73ea,$aef0,$03bc,$0000
		dc.w	$c714,$7002,$8077,$7c00,$7f7d,$dddf,$ffbb,$8777
		dc.w	$7777,$221c,$9c84,$081c,$9077,$f77f,$fe7f,$773d
		dc.w	$fc77,$73df,$df00,$3bbd,$ddce,$eeee,$f763,$eefb
		dc.w	$bbc7,$1ffa,$f9f9,$d10a,$1047,$bf50,$2703,$9f03
		dc.w	$b800,$1000,$20f7,$7fff,$ffe7,$1c71,$c738,$75d7
		dc.w	$5d75,$d6ce,$e7b9,$ceff,$fffe,$6e10,$e9d3,$a762

		dc.w	$0000,$0000,$0002,$0000,$000a,$a010,$0000,$0000
		dc.w	$0000,$0001,$0000,$0000,$0000,$0000,$0000,$0000
		dc.w	$0000,$0000,$0088,$0800,$0000,$0000,$0000,$0000
		dc.w	$0018,$0000,$0001,$0000,$0000,$0000,$8000,$0000
		dc.w	$0044,$0000,$0000,$0100,$0044,$0050,$2000,$1000
		dc.w	$1000,$0000,$0002,$0000,$0000,$0000,$0000,$0000
		dc.w	$0000,$0000,$0400,$0000,$0000,$1010,$0000,$0022

		dc.w	$0000,$0000,$0000,$0000,$000c,$6010,$0000,$0000
		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
		dc.w	$0000,$0000,$0070,$0000,$0000,$0000,$0000,$0000
		dc.w	$0000,$0000,$0000,$8000,$0000,$0000,$8000,$0000
		dc.w	$0044,$0000,$0000,$0100,$0044,$0088,$2000,$1000
		dc.w	$0800,$0000,$0001,$0000,$0000,$0000,$0000,$0000
		dc.w	$0000,$0000,$0400,$0000,$0000,$0010,$0000,$003e

		dc.w	$0000,$0000,$0000,$0000,$0000,$00e0,$0000,$0000
		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
		dc.w	$0000,$0000,$00ff,$0000,$0000,$0001,$c000,$0000
		dc.w	$00f8,$0000,$0000,$0600,$0184,$0000,$2000,$0000
		dc.w	$3000,$0000,$0006,$0000,$0000,$0000,$0000,$0000
		dc.w	$0000,$0000,$0e00,$0000,$0000,$0060,$0000,$0000

FONT13MODULO	EQU	112

	END
