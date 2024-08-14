	TITLE	PC DOS 3.3 Keyboard Definition File

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; PC DOS 3.3 - NLS Support - Keyboard Definition File
;; (c) Copyright IBM Corp 198?,...
;;
;; This file contains the eof marker for the entire table
;; and the keyboard.sys copyright information
;;
;; Linkage Instructions:
;;	Refer to KDF.ASM.
;;
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
				       ;;
CODE	SEGMENT PUBLIC 'CODE'          ;;
	ASSUME CS:CODE,DS:CODE	       ;;
				       ;;
				       ;;
				       ;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Copyright statement
;;	DB 'KEYBOARD.SYS Version 3.30 (C) Copyright IBM Corp. 1986,1987',13,10  ;;
;;	DB 'Authors : Bill Devlin, Nick Savage, Mike Saunders, et al..',13,10
;;	DB 'Development: Toronto,Boca Raton,Basingstoke',13,10
	DB 'KEYBOARD.SYS Version 3.30                                  ',13,10  ;;
	DB '    Copyright (c) 1987                                    ',13,10
	DB '                                           ',13,10
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	DB  1AH 		       ;; EOF
				       ;;
CODE	ENDS			       ;;
	END			       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
