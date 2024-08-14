/*
*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************
*/

/*
		Phoneme code equates
*/

#define	PC_BL		0
#define	PC_PE		PC_BL+1
#define	PC_QM		PC_PE+1
#define	PC_CO		PC_QM+1
#define	PC_DA		PC_CO+1
#define	PC_LP		PC_DA+1
#define	PC_RP		PC_LP+1
#define	PC_RES3 	PC_RP+1
#define	PC_RES4 	PC_RES3+1
#define	PC_IY		PC_RES4+1
#define	PC_IYP		PC_IY+1
#define	PC_IH		PC_IYP+1
#define PC_IHP		PC_IH+1
#define	PC_EH		PC_IHP+1
#define PC_EHP		PC_EH+1
#define	PC_AE		PC_EHP+1
#define PC_AEP		PC_AE+1
#define	PC_AA		PC_AEP+1
#define	PC_AH		PC_AA+1
#define	PC_AO		PC_AH+1
#define PC_AOP		PC_AO+1
#define	PC_UH		PC_AOP+1
#define PC_UHP		PC_UH+1
#define	PC_AX		PC_UHP+1
#define	PC_IX		PC_AX+1
#define	PC_ER		PC_IX+1
#define PC_ERP		PC_ER+1
#define	PC_QX		PC_ERP+1
#define	PC_AXP		PC_QX+1
#define	PC_RX		PC_AXP+1
#define	PC_LX		PC_RX+1
#define	PC_EY		PC_LX+1
#define	PC_EYP  	PC_EY+1
#define	PC_AY		PC_EYP+1
#define	PC_AYP  	PC_AY+1
#define	PC_OY		PC_AYP+1
#define	PC_OYP  	PC_OY+1
#define	PC_AW		PC_OYP+1
#define	PC_AWP  	PC_AW+1
#define	PC_OW		PC_AWP+1
#define	PC_OWP  	PC_OW+1
#define	PC_UW		PC_OWP+1
#define	PC_UWP  	PC_UW+1
#define PC_IXR		PC_UWP+1
#define PC_IXRP		PC_IXR+1
#define PC_EXR		PC_IXRP+1
#define PC_EXRP		PC_EXR+1
#define PC_AXR		PC_EXRP+1
#define PC_AXRP		PC_AXR+1
#define PC_OXR		PC_AXRP+1
#define PC_OXRP		PC_OXR+1
#define PC_UXR		PC_OXRP+1
#define PC_UXRP		PC_UXR+1
#define	PC_WH		PC_UXRP+1
#define	PC_RR		PC_WH+1
#define	PC_L 		PC_RR+1
#define	PC_W 		PC_L+1
#define	PC_Y	 	PC_W+1
#define PC_YU		PC_Y+1
#define PC_YUP		PC_YU+1
#define	PC_M 		PC_YUP+1
#define	PC_N 		PC_M+1
#define	PC_NX		PC_N+1
#define	PC_DX		PC_NX+1
#define	PC_Q 		PC_DX+1
#define	PC_S 		PC_Q+1
#define	PC_SH		PC_S+1
#define	PC_F 		PC_SH+1
#define	PC_TH		PC_F+1
#define	PC_Z 		PC_TH+1
#define	PC_ZH		PC_Z+1
#define	PC_V 		PC_ZH+1
#define	PC_DH		PC_V+1
#define PC_CH		PC_DH+1
#define	PC_J 		PC_CH+1
#define	PC_HH		PC_J+1
#define PC_HX		PC_HH+1
#define	PC_B 		PC_HX+1
#define	PC_D 		PC_B+1
#define	PC_G		PC_D+1
#define	PC_GX		PC_G+1
#define	PC_P 		PC_GX+1
#define	PC_T 		PC_P+1
#define	PC_TQ		PC_T+1
#define	PC_K 		PC_TQ+1
#define	PC_KX		PC_K+1
#define	PC_UL		PC_KX+1
#define	PC_UM		PC_UL+1
#define	PC_UN		PC_UM+1
#define	PC_IL		PC_UN+1
#define	PC_IM		PC_IL+1
#define	PC_IN		PC_IM+1
#define	PC_OH		PC_IN+1
#define	PC_HC		PC_OH+1
#define NUM_PCS		PC_HC+1
