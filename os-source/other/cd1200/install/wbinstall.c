; $Date: 93/08/13 10:40:30 $ $Revision: 40.17 $
; Script to install Release 3.1 Workbench

(complete 0)

;=============================================================================
; English strings

(if (= @language "english")
(
(set default_lang 4)

(set #bad-kick
(cat "You must be using Kickstart 3.0 to install Workbench�3.1"
))

(set #introduction
(cat "\n\nThis program lets you install Release 3.1 of the Amiga Operating "
     "System on a hard drive. It can be used to upgrade an older release, "
     "to install from scratch, or to control which languages are available "
     "on a hard drive with Release 3.1 already installed."
))

(set #ask-function
(cat "\n\nSelect \"Install Release 3.1\" to do a complete installation of "
     "Release 3.1, or select \"Update Languages\" to modify which languages "
     "are available on a hard drive with Release 3.1 already installed."
))

(set #ask-function-1
(cat "Install Release 3.1"
))

(set #ask-function-2
(cat "Update Languages"
))

(set #ask-function-help
(cat "\nYou must choose which operation you would like to "
     "perform.\n\n"
     "Selecting the \"Install Release 3.1\" gadget lets you perform a complete "
     "installation of the Release 3.1 Workbench software. This option should "
     "be chosen when upgrading a system from prior releases, or when "
     "installing Release 3.1 on a new hard drive unit.\n\n"
     "The Amiga can be operated in many different "
     "languages. Several files must be copied to your "
     "hard drive for each language supported. Selecting the "
     "\"Update Languages\" gadget lets you control which "
     "languages are installed and available for use on your hard drive. "
     "This option should only be chosen when Release 3.1 has already "
     "been installed.\n\n"
))

(set #confirm-target
(cat "\nRelease 3.1 will now be installed on your hard drive. Any "
     "previous release already present will be replaced. If applicable, "
     "certain files from previous releases will be preserved, including "
     "the Startup-Sequence.\n\n"
     "Do you want Release 3.1 installed in the \"%s\" partition?"
))

(set #confirm-target-help
(cat "\nThe installer has determined that Release 3.1 should probably be "
     "installed in the partition named \"%s\". Proceeding with the "
     "installation will cause any Workbench files in this partition to be "
     "replaced by newer files.\n\n"
     "Some files will first be copied to a "
     "safe place before their replacements are installed, including the "
     "Startup-Sequence file."
))

(set #confirm-target-lang
(cat "\n\nThe available Release 3.1 languages on your hard drive will now be "
     "updated. Your Release 3.1 Workbench files appear "
     "to be located in the \"%s\" "
     "partition.\n\n"
     "Is this correct?"
))

(set #confirm-target-lang-help
(cat "\nThe installer has determined that your current copy of "
     "Workbench is in the partition named \"%s\". Proceeding with the "
     "installation will cause the Workbench language files to be copied there. "
))

(set #which-disk-lang
(cat "In which partition should the Release�3 language files be installed?"
))

(set #which-disk-lang-help
(cat "\nThis section lets you choose on which hard drive partition the "
     "Release 3.1 language files should be installed. These should go "
     "in the same location as the Release 3.1 Workbench files.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nThe update of Release 3.1 languages is now complete.\n\n"
     "To use these new languages, you must reboot your Amiga. After rebooting, "
     "you must run the \"Locale\" preferences editor and pick the "
     "language you wish to use.\n\n"
     "Remove the \"Amiga Install\" disk from the floppy drive, "
     "and select the \"Proceed\" gadget to reboot your Amiga."
))

(set #which-disk
(cat "In which partition should Release�3 be installed?"
))

(set #which-disk-help
(cat "\nThis section lets you choose on which hard drive partition the "
     "Release 3.1 files will be installed. These should normally go "
     "in the same location as the current Release 1.3, Release 2, or "
     "Release 2.1 files.\n\n"
     @askdir-help
))

(set #old-name
(cat "OLD"
))

(set #move-old
(cat "\n\nShould the following files be moved to the \"%s\" drawer?\n\n%s"
))

(set #move-old-help
(cat "\nInstalling Release 3.1 will replace most current Workbench files, "
     "including the Startup-Sequence file. "
     "In order to preserve any changes you have previously made to these "
     "files, the installation process will copy them to the \"%s\" drawer "
     "where they can later be retrieved."
))

(set #move-old-1
(cat "Move"
))

(set #move-old-2
(cat "Skip"
))

(set #checking-target
(cat "\n\nChecking \"%s\" for obsolete files..."
))

(set #delete-old
(cat "\n\nMany files used by the system under earlier releases "
     "are no longer needed under Release 3.1 "
     "These files may now be deleted from your system.\n\n"
     "Do you wish to be asked permission before deleting each file, or "
     "should they be deleted automatically?"
))

(set #delete-old-help
(cat "\nEarlier releases have numerous files that are not "
     "useful under Release 3.1. These files should be deleted to "
     "avoid clutter and save space on your hard drive.\n\n"
     "The \"Delete Automatically\" gadget will cause all obsolete system files "
     "to be deleted without asking for your permission on any of the files.\n\n"
     "The \"Ask Before Deleting\" gadget will force the installation "
     "process to ask for your permission before deleting each file. This "
     "enables you to preserve specific files that you do not wish to "
     "delete.\n\n"
))

(set #delete-old-1
(cat "Delete Automatically"
))

(set #delete-old-2
(cat "Ask Before Deleting"
))

(set #deleting-obsolete
(cat "\n\nDeleting obsolete files..."
))

(set #confirm-delete
(cat "\n\nShould the file \"%s\" be deleted?"
))

(set #confirm-delete-help
(cat "\nThe file \"%s\" is no longer "
     "useful under Release 3.1. You can delete the file by selecting "
     "the \"Yes\" gadget, or preserve the file by selecting the \"No\" "
     "gadget."
))

(set #confirm-delete-reason-1
(cat "This file is now incorporated in the Release 3.1 ROM."
))

(set #confirm-delete-reason-2
(cat "The functionality of this program has been integrated into other "
     "Workbench programs."
))

(set #confirm-delete-reason-3
(cat "This program was moved to a different location and will be updated "
     "there."
))

(set #confirm-delete-reason-4
(cat "This program is obsolete and should not be used with Release 3.1"
))

(set #confirm-delete-1
(cat "Yes"
))

(set #confirm-delete-2
(cat "No"
))

(set #preparing-icons
(cat "\n\nPreparing icons..."
))

(set #keeping-old
(cat "\n\nMaking backup copies of important files..."
))

(set #ask-workbench
(cat "\n\nPlease insert the \"Amiga Workbench\" disk Version�3.1 in any floppy drive."
))

(set #ask-extras
(cat "\n\nPlease insert the \"Amiga Extras\" disk Version�3.1 in any floppy drive."
))

(set #ask-storage
(cat "\n\nPlease insert the \"Amiga Storage\" disk Version�3.1 in any floppy drive."
))

(set #which-printer
(cat "Which printer drivers should be installed?"
))

(set #which-printer-help
(cat "\nThe Amiga can control many different printers. "
     "Printer drivers are files that let the Amiga adapt to a given "
     "printer. There must be a printer driver copied to your "
     "hard drive for each printer supported.\n\n"
     "To reduce the amount of space consumed by the "
     "printer driver files, you can select only the "
     "drivers that are useful to you.\n\n"
     "Check the boxes of the printers you wish "
     "to have available on your system.\n\n"
     @askoptions-help
))

(set #which-keymap
(cat "Which keymaps should be installed?"
))

(set #which-keymap-help
(cat "\nThe Amiga can be operated with many different types of keyboards. "
     "A Keymap file describes your keyboard to the Amiga so it can interact "
     "with it. Pick the type of keyboard you have from the list of available "
     "choices.\n\n"
     "Check the boxes of the keymap files you wish "
     "to have available on your system.\n\n"
     @askoptions-help
))

(set #which-language
(cat "\nWhich languages should be installed?"
))

(set #which-language-help
(cat "\nThe Amiga can be operated in many different "
     "languages. Several files must be copied to your "
     "hard drive for each language supported.\n\n"
     "To reduce the amount of space consumed by the "
     "language files, you can select only the "
     "files of specific languages to be copied.\n\n"
     "Check the boxes of the languages you wish "
     "to have available on your system.\n\n"
     @askoptions-help
))

(set #ask-locale
(cat "\n\nPlease insert the \"Amiga Locale\" disk Version�3.1 in any floppy drive."
))

(set #no-font-space
(cat "\nThere is currently not enough room on \"%s\" to fit the standard Amiga "
     "font set.\n\n"
     "The Amiga font set will not be copied.\n\n"
     "You can copy the fonts at a later time by making room on \"%s\" and "
     "dragging the icon of the "
     "\"Amiga Fonts\" disk Version 3.1 on top of the \"%s\" icon."
))

(set #ask-fonts
(cat "\n\nPlease insert the \"Amiga Fonts\" disk Version�3.1 in any floppy drive."
))

(set #ask-install
(cat "\n\nPlease insert the \"Amiga Install\" disk Version�3.1 in any floppy drive."
))

(set #reorganizing
(cat "\n\nAdjusting files to match your previous configuration..."
))

(set #a2090
(cat "If you are using an A2090 hard disk controller with a SCSI "
     "hard drive and are booting from floppy, you should install "
     "the file Startup-A2090SCSI from the Update drawer on this disk "
     "as your Startup-Sequence on the boot floppy disk. "
     "If you are using an A2090 hard disk controller with an ST-506 "
     "hard drive and are booting from floppy, you should install "
     "the file Startup-A2090ST506 from the Update drawer on this disk "
     "as your Startup-Sequence on the boot floppy disk."
))

(set #reboot
(cat "\nThe installation of Release 3.1 is now complete.\n\n"
     "To enable Release 3.1, you must reboot your Amiga.\n\n"
     "Remove the \"Amiga Install\" disk from the floppy drive, "
     "and select the \"Proceed\" gadget to reboot your Amiga."
))

))


;=============================================================================
; Portugues strings

(if (= @language "portugu�s")
(
(set default_lang 256)

(set #bad-kick
(cat "Percisa de estar a usar o Kickstart 3.0 para instalar o Workbench�3.1"
))

(set #introduction
(cat "\n\nEste programa permite-lhe instalar a Vers�o 3.1 do Sistema Operativo "
     "do Amiga no seu disco r�gido. Pode ser usado para fazer a substitui��o "
     "de uma vers�o anterior, para instalar do zero ou para controlar as "
     "linguagens que est�o � sua disposi��o num disco r�gido com a Vers�o 3.1 "     "j� instalada."
     "j� instalada."
))

(set #ask-function
(cat "\n\nSeleccione \"Instalar Vers�o 3.1\" para fazer uma instala��o da "
     "Vers�o 3.1, ou seleccione \"Mudar Linguagens\" para modificar as "
     "linguagens que est�o � sua disposi��o num disco r�gido com a Vers�o 3.1 "
     "j� instalada."
))

(set #ask-function-1
(cat "Instalar Vers�o 3.1"
))

(set #ask-function-2
(cat "Mudar Linguagens"
))

(set #ask-function-help
(cat "\nTer� que escolher qual � a opera��o que quer realizar.\n\n"
     "Seleccionando o bot�o \"Instalar Vers�o 3.1\" vai fazer uma instala��o "
     "completa da Vers�o 3.1 do software do Workbench. Esta op��o ter� que ser "
     "escolhida quando se est� a fazer a substitui��o de vers�es anteriores, "
     "ou se est� a instalar a Vers�o 3.1 a partir do zero.\n\n"
     "O Amiga pode ser operado em muitas linguagens diferentes. "
     "Alguns ficheiros t�m que ser copiados para o seu disco r�gido, por cada "
     "uma das linguagens a serem suportadas pelo seu sistema. A selec��o do "
     "bot�o \"Mudar Linguagens\" vai permitir-lhe controlar quais as "
     "linguagens que est�o instaladas e prontas a serem usadas, no seu"
     "disco r�gido. "
     "Esta op��o deve ser usada apenas ap�s a instala��o da Vers�o 3.\n\n"
))

(set #confirm-target
(cat "\n\nEste programa vai actualizar o seu Hard-Disk para o Workbench�3.1. "
     "Os ficheiros do Workbench anterior ser�o apagados, embora a sua "
     "Startup-Sequence sejam preservadas. "
     "Os ficheiros do seu Workbench parecem estar na gaveta \"%s\".\n\n"
     "Quer actualizar os ficheiros na gaveta \"%s\"?"
))

(set #confirm-target-help
(cat "\nO programa de instala��o determinou que a sua c�pia actual do "
     "Workbench est� na gaveta chamada \"%s\". Prosseguir com a instal��o "
     "vai implicar a substitui��o dos ficheiros existentes por outros "
     "novos.\n\nAlguns destes ser�o copiados para um lugar seguro antes "
     "de serem substituidos. Eles s�o "
     "Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nAs linguagens da Vers�o 3.1 dispon�veis seram mudadas agora."
     "Os ficheiros do seu Workbench Vers�o 3.1 parecem estar localizados "
     "na parti��o \"%s\".\n\n"
     "Est� correcto?"
))

(set #confirm-target-lang-help
(cat "\nO programa de instala��o determinou que a sua c�pia corrente "
     "do Workbench se encontra na parti��o chamada \"%s\". A continua��o "
     "do processo de instala��o vai fazer com que os ficheiros de linguagem "
     "do Workbench sejam copiados para a�. "
))

(set #which-disk-lang
(cat "Para que parti��o dever�o ser copiados os ficheiros de linguagem da Vers�o 3?"
))

(set #which-disk-lang-help
(cat "\nEsta parte permite-lhe escolher em que parti��o dever�o ser instalados "
     "os ficheiros de linguagem da Vers�o 3.1. Estes dever�o ir para o mesmo "
     "s�tio onde se encontram os ficheiros da Vers�o 3.1 do Workbench.\n\n"
))

(set #reboot-lang
(cat "\nO processo de mudan�a das linguagens da Vers�o 3.1 est� completo.\n\n"
     "Para poder usar as novas lingagens, ter� que reinicializar o seu Amiga. "
     "Depois do arranque do computador ter� que executar o programa de "
     "prefer�ncias \"Locale\" e escolher a linguagem que quizer utilizar.\n\n"
     "Retire a diskette \"Amiga Install\" da drive e seleccione \"Continuar\""
     "Para reinicializar o seu Amiga."
))

(set #which-disk
(cat "Em que disco ou gaveta se vai instalar o Workbench�3.1?"
))

(set #which-disk-help
(cat "\nEsta sec��o permite-lhe escolher em que gaveta ou disco os "
     "ficheiros do Workbench�3.1 ser�o instalados. Este normalmente ir�o "
     "para o mesmo lugar dos ficheiros do  Workbench�1.3 ou Workbench�2.0.\n\n"
     @askdir-help
))

(set #old-name
(cat "OLD"
))

(set #move-old
(cat "\n\nPode-se proseguir com a mudan�a dos ficheiros para a gaveta \"%s\"?\n\n%s"
))

(set #move-old-help
(cat "\nA instala��o do Workbench�3.1 vai causar a substitui��o de ficheiros, "
     "incluindo os ficheiros: Startup-Sequence e Shell-Startup. "
     "Para conservar as mudan�as feitas a estes ficheiros, o processo de "
     "instala��o vai mud�-los para a gaveta \"%s\" "
     "de onde poderam ser retirados mais tarde."
))

(set #move-old-1
(cat "Move"
))

(set #move-old-2
(cat "Passar"
))

(set #checking-target
(cat "\n\nProcurando ficheiros antigos em \"%s\"..."
))

(set #delete-old
(cat "\n\nMuitos dos ficheiros usados pelo sistema no Workbench�1.3 "
     "e Workbench�2.0 j� n�o ser�o precisos pelo Workbench�3.1. "
     "Estes ficheiros poderam agora ser apagados.\n\n"
     "Quer que o sistema lhe pe�a autoriza��o para apagar cada ficheiro "
     "ou poder-se-� proceder a essa ac��o autom�ticamente?"
))

(set #delete-old-help
(cat "\nO Workbench�1.3 e o Workbench�2.0 t�m numerosos ficheiros que n�o "
     "s�o �teis ao Workbench�3.1. Estes t�m de ser apagados para evitar "
     "conflitos e n�o ocuparem espa�o no seu disco r�gido.\n\n"
     "O bot�o \"Confirmar Apagamento\" vai for�ar o processo de instal��o "
     "a pedir a sua confirma��o antes de apagar cada ficheiro. Isto "
     "permite-lhe conservar algum ficheiro que n�o deseja apagar.\n\n"
     "O bot�o \"Apagamento Autom�tico\" vai fazer com que todos os ficheiros "
     "antigos sejam apagados sem que para isso seja pedida a sua confirma��o."
))

(set #delete-old-1
(cat "Apagamento Autom�tico"
))

(set #delete-old-2
(cat "Confirmar Apagamento"
))

(set #deleting-obsolete
(cat "\n\nApagando os ficheiros antigos..."
))

(set #confirm-delete
(cat "\n\nO ficheiro \"%s\" pode ser apagado?"
))

(set #confirm-delete-help
(cat "\nO programa de instala��o determinou que o ficheiro \"%s\" j� n�o "
     "� util para o Workbench 3.1. Pode apag�-lo selecionando o bot�o "
     "\"Sim\", se quizer conserv�-lo escolha o bot�o \"N�o\". "
))

(set #confirm-delete-reason-1
(cat "Este ficheiro foi incorporado na ROM Vers�o 3.1"
))

(set #confirm-delete-reason-2
(cat "As fun��es deste programa foram integradas em outros programas do "
     "Workbench."
))

(set #confirm-delete-reason-3
(cat "Este programa foi movido para outro s�tio e ser� mudado a�."
))

(set #confirm-delete-reason-4
(cat "Este programa est� desactualizado e por isso n�o dever� ser "
     "utilizado com a Vers�o 3.1"
))

(set #confirm-delete-1
(cat "Sim"
))

(set #confirm-delete-2
(cat "N�o"
))

(set #preparing-icons
(cat "\n\nPreparando os icones..."
))

(set #keeping-old
(cat "\n\nFazendo c�pias dos ficheiros mais importantes..."
))

(set #ask-workbench
(cat "\n\nInsira por favor a diskette \"Amiga Workbench\" Vers�o�3.1 em qualquer drive."
))

(set #ask-extras
(cat "\n\nInsira por favor a diskette \"Amiga Extras\" Vers�o�3.1 em qualquer drive."
))

(set #ask-storage
(cat "\n\nInsira por favor a diskette \"Amiga Storage\" Vers�o�3.1 em qualquer drive."
))

(set #which-printer
(cat "Que Programas de Controlo de Impressora devem ser instalados? "
))

(set #which-printer-help
(cat "\nO Amiga pode controlar muitas impressoras diferentes. "
     "Os Ficheiros de Controlo de Impressora s�o ficheiros que "
     "fazem com que o Amiga se adapte a uma determinada impressora. "
     "Tem que se copiar um destes ficheiros para o disco r�gido "
     "por cada impressora que queira que o Amiga suporte.\n\n"
     "Para reduzir o consumo de espa�o por estes ficheiros "
     "poder� apenas escolher aqueles que lhe ser�o �teis.\n\n"
     "Para isto basta apenas marcar as caixas com os nomes "
     "das impressoras para que estas sejam instaladas.\n\n"
     @askoptions-help
))

(set #which-keymap
(cat "Que Mapa de Teclado deve ser instalado?"
))

(set #which-keymap-help
(cat "\nO Amiga pode ser operado com muitos tipos diferentes de teclados. "
     "Um ficheiro de Mapa de Teclado descreve o seu teclado espec�fico ao "
     "Amiga para que assim possa interagir com ele. Escolha o seu tipo de "
     "teclado da lista.\n\n"
     "Marque as caixas correspondentes aos Mapas de Teclado que desejar "
     "ter � sua disposi��o no sistema.\n\n"
))

(set #which-language
(cat "\nQue linguagens dever�o ser instaladas?"
))

(set #which-language-help
(cat "\nO Amiga pode ser operado em muitas linguagens "
     "diferentes. Alguns ficheiros t�m de ser copiados "
     "para o seu disco r�gido por cada linguagem suportada.\n\n"
     "Para reduzir o consumo de espa�o no disco pelas linguagens "
     "poder� apenas escolher aquelas que lhe ser�o �teis.\n\n"
     "Para isto basta apenas marcar as caixas com os nomes das "
     "linguagens para que estas sejam instaladas.\n\n"
     @askoptions-help
))

(set #ask-locale
(cat "\n\nInsira por favor a diskette \"Amiga Locale\" Vers�o�3.1 em qualquer drive."
))

(set #no-font-space
(cat "\nNeste momento n�o h� espa�o no disco \"%s\" para p�r as fontes de "
     "texto standard do Amiga.\n\n"
     "As fontes de texto do Amiga n�o ser�o copiadas.\n\n"
     "Poder� fazer isto mais tarde puxando o icone da diskette  "
     "\"Amiga Fonts\" Vers�o 3.1 para cima do icone do disco \"%s\"."
))

(set #ask-fonts
(cat "\n\nInsira por favor a diskette \"Amiga Fonts\" Vers�o�3.1 em qualquer drive."
))

(set #ask-install
(cat "\n\nInsira por favor a diskette \"Amiga Install\" Vers�o�3.1 em qualquer drive."
))

(set #reorganizing
(cat "\n\nModificando os ficheiros para ficarem de acordo com a sua "
     "consfigura��o anterior..."
))

(set #a2090
(cat "Se est� a usar uma controladora A2090 com um disco r�gido SCSI "
     "e est� a arrancar a partir de diskette, dever� instalar o ficheiro "
     "Startup-A2090SCSI, que se encontra na gaveta Update desta diskette, "
     "como a sua Startup-Sequence na sua diskette de arranque. "
     "Se est� a usar uma controladora A2090 com um disco r�gido ST-506"
     "e est� a arrancar a partir de diskette, dever� instalar o ficheiro "
     "Startup-A2090ST-506, que se encontra na gaveta Update desta diskette, "
     "como a sua Startup-Sequence na sua diskette de arranque. "
))

(set #reboot
(cat "\nO processo de instala��o da Vers�o 3.1 est� completo.\n\n"
     "Para a poder usar ter� que reinicializar o seu Amiga.\n\n"
     "Retire a diskette \"Amiga Install\" da drive e seleccione "
     "o bot�o \"Continuar\" para reinicializar o seu Amiga."
))

))


;=============================================================================
; Spanish strings

(if (= @language "espa�ol")
(
(set default_lang 8)

(set #bad-kick
(cat "Necesita usar el Kickstart 3.0 para instalar el Workbench�3.1"
))

(set #introduction
(cat "\n\nEste programa le permite instalar la Versi�n 3.1 del Sist. Operativo "
     "de Amiga en el disco duro. Se puede utilizar para actualizar su "
     "sistema, instalar por primera vez o  controlar qu� idiomas hay "
     "disponibles en el disco duro con la Versi�n 3.1 ya instalada."
))

(set #ask-function
(cat "\n\nElija \"Instalar Versi�n 3.1\" para instalar por completo la "
     "Versi�n 3.1, o \"Actualizar Idiomas\" para modificar los idiomas "
     "disponibles en el disco duro con la Versi�n 3.1 ya instalada."
))

(set #ask-function-1
(cat "Instalar Versi�n 3.1"
))

(set #ask-function-2
(cat "Actualizar Idiomas"
))

(set #ask-function-help
(cat "\nDebe elegir la operaci�n que desea realizar.\n\n"
     "Si elige \"Instalar Versi�n 3.1\", llevar� a cabo la instalaci�n "
     "completa del Workbench Versi�n 3.1. Debe utilizar esta opci�n al "
     "actualizar la versi�n de su sistema, o cuando tenga que cargar "
     "la Versi�n 3.1 en un disco duro nuevo.\n\n"
     "Amiga se puede utilizar en diferentes idiomas. Hay que copiar "
     "algunos ficheros al disco duro para cada idioma. Si elige la "
     "opci�n \"Actualizar Idiomas\" podr� controlar qu� idiomas est�n "
     "instalados y disponibles en el disco duro. Esta opci�n se debe "
     "usar cuando la Versi�n 3.1 ya ha sido instalada.\n\n"
))

(set #confirm-target
(cat "\nse va a instalar la Versi�n 3.1 en el disco duro. Se borrar� "
     "la versi�n anterior del sistema operativo del disco duro."
     "Algunos ficheros de la versi�n anterior, no se borrar�n, como "
     "el fichero Startup-Sequence.\n\n"
     "�Quiere instalar la Versi�n 3.1 en la partici�n \"%s\"?"
))

(set #confirm-target-help
(cat "\nEl instalador ha determinado que la Versi�n 3.1 se instalar� "
     "en la partici�n \"%s\". El continuar con la instalaci�n har� "
     "que la mayor�a de los ficheros de este directorio se reemplacen "
     "por los nuevos.\n\nAlgunos ficheros se copiar�n a un lugar seguro "
     "antes de reemplazarlos, incluyendo el Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nLos idiomas disponibles de la Versi�n 3.1 en el disco duro ya"
     "se han actualizado. Los ficheros del Workbench 3.1 parece que est�n "
     "en la partici�n \"%s\" \n\n"
     "� Es correcto ?"
))

(set #confirm-target-lang-help
(cat "\nSu copia actual del Workbench est� en la partici�n \"%s\". "
     "Si contin�a con la instalaci�n, los ficheros de idiomas del "
     "Workbench se copiar�n a dicha partici�n. "
))

(set #which-disk-lang
(cat "�En qu� partici�n deber�a instalar los idiomas de la Versi�n 3?"
))

(set #which-disk-lang-help
(cat "\nEsta secci�n le permite elegir la partici�n del disco duro "
     "donde instalar los ficheros de idiomas de la Versi�n 3.1. Deben "
     "estar en el mismo lugar ue los ficheros del Workbench 3.1.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "Para utilizar los nuevos idiomas, debe arrancar su Amiga. Despu�s, "
     "tiene que ejecutar el editor de preferencias \"Locale\" y elegir "
     "el idioma que prefiere utilizar.\n\n"
     "Retire el disco \"Amiga Install\" de la unidad y seleccione "
     "\"Seguir\" para reinicializar el Amiga."
))

(set #which-disk
(cat "�En qu� unidad o directorio quiere instalar el Workbench�3.1?"
))

(set #which-disk-help
(cat "\nEsta secci�n le permite elegir en qu� unidad o directorio quiere "
     "instalar los ficheros del Workbench�3.1. Normalmente ser� el mismo "
     "lugar donde tiene actualmente su Workbench�1.3 o Workbench�2.0.\n\n"
     @askdir-help
))

(set #old-name
(cat "ANTERIORES"
))

(set #move-old
(cat "\n\n�Pueden ser movidos estos ficheros al caj�n \"%s\"?\n\n%s"
))

(set #move-old-help
(cat "\nLa instalaci�n del Workbench�3.1 reemplazar� la mayor parte de los "
     "ficheros, incluido el Startup-Sequence. "
     "Para conservar los cambios que haya hecho a estos ficheros antes, el "
     "proceso de instalaci�n los copiar� al caj�n \"%s\" de donde los podr� "
     "recuperar m�s tarde."
))

(set #move-old-1
(cat "Mover"
))

(set #move-old-2
(cat "Saltar"
))

(set #checking-target
(cat "\n\nComprobando ficheros antiguos en \"%s\"..."
))

(set #delete-old
(cat "\n\nAlgunos de los ficheros utilizados en versiones anteriores "
     "no son necesarios en el Workbench�3.1. "
     "Estos ficheros se pueden borrar de su sistema.\n\n"
     "�Quiere confirmaci�n individual antes de borrar cada fichero o que "
     "se borren todos autom�ticamente?"
))

(set #delete-old-help
(cat "\nEl Workbench�1.3 y el Workbench�2.0 tiene algunos ficheros que no "
     "son necesarios en el  Workbench�3.1. Estos ficheros tiene que ser "
     "borrados para salvar espacio en el disco duro.\n\n"
     "El gadget \"Preguntar antes de borrar\" forzar� a que el proceso de "
     "instalaci�n pregunte antes de borrar cada fichero. Esto permite que "
     "guarde determinados ficheros que no quiere borrar.\n\n"
     "El gadget \"Borrado autom�tico\" har� que se borren los ficheros "
     "del sistema antiguo sin pedir ninguna confirmaci�n."
))

(set #delete-old-1
(cat "Borrado autom�tico"
))

(set #delete-old-2
(cat "Preguntar antes de borrar"
))

(set #deleting-obsolete
(cat "\n\nBorrando ficheros antiguos"
))

(set #confirm-delete
(cat "\n\n�Borrar el fichero \"%s\"?"
))

(set #confirm-delete-help
(cat "\nEl instalador ha determinado que el fichero \"%s\" no es v�lido "
     "en el Workbench 3.1. Puede borrar el fichero si pulsa en el gadget "
     "\"S�\" o conservarlo si elige el gadget \"No\"."
))

(set #confirm-delete-reason-1
(cat "Ahora, este fichero est� inclu�do en ROM."
))

(set #confirm-delete-reason-2
(cat "La utilidad de este programa est� integrada en otros programas "
     "del Workbench."
))

(set #confirm-delete-reason-3
(cat "Este prgrama se a transaldado a otro lugar y se actualizar� all�."
))

(set #confirm-delete-reason-4
(cat "Este programa es antiguo y no deber�a usarse con la Versi�n 3.1"
))

(set #confirm-delete-1
(cat "S�"
))

(set #confirm-delete-2
(cat "No"
))

(set #preparing-icons
(cat "\n\nPreparando iconos..."
))

(set #keeping-old
(cat "\n\nHaciendo copia de seguridad de ficheros importantes..."
))

(set #ask-workbench
(cat "\n\nPor favor, inserte el disco \"Amiga Workbench\" Versi�n�3.1 en cualquier unidad."
))

(set #ask-extras
(cat "\n\nPor favor, inserte el disco \"Amiga Extras\" Versi�n�3.1 en cualquier unidad."
))

(set #ask-storage
(cat "\n\nPor favor, inserte el disco \"Amiga Storage\" Versi�n�3.1 en cualquier unidad."
))

(set #which-printer
(cat "�Qu� manejadores de impresora tienen que instalarse?"
))

(set #which-printer-help
(cat "\nAmiga puede controlar muchos tipos de impresoras. "
     "Los manejadores (drivers) de impresora son ficheros que permiten "
     "adaptar el Amiga a su impresora. Debe haber manejador de impresora "
     "en el disco duro por cada impresora soportada.\n\n"
     "Para reducir el espacio usado en el disco duro por los manejadores "
     "de impresora, puede seleccionar s�lo el que necesite.\n\n"
     "S�lo tiene que pulsar en el modelo de impresora que tenga conectada "
     "a su sistema.\n\n"
     @askoptions-help
))

(set #which-keymap
(cat "�Qu� mapa de teclado hay que instalar?"
))

(set #which-keymap-help
(cat "\nAmiga puede utilizar distintos tipos de teclado. Un fichero de "
     "mapa de teclado (keymap) describe el tecla a Amiga para que sepa "
     "c�mo manejarlo. Elija el tipo de teclado de los que aparecen en "
     "la lista.\n\n"
     "Marque las cajas de los mapas de teclado que desea tener en su "
     "sistema.\n\n"
     @askoptions-help
))

(set #which-language
(cat "\n�Qu� idioma quiere instalar?"
))

(set #which-language-help
(cat "\nAmiga puede operar en diferentes idiomas. "
     "Se copiar�n algunos ficheros a su disco duro "
     "por cada idioma utilizado.\n\n"
     "Para reducir el espacio usado en disco duro "
     "puede seleccionar s�lo los ficheros del idioma "
     "que va a utilizar.\n\n"
     "S�lamente tiene que pulsar en el idioma que "
     "quiera instalar en su sistema.\n\n"
     @askoptions-help
))

(set #ask-locale
(cat "\n\nPor favor, inserte el dico \"Amiga Locale\" Versi�n�3.1 en cualquier unidad."
))

(set #no-font-space
(cat "\nNo hay suficiente espacio en  \"%s\" para el juego de caracteres "
     "de Amiga.\n\n"
     "No se copiar� el juego de caracteres Amiga.\n\n"
     "Puede copiar los tipos de letra m�s tarde moviendo el icono del "
     "disco \"Amiga Fonts\" Versi�n 3.1 sobre el icono \"%s\"."
))

(set #ask-fonts
(cat "\n\nPor favor, inserte el disco \"Amiga Fonts\" Versi�n�3.1 en cualquier unidad."
))

(set #ask-install
(cat "\n\nPor favor, inserte el disco \"Amiga Install\" Versi�n�3.1 en cualquier unidad."
))

(set #reorganizing
(cat "\n\nAjustando ficheros para que coincidan con su configuraci�n..."
))

(set #a2090
(cat "Si tiene una controladora A2090 con un disco duro SCSI y est� "
     "arrancando desde disquete, deber�a instalar el fichero "
     "Startup-A2090SCSI del caj�n Update de �ste disco como su "
     "Startup-sequence en el disco de arranque. "
     "Si tiene una controladora A2090 con un disco duro ST-506 y est� "
     "arrancando desde disquete, deber�a instalar el fichero "
     "Startup-A2090ST506 del caj�n Update de �ste disco como su "
     "startup-Sequence en el disco de arranque."
))

(set #reboot
(cat "La instalaci�n de Workbench 3.1 ha terminado.\n\n"
     "Para activarla, debe reinicializar su Amiga.\n\n"
     "Retire el disco \"Amiga Install\" de la unidad, "
     "y eliga \"Seguir\" para arrancar el Amiga."
))

))


;=============================================================================
; French strings

(if (= @language "fran�ais")
(
(set default_lang 16)

(set #bad-kick
(cat "Il vous faut utiliser le Kickstart 3.0 pour installer le Workbench 3.1"
))

(set #introduction
(cat "\n\nCe programme vous permet d'installer la version 3.1 du syst�me d'exploitation "
     "AmigaDOS sur un disque dur. Il peut �tre utilis� pour mettre � jour une "
     "version ant�rieure, installer pour la premi�re fois, ou pour contr�ler quels "
     "langues sont install�s sur un disque dur d�ja configur� avec le Workbench 3.1. "
))

(set #ask-function
(cat "\n\nChoississez \"Installer Version 3.1\" pour faire l'installation compl�te, ou "
     "\"Mettre � jour langues\" pour modifier les langues install�es sur un disque "
     "dur d�ja configur� avec le Workbench 3.1."
))

(set #ask-function-1
(cat "Installer Version 3.1"
))

(set #ask-function-2
(cat "Mettre � jour langues"
))

(set #ask-function-help
(cat "\nVous devez choisir quelle op�ration effectuer.\n\n"
     "Choisir \"Installer Version 3.1\" vous permet d'effectuer l'installation "
     "compl�te de la version 3.1. Cette option doit �tre choisie pour mettre � jour "
     "depuis une version ant�rieure, ou pour installer le Workbench 3.1 sur un "
     " disque dur vierge.\n\n"
     "Les ordinateurs Amiga peuvent �tre utilis�s avec plusieures langues. "
     "Quelques fichiers doivent �tre copi�s sur votre disque dur pour "
     "chacune des langues. Choisir \"Mettre � jour Langues\" vous permet "
     "de selectionner quelles langues sont install�s et utilisables sur "
     "le disque dur. Cette option ne doit �tre choisie que lorsque le "
     "workbench 3.1 a d�ja �t� install�.\n\n"
))

(set #confirm-target
(cat "\n\nCe programme va mettre votre disque dur � jour en installant le "
     "Workbench 3.1. Les fichiers du Workbench actuels seront rempla��s. "
     "Si n�cessaire, certains fichiers seront conserv�s, par exemple le "
     "fichier Startup-Sequence.\n\n"
     "D�sirez-vous que le Workbench 3.1 soit install� dans la partition \"%s\" ?"
))

(set #confirm-target-help
(cat "\nL'Installer a d�termin� que le Workbench 3.1 devrait probablement �tre "
     "install� dans la partition \"%s\". Continuer l'installation "
     "remplacera tous les fichiers du Workbench actuel par de nouveaux "
     "fichiers.\n\n"
     "Certains fichiers seront d'abord copi�s dans un endroit s�r, "
     "avant qu'ils ne soient remplac�s, comme par exemple le fichier "
     "Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nLes langues disponibles avec le Workbench 3.1 vont "
     "�tre mis � jour. Votre Workbench 3.1 semble �tre install� "
     "dans la partition \"%s\".\n\n"
     "Est-ce correct ?"
))

(set #confirm-target-lang-help
(cat "L'Installer a d�termin� que votre Workbench 3.1 est install� "
     "dans la partition \"%s\". Continuer l'installation va provoquer "
     "la copie des fichiers langages du Workbench � cet endroit."
))

(set #which-disk-lang
(cat "Dans quelles partitions les fichiers langages du Workbench 3.1 "
     "doivent-ils �tre install�s ?"
))

(set #which-disk-lang-help
(cat "Cette partie vous permet de choisir sur quelle partition les fichiers "
     "langages du Workbench 3.1 doivent �tre install�s. Ils devraient aller "
     "au m�me endroit que les fichiers du Workbench 3.1.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nLa mise � jour des langues du Workbench 3.1 est termin�e.\n\n"
     "Pour utiliser ces langues, vous devez red�marrer votre ordinateur. "
     "Apr�s le d�marrage, vous devez utiliser le programme des "
     "pr�f�rences \"Locale\" pour choisir la langue que vous souhaitez "
     "utiliser.\n\n"
     "Retirez la disquette \"Amiga Install\" du lecteur,"
     "et choississez la cellule \"Continuer\" pour red�marrer"
     "votre ordinateur Amiga."
))

(set #which-disk
(cat "Dans quelle partition faut-il installer le Workbench 3.1 ?"
))

(set #which-disk-help
(cat "\nCette partie vous permet de choisir la partition dans laquelle les "
     "fichiers du Workbench 3.1 seront install�s. Normalement, ils devraient "
     "�tre � la place des fichiers du Workbench 1.3, Workbench 2 ou "
     "Workbench 2.1.\n\n"
     @askdir-help
))

(set #old-name
(cat "ANCIEN"
))

(set #move-old
(cat "\n\nLes fichiers suivants peuvent-ils �tre d�plac�s dans le "
     "tiroir \"%s\" ?\n\n%s"
))

(set #move-old-help
(cat "\nL'installation du Workbench 3.1 remplacera la plupart des fichiers "
     "du Workbench actuel, y compris le fichier Startup-Sequence."
     "Pour pr�server toutes les modifications que vous auriez pu "
     "y faire, ces fichiers seront copi�s dans le tiroir \"%s\" o� vous "
     "pourrez les r�cup�rer ult�rieurement."
))

(set #move-old-1
(cat "D�placer"
))

(set #move-old-2
(cat "Sauter"
))

(set #checking-target
(cat "\n\nRecherche des anciens fichiers dans \"%s\" ..."
))

(set #delete-old
(cat "\n\nDe nombreux fichiers utilis�s par le Workbench 1.3 et le "
     "Workbench 2.0 sont inutiles avec le Workbench 3.1. "
     "Ces fichiers peuvent �tre effac�s de votre syst�me.\n\n"
     "Voulez-vous avoir � donner une autorisation avant que chaque "
     "fichier soit effac�, ou peuvent-ils tous �tre effac�s automatiquement ?"
))

(set #delete-old-help
(cat "\n\nDe nombreux fichiers du Workbench 1.3 et du Workbench 2.0 "
     "sont inutiles avec le Workbench 3.1. Ces fichiers devraient �tre "
     "effac�s pour �viter d'encombrer le disque dur et y r�cup�rer de la "
     "place. La cellule \"Demander avant d'effacer\" obligera le processus "
     "d'installation � obtenir votre autorisation avant d'effacer chacun de "
     "ces fichiers. Ceci vous permettra de conserver les fichiers de votre "
     "choix.\n\nLa cellule \"Effacer automatiquement\" autorisera "
     "l'effacement de ces fichiers sans qu'il vous soit rien demand�. "
))

(set #delete-old-1
(cat "Effacer automatiquement"
))

(set #delete-old-2
(cat "Demander avant d'effacer"
))

(set #deleting-obsolete
(cat "\n\nEffacement des fichiers inutiles..."
))

(set #confirm-delete
(cat "\n\nEffacer le fichier \"%s\" ?"
))

(set #confirm-delete-help
(cat "\nLe fichier \"%s\" est inutile avec le Workbench 3.1. Vous pouvez l'effacer "
     " en cliquant dans la cellule \"OUI\" ou le garder en cliquant sur \"NON\". "
))

(set #confirm-delete-reason-1
(cat "Le fichier est maintenant int�gr� dans les routines des m�moires mortes "
))

(set #confirm-delete-reason-2
(cat "Les fonctionnalit�s de ce programme sont reprises par d'autres "
     "programmes du Workbench."
))

(set #confirm-delete-reason-3
(cat "Ce programme a chang� de place, et y sera mis � jour."
))

(set #confirm-delete-reason-4
(cat "Ce programme est obsol�te et ne devra pas �tre utilis� avec "
     "le Workbench 3.1."
))

(set #confirm-delete-1
(cat "OUI"
))

(set #confirm-delete-2
(cat "NON"
))

(set #preparing-icons
(cat "\n\nMise en place des ic�nes..."
))

(set #keeping-old
(cat "Copie des fichiers � conserver..."
))

(set #ask-workbench
(cat "\n\nVeuillez introduire la disquette \"Amiga Workbench\" version 3.1 "
     "dans un des lecteurs. "
))

(set #ask-extras
(cat "\n\nVeuillez introduire la disquette \"Amiga Extras\" version 3.1 "
     "dans un des lecteurs. "
))

(set #ask-storage
(cat "\n\nVeuillez introduire la disquette \"Amiga Storage\" version 3.1 "
     "dans un des lecteurs. "
))

(set #which-printer
(cat "Quels sont les pilotes d'imprimantes � installer ?"
))

(set #which-printer-help
(cat "\nL'Amiga peut g�rer de nombreuses imprimantes. "
     "Les pilotes d'imprimantes sont des fichiers qui "
     "permettent � l'Amiga de s'adapter � une imprimante donn�e. "
     "Il doit y avoir un pilote d'imprimante copi� sur le disque dur "
     "pour chaque imprimante utilis�e.\n\n"
     "Pour r�duire la quantit� d'espace utilis� par les pilotes "
     "d'imprimantes, vous pouvez s�lectionner seulement ceux qui "
     "vous seront utiles.\n\n"
     "Cochez les pilotes d'imprimantes que vous souhaitez "
     "sur votre syst�me.\n\n"
     @askoptions-help
))

(set #which-keymap
(cat "Quel sont les claviers � installer ?"
))

(set #which-keymap-help
(cat "\nVotre ordinateur Amiga peut �tre utilis� avec des claviers diff�rents. "
     "Un fichier \"Keymap\" d�crit le clavier afin que l'ordinateur puisse le "
     "reconna�tre. Choississez le clavier parmi la liste des claviers "
     "disponibles.\n\n"
     "Cochez le(s) fichier(s) \"Keymap\" que vous souhaitez "
     "sur votre syst�me.\n\n"
     @askoptions-help
))

(set #which-language
(cat "\nQuelles sont les langues � installer ?"
))

(set #which-language-help
(cat "\nL'Amiga peut fonctionner dans diff�rentes langues. "
     "Plusieurs fichiers doivent �tre copi�s sur le disque dur "
     "pour chaque langue utilis�ee\n\n"
     "Pour r�duire la quantit� d'espace utilis� par ces fichiers, "
     "vous devez s�lectionner seulement les langues qui vous seront "
     "utiles.\n\n"
     "Cochez les langues que vous souhaitez "
     "sur votre syst�me\n\n"
     @askoptions-help
))

(set #ask-locale
(cat "\n\nVeuillez introduire la disquette \"Amiga Locale\" version 3.1 "
     "dans un des lecteurs. "
))

(set #no-font-space
(cat "\nIl n'y a, pour l'instant, pas assez de place sur \"%s\" pour "
     "mettre les fontes standard de l'Amiga.\n\n"
     "Le jeu de fontes Amiga ne sera pas copi�.\n\n"
     "Plus tard, apr�s avoir fait de la place sur \"%s\", "
     "vous pourrez copier ces fontes en d�pla�ant l'ic�ne de "
     "la disquette \"Amiga Fonts\" version 3.1 sur l'ic�ne \"%s\"."
))

(set #ask-fonts
(cat "\n\nVeuillez introduire la disquette \"Amiga Fonts\" version 3.1 dans un "
     "des lecteurs. "
))

(set #ask-install
(cat "\n\nVeuillez introduire la disquette \"Amiga Install\" version�3.1 dans un "
     "des lecteurs. "
))

(set #reorganizing
(cat "\n\nModification des fichiers pour se rapprocher de votre "
     "configuration pr�c�dente..."
))

(set #a2090
(cat "Si vous utilisez un contr�lleur de disque dur A2090 avec un "
     "disque dur SCSI, et que vous d�marrez depuis une disquette, "
     "vous devez copier le fichier Startup-A2090SCSI du tiroir Update "
     "de cette disquette sur votre disquette de d�marrage en tant que "
     "fichier Startup-Sequence.\n"
     "Si vous utilisez un contr�lleur de disque dur A2090 avec un "
     "disque dur ST-506, et que vous d�marrez depuis une disquette, "
     "vous devez copier le fichier Startup-A2090ST506 du tiroir Update "
     "de cette disquette sur votre disquette de d�marrage en tant que "
     "fichier Startup-Sequence."
))

(set #reboot
(cat "\nL'installation du Workbench 3.1 est termin�e.\n\n"
     "Pour en profiter, vous devez red�marrer votre ordinateur Amiga.\n\n"
     "Retirez la disquette \"Amiga Install\" du lecteur de disquette, "
     "et choississez la cellule \"Continuer\" pour red�mmarrer votre "
     "ordinateur Amiga."
))

))


;=============================================================================
; German strings

(if (= @language "deutsch")
(
(set default_lang 2)

(set #bad-kick
(cat "Sie m�ssen Kickstart 3.0 zum Installieren von Workbench�3.1 benutzen."
))

(set #introduction
(cat "\n\nHiermit k�nnen Sie Version 3.1 des Amiga-Betriebssystems auf einer "
     "Festplatte installieren. Sie k�nnen damit eine �ltere Version "
     "aktualisieren, von Grund auf neu installieren oder nachtr�glich "
     "w�hlen, welche Sprachen unter Version 3.1 auf der Festplatte zur "
     "Verf�gung stehen sollen."
))

(set #ask-function
(cat "\n\nW�hlen Sie \"Version 3.1 installieren\", um eine komplette "
     "Installation durchzuf�hren, oder \"Sprachen aktualisieren\", um "
     "andere Sprachen auf einer Festplatte zur Verf�gung zu stellen, "
     "auf der bereits Version 3.1 installiert ist."
))

(set #ask-function-1
(cat "Version 3.1 installieren"
))

(set #ask-function-2
(cat "Sprachen aktualisieren"
))

(set #ask-function-help
(cat "\nSie m�ssen ausw�hlen, was installiert werden soll.\n\n"
     "Mit \"Version 3.1 installieren\" wird eine komplette Installation "
     "der Version 3.1 der Workbench-Software durchgef�hrt. Dies sollten "
     "Sie w�hlen, um ein System von einer �lteren Version ausgehend zu "
     "aktualisieren oder die Version 3.1 frisch auf einer neuen Festplatte "
     "zu installieren.\n\n"
     "Der Amiga kann in vielen verschiedenen Sprachen betrieben werden. "
     "F�r jede verwendete Sprache m�ssen mehrere Dateien auf Ihre "
     "Festplatte kopiert werden. Mit \"Sprachen aktualisieren\" k�nnen "
     "Sie ausw�hlen, welche Sprachen auf der Festplatte installiert "
     "werden sollen, um dort zur Verf�gung zu stehen. Dies d�rfen Sie "
     "nur w�hlen, wenn die Version 3.1 schon installiert wurde.\n\n"
))

(set #confirm-target
(cat "\n\nDieses Programm installiert Version 3.1 auf Ihrer Festplatte. "
     "Bisherige Workbench-Versionen werden �berschrieben. Wenn angebracht, "
     "werden bestimmte Dateien der fr�heren Version bewahrt, darunter "
     "die Startup-Sequence.\n\n"
     "Wollen Sie die Version 3.1 auf der Partition \"%s\" installiert "
     "bekommen?"
))

(set #confirm-target-help
(cat "\nDas Installationsprogramm hat herausgefunden, da� die Version 3.1 "
     "wahrscheinlich auf der Partition namens \"%s\" installiert "
     "werden soll. Bei der weiteren Installation "
     "werden die meisten Dateien in dieser Partition durch neuere Dateien "
     "ersetzt.\n\nEinige Dateien werden vorher an einen sicheren Platz "
     "kopiert, bevor ihre Nachfolger installiert werden, darunter die "
     "Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nDie unter Version 3.1 verf�gbaren Sprachen werden jetzt auf Ihrer "
     "Festplatte aktualisiert. Ihre Workbench-Dateien der Version 3.1 sind "
     "offenbar auf der Partition \"%s\" installiert.\n\n"
     "Ist das korrekt?"
))

(set #confirm-target-lang-help
(cat "\nDer Installer hat herausgefunden, da� die aktuellen "
     "Workbench-Dateien offenbar in der Partition namens \"%s\" liegen. "
     "Wenn Sie die Installation einfach weiterlaufen lassen, werden die "
     "Sprachendateien dorthin kopiert."
))

(set #which-disk-lang
(cat "Auf welcher Festplattenpartition sollen die Sprachendateien f�r "
     "Version 3.1 installiert werden?"
))

(set #which-disk-lang-help
(cat "\nHier k�nnen Sie w�hlen, auf welcher Festplattenpartition die "
     "Sprachendateien f�r Version 3.1 installiert werden sollen. Das "
     "sollte dieselbe Partition wie die mit den Workbench-Dateien "
     "der Version 3.1 sein.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nDie Aktualisierung der Version 3.1 ist nun abgeschlossen.\n\n"
     "Um diese neuen Sprachen zu aktivieren, m�ssen Sie Ihren Amiga "
     "neu starten und danach den Voreinsteller-Editor \"Locale\" "
     "aus der Schublade \"Prefs\" aufrufen und dort die gew�nschte "
     "Sprache ausw�hlen.\n\n"
     "Entfernen Sie zun�chst die Diskette \"Amiga Install\" aus dem "
     "Diskettenlaufwerk und w�hlen Sie dann das Symbol \"Weiter\", "
     "um Ihren Amiga neu zu starten."
))

(set #which-disk
(cat "Auf welcher Partition soll Version�3 installiert werden?"
))

(set #which-disk-help
(cat "\nHier k�nnen Sie w�hlen, auf welcher Festplattenpartition "
     "die Dateien von Version 3.1 installiert werden sollen. "
     "Sie sollten normalerweise an dieselbe Stelle installiert werden "
     "wie die bisherigen Dateien von Version 1.3, 2 oder 2.1.\n\n"
     @askdir-help
))

(set #old-name
(cat "ALT"
))

(set #move-old
(cat "\n\nSollen folgende Dateien in die Schublade \"%s\" kopiert "
     "werden?\n\n%s"
))

(set #move-old-help
(cat "\nDie Installation der Version 3.1 ersetzt die meisten bisherigen "
     "Workbench-Dateien, darunter die Startup-Sequence. "
     "Damit alle �nderungen erhalten bleiben, die Sie fr�her an diesen "
     "Dateien vorgenommen haben, werden sie bei der Installation in die "
     "Schublade \"%s\" kopiert, woher Sie sie sp�ter zur�ckholen k�nnen."
))

(set #move-old-1
(cat "Kopieren"
))

(set #move-old-2
(cat "�berspringen"
))

(set #checking-target
(cat "\n\n�berpr�fen von \"%s\" auf �berholte Dateien ..."
))

(set #delete-old
(cat "\n\nViele System-Dateien, die unter fr�heren Versionen "
     "ben�tigt wurden, werden unter Version 3.1 nicht l�nger gebraucht. "
     "Diese Dateien sollen nun aus Ihrem System gel�scht werden.\n\n"
     "Wollen Sie bei jeder Datei einzeln um Best�tigung gebeten werden, "
     "oder k�nnen sie alle automatisch gel�scht werden?"
))

(set #delete-old-help
(cat "\n�ltere Systemversionen umfassen mehrere Dateien, die "
     "unter Version 3.1 nutzlos sind. Diese Dateien sollten gel�scht "
     "werden, um Unordnung zu vermeiden und Platz auf Ihrer Festplatte "
     "zu sparen.\n\n"
     "Nach Anklicken von \"Automatisch l�schen\" werden alle �berholten "
     "Systemdateien ohne weitere Nachfrage gel�scht.\n"
     "Nach Anklicken von \"L�schen best�tigen\" bittet das Installationsprogramm "
     "bei jeder Datei vor dem L�schen um Best�tigung. Somit k�nnen Sie "
     "bestimmte Dateien erhalten, die Sie nicht l�schen wollen.\n\n"
))

(set #delete-old-1
(cat "Automatisch l�schen"
))

(set #delete-old-2
(cat "L�schen best�tigen"
))

(set #deleting-obsolete
(cat "\n\nL�schen �berholter Dateien"
))

(set #confirm-delete
(cat "\n\nKann die Datei \"%s\" gel�scht werden?"
))

(set #confirm-delete-help
(cat "\nDie Datei "
     "\"%s\" ist unter Version 3.1 �berholt. Sie k�nnen die Datei "
     "durch Anklicken von \"Ja\" l�schen oder durch Anklicken von \"Nein\" "
     "erhalten."
))

(set #confirm-delete-reason-1
(cat "Diese Datei ist in Version 3.1 jetzt in das ROM integriert."
))

(set #confirm-delete-reason-2
(cat "Die Aufgaben dieses Programms werden jetzt von anderen "
     "Workbench-Programmen �bernommen."
))

(set #confirm-delete-reason-3
(cat "Dieses Programm wurde auf einen anderen Pfad verlegt und wird "
     "dort aktualisiert."
))

(set #confirm-delete-reason-4
(cat "Dieses Programm ist �berholt und sollte nicht unter Version 3.1 "
     "benutzt werden."
))

(set #confirm-delete-1
(cat "Ja"
))

(set #confirm-delete-2
(cat "Nein"
))

(set #preparing-icons
(cat "\n\nVorbereiten der Piktogramme ..."
))

(set #keeping-old
(cat "\n\nErstellen von Sicherheitskopien wichtiger Dateien ..."
))

(set #ask-workbench
(cat "\n\nBitte legen Sie die Diskette \"Amiga Workbench\" Version 3.1 "
     "in ein beliebiges Diskettenlaufwerk ein."
))

(set #ask-extras
(cat "\n\nBitte legen Sie die Diskette \"Amiga Extras\"  Version�3.1 in "
     "ein beliebiges Diskettenlaufwerk ein."
))

(set #ask-storage
(cat "\n\nBitte legen Sie die Diskette \"Amiga Storage\"  Version�3.1 in "
     "ein beliebiges Diskettenlaufwerk ein."
))

(set #which-printer
(cat "Welche Druckertreiber sollen installiert werden?"
))

(set #which-printer-help
(cat "\nDer Amiga kann viele verschiedene Drucker ansteuern. "
     "Druckertreiber sind Dateien, mit denen sich der Amiga an den "
     "jeweiligen Drucker anpassen kann. F�r jeden verwendeten Drucker "
     "mu� eine Druckertreiberdatei auf die Festplatte kopiert werden.\n\n"
     "Um den Platz in Grenzen zu halten, den die Treiberdateien einnehmen, "
     "k�nnen Sie w�hlen, nur diejenigen Treiber zu installieren, die Sie "
     "�berhaupt verwenden k�nnen.\n\n"
     "Klicken Sie einfach die Felder derjenigen Drucker an, die Sie in "
     "Ihrem System verf�gbar haben wollen, so da� diese Drucker mit "
     "einem H�kchen markiert sind.\n\n"
     @askoptions-help
))

(set #which-keymap
(cat "Welche Tastaturbelegung (Keymap) soll installiert werden?"
))

(set #which-keymap-help
(cat "\nDer Amiga kann mit vielen verschiedenen Tastaturen betrieben "
     "werden. "
     "Eine \"Keymap\"-Datei (Tastaturbelegung) beschreibt dem Amiga "
     "eine Tastatur, so da� sie korrekt benutzt werden kann. W�hlen "
     "Sie aus der Liste die Tastatur aus, die Sie haben.\n\n"
     "Klicken Sie einfach die Felder derjenigen Tastaturbelegungen an, "
     "die Sie in Ihrem System verf�gbar haben wollen, so da� diese mit "
     "einem H�kchen markiert sind.\n\n"
     @askoptions-help
))

(set #which-language
(cat "\nWelche Sprachen sollen installiert werden?"
))

(set #which-language-help
(cat "\nDer Amiga kann in vielen verschiedenen Sprachen betrieben werden. "
     "F�r jede verwendete Sprache m�ssen mehrere Dateien auf Ihre "
     "Festplatte kopiert werden.\n\n"
     "Um den Platz in Grenzen zu halten, den die Sprachentreiberdateien "
     "einnehmen, k�nnen Sie w�hlen, nur die Treiber f�r bestimmte "
     "Sprachen zu installieren.\n\n"
     "Klicken Sie einfach die Felder derjenigen Sprachen an, die Sie in "
     "Ihrem System verf�gbar haben wollen, so da� diese Sprachen mit "
     "einem H�kchen markiert sind.\n\n"
     @askoptions-help
))

(set #ask-locale
(cat "\n\nBitte legen Sie die Diskette \"Amiga Locale\" Version�3.1 in "
     "ein beliebiges Diskettenlaufwerk ein."
))

(set #no-font-space
(cat "\nIm Augenblick reicht der Platz auf \"%s\" nicht aus, um alle "
     "standardm��igen Amiga-Zeichens�tze (Fonts) zu installieren.\n\n"
     "Die Zeichens�tze werden vorerst nicht kopiert.\n\n"
     "Sie k�nnen die Zeichens�tze aber sp�ter kopieren, indem Sie das "
     "Piktogramm der Diskette \"Amiga Fonts\" Version 3.1 �ber das "
     "Piktogramm \"%s\" ziehen."
))

(set #ask-fonts
(cat "\n\nBitte legen Sie die Diskette \"Amiga Fonts\" Version�3.1 in "
     "ein beliebiges Diskettenlaufwerk ein."
))

(set #ask-install
(cat "\n\nBitte legen Sie die Diskette \"Amiga Install\" Version 3.1 in "
     "ein beliebiges Diskettenlaufwerk ein."
))

(set #reorganizing
(cat "\n\nAnpassen der Dateien an Ihre vorherige Konfiguration ..."
))

(set #a2090
(cat "Wenn Sie einen Festplattencontroller A2090 mit einer SCSI-Festplatte "
     "benutzen und von einer Diskette starten, sollten Sie die Datei "
     "Startup-A2090SCSI aus der Schublade \"Update\" auf dieser Diskette "
     "als Datei \"Startup-Sequence\" auf Ihrer Startdiskette "
     "installieren.\n"
     "Wenn Sie einen Festplattencontroller A2090 mit einer "
     "ST-506-Festplatte benutzen und von einer Diskette starten, sollten "
     "Sie die Datei \"Startup-A2090ST506\" aus der Schublade \"Update\" "
     "auf dieser Diskette als Datei \"Startup-Sequence\" auf Ihrer "
     "Startdiskette installieren."
))

(set #reboot
(cat "Die Installation der Version 3.1 ist nun abgeschlossen.\n\n"
     "Um die Version 3.1 zu aktivieren, m�ssen Sie Ihren Amiga neu "
     "starten. "
     "Nehmen Sie die Diskette \"Amiga Install\" aus dem Diskettenlaufwerk "
     "und klicken Sie dann auf \"Weiter\", um Ihren Amiga neu zu starten.\n\n"
     "Danach m�ssen Sie ggf. den Voreinsteller-Editor \"Locale\" "
     "aus der Schublade \"Prefs\" aufrufen und dort Ihr Land und die "
     "gew�nschte Sprache ausw�hlen. Pr�fen Sie im Voreinsteller-Editor "
     "\"Input\", ob dort die gew�nschte Tastaturbelegung "
     "ausgew�hlt ist."
))

))


;=============================================================================
; Dutch strings

(if (= @language "nederlands")
(
(set default_lang 64)

(set #bad-kick
(cat "U moet Kickstart 3.0 gebruiken om Workbench�3.1 te installeren"
))

(set #introduction
(cat "\n\nDit programma installeert Release 3.1 van het Amiga besturings "
     "systeem op uw harddisk. Het kan gebruikt worden om van een "
     "oudere versie naar een nieuwere versie van het besturings systeem "
     "over te gaan, om een volledig nieuwe installatie te doen, of om een "
     "andere taal te installeren op een harddisk, waarop Release 3.1 reeds "
     "ge�nstalleerd is."
))

(set #ask-function
(cat "\n\nSelecteer \"Installeer Release 3.1\" om een complete installatie "
     "van Release 3.1 te doen, of selecteer \"Andere talen\" om de talen welke "
     "beschikbaar zijn op een harddisk, waarop Release 3.1 reeds is "
     "ge�nstalleerd, te wijzigen."
))

(set #ask-function-1
(cat "Installeer Release 3.1"
))

(set #ask-function-2
(cat "Andere talen"
))

(set #ask-function-help
(cat "\nU moet kiezen welke functie u wilt uitvoeren.\n\n"
     "Het selecteren van \"Installeer Release 3.1\" stelt u in staat "
     "een volledige installatie van de Release 3.1 Workbench software "
     "uit te voeren. Deze functie moet gekozen worden wanneer u "
     "upgrade van een eerdere systeem Release, of wanneer u Release 3.1 "
     "installeert op een een nieuwe harddisk.\n\nDe Amiga kan gebruikt "
     "worden in vele verschillende talen. Voor elke ondersteunde taal "
     "moeten verschillende bestanden naar de harddisk gekopieerd worden. "
     "Het selecteren van \"Andere talen\" stelt u in staat te bepalen "
     "welke talen op de harddisk ge�nstalleerd en beschikbaar zijn. De "
     "functie kan alleen gekozen worden wanneer Release 3.1 reeds is "
     "ge�nstalleerd.\n\n"
))

(set #confirm-target
(cat "\n\nRelease 3.1 wordt nu ge�nstalleerd op uw harddisk. Elke "
     "voorgaande Release, met uitzondering van specifieke bestanden "
     "zoals de startup-Sequence, zal vervangen worden.\n\n"
     "Wilt u Release 3.1 ge�nstalleerd hebben op de \"%s\" partitie?"
))

(set #confirm-target-help
(cat "\nHet installatie programma heeft ontdekt dat uw huidige versie "
     "van de Workbench in de lade \"%s\" staat. Bij het "
     "voortzetten van deze installatie procedure zullen de meeste "
     "bestanden in deze "
     "lade worden vervangen door nieuwere bestanden.\n\nSommige bestanden "
     "zullen echter eerst naar een andere plaats worden gekopieerd, "
     "voordat zij vervangen worden. Hieronder vallen ook uw huidige "
     "Startup-Sequence en Shell-Startup bestanden."
))

(set #confirm-target-lang
(cat "\n\nDe Release 3.1 talen op uw harddisk worden nu ge-update. Uw Release 3.1 "
     "Workbench bestanden blijken zich op de \"%s\" partitie te bevinden.\n\n"
     "Is dit juist?"
))

(set #confirm-target-lang-help
(cat "\nDe installer heeft bepaald dat uw huidige versie van uw Workbench "
     "op de \"%s\" partitie staat. Voortgang van de installatie zal de "
     "Workbench taal bestanden daarheen kopi�ren."
))

(set #which-disk-lang
(cat "Op welke partitie moeten de Release 3.1 taal bestanden ge�nstalleerd "
     "worden?"
))

(set #which-disk-lang-help
(cat "\nDit gedeelte stelt u in staan te kiezen op welke partitie de "
     "Release 3.1 taal bestanden ge�nstalleerd moeten worden. Deze behoren "
     "op dezelfde plaats als de Release 3.1 Workbench bestanden te komen.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nDe installatie van Release 3.1 taal bestanden is afgerond. Om de "
     "nieuwe talen te gebruiken, moet u uw Amiga herstarten. Als de Amiga "
     "opnieuw opgestart is, kunt u de \"Locale\" preferences editor "
     "gebruiken om de gewenste taal in te stellen.\n\n"
     "Verwijder de \"Amiga Install\" diskette uit de diskette-eenheid, en "
     "selecteer \"Doorgaan\" om de Amiga te herstarten."
))

(set #which-disk
(cat "Op welke partitie moet Workbench�3.1 ge�nstalleerd worden?"
))

(set #which-disk-help
(cat "\nDit gedeelte stelt u in staat te kiezen op welke partitie de "
     "Release 3.1 bestanden ge�nstalleerd moeten worden. Gewoonlijk "
     "dienen deze op dezelfde plaats te komen als de huidige Workbench�"
     "1.3 of Workbench�2.0 bestanden.\n\n"
     @askdir-help
))

(set #old-name
(cat "OLD"
))

(set #move-old
(cat "\n\nKunnen de volgende bestanden worden verplaatst naar de "
     "\"%s\" lade?\n\n%s"
))

(set #move-old-help
(cat "\nDe installatie van Release 3.1 zal de meeste huidige Workbench "
     "bestanden, waaronder ook de Startup-Sequence en Shell-Startup, "
     "vervangen. Om de door u in deze bestanden aangebrachte wijzigingen "
     "te behouden, zal de installatie procedure ze naar de \"%s\" lade "
     "kopi�ren, zodat ze daar alsnog door u opgevraagd kunnen worden."
))

(set #move-old-1
(cat "Verplaats"
))

(set #move-old-2
(cat "Overslaan"
))

(set #checking-target
(cat "\n\n\"%s\" wordt gecontroleerd op overbodige bestanden..."
))

(set #delete-old
(cat "\n\nVeel bestanden, die het systeem onder Workbench 1.3 en "
     "Release 2 gebruikte, zijn onder Release 3.1 niet meer nodig. "
     "Deze bestanden mogen nu verwijderd worden.\n\n"
     "Wilt u voor ieder bestand afzonderlijk om bevestiging gevraagd "
     "worden of mogen zij allemaal automatisch verwijderd worden?"
))

(set #delete-old-help
(cat "\nWorkbench�1.3 en Release 2 bevatten diverse bestanden, die "
     "onder Release 3.1 overbodig zijn geworden. Deze bestanden zouden "
     "verwijderd moeten worden om e.e.a. overzichtelijk te houden en "
     "bovendien om ruimte op uw harddisk vrij te maken.\n\n"
     "De \"Eerst vragen\" keuze verplicht de installatie procedure u eerst "
     "om een bevestiging te vragen, alvorens ieder afzonderlijk bestand "
     "te verwijderen. Hiermede kunt u bepaalde bestanden behouden die u "
     "beslist niet wilt laten verwijderen.\n\n"
     "De \"Verwijder automatisch\" keuze zal alle bestanden, zonder uw "
     "tussenkomst verwijderen."
))

(set #delete-old-1
(cat "Verwijder automatisch"
))

(set #delete-old-2
(cat "Eerst vragen"
))

(set #deleting-obsolete
(cat "\n\nOverbodige bestanden worden nu verwijderd"
))

(set #confirm-delete
(cat "\n\nMag het bestand \"%s\" verwijderd worden?"
))

(set #confirm-delete-help
(cat "\nHet installatie programma heeft geconstateerd dat het bestand "
     "\"%s\" onder Release 3.1 overbodig is geworden. U kunt dit "
     "bestand verwijderen door \"Ja\" te selecteren, of het bestand "
     "behouden door \"Nee\" te selecteren."
))

(set #confirm-delete-reason-1
(cat "Dit bestand bevindt zich in de Release 3.1 ROM."
))

(set #confirm-delete-reason-2
(cat "De functionaliteit van dit programma is geintegreerd in andere "
     "Workbench programma's."

))

(set #confirm-delete-reason-3
(cat "Dit programma is verplaatst naar een andere locatie, en wordt daar "
     "ge-update."
))

(set #confirm-delete-reason-4
(cat "Dit programma is overbodig mag niet meer gebruikt worden onder "
     "Release 3.1"
))

(set #confirm-delete-1
(cat "Ja"
))

(set #confirm-delete-2
(cat "Nee"
))

(set #preparing-icons
(cat "\n\nPrepareren iconen..."
))

(set #keeping-old
(cat "Belangrijke bestanden worden nu veilig gesteld..."
))

(set #ask-workbench
(cat "\n\nPlaats a.u.b. de \"Amiga Workbench\" diskette Versie�3.1 "
     "in een willekeurige diskette-eenheid."
))

(set #ask-extras
(cat "\n\nPlaats a.u.b. de \"Amiga Extras\" diskette Versie�3.1 in "
     "een willekeurige diskette-eenheid."
))

(set #ask-storage
(cat "\n\nPlaats a.u.b. de \"Amiga Storage\" diskette Versie�3.1 in "
     "een willekeurige diskette-eenheid."
))

(set #which-printer
(cat "Welke printer-drivers moeten ge�nstalleerd worden?"
))

(set #which-printer-help
(cat "\nDe Amiga heeft de mogelijkheid om verschillende printers. "
     "aan te sturen. Printer-drivers zijn bestanden die de Amiga "
     "nodig heeft om een printer aan te sturen. Er moet een printer "
     "driver naar de harddisk gekopieerd worden voor elke printer "
     "die ondersteund moet worden.\n\n"
     "Om de groote van het door de printer-drivers gebruikte geheugen "
     "te beperken kunt u een keuze maken uit de printer-drivers die "
     "uw systeem nodig heeft om uw printer op de juiste manier aan te "
     "sturen.\n\n"
     "U hoeft alleen die printer-drivers die u op uw systeem wilt "
     "hebben aan te klikken.\n\n"
     @askoptions-help
))

(set #which-keymap
(cat "Welke Keymap moet ge�nstalleerd worden?"
))

(set #which-keymap-help
(cat "De Amiga kan werken met verschillende toetsenborden."
     "Een Keymap bestand omschrijft het toetsenbord voor de Amiga, zodat "
     "het met het betreffende toetsenbord kan werken. Kies het type "
     "toetsenbord dat u heeft, uit de lijst met beschikbare keuzes.\n\n"
     "Kies de toetsenbord bestanden die u beschikbaar wilt hebben op uw "
     "systeem.\n\n"
     @askoptions-help
))

(set #which-language
(cat "\nWelke taal moet ge�nstalleerd worden?"
))

(set #which-language-help
(cat "\nDe Amiga kan werken in verschillende talen."
     "Voor elke taal moeten er een aantal bestanden naar "
     "uw harddisk gekopieerd worden.\n\n"
     "Om de groote van het geheugen die de taal bestanden "
     "innemen te verkleinen, kunt u een keuze maken uit "
     "de talen, die vervolgens naar uw harddisk "
     "gekopieerd worden.\n\nU hoeft alleen die talen die u op "
     "uw systeem wilt hebben aan te klikken.\n\n"
     @askoptions-help
))

(set #ask-locale
(cat "\n\nPlaats a.u.b. de \"Amiga Locale\" diskette Versie�3.1 in "
     "een willekeurige diskette-eenheid."
))

(set #no-font-space
(cat "\nEr is niet genoeg ruimte op \"%s\" voor de standaard "
     "Amiga fonts.\n\n"
     "De Amiga fonts zullen niet gekopieerd worden.\n\n"
     "U kunt de fonts later kopi�ren door het icoon van de diskette "
     "\"Amiga Fonts\" Versie 3.1 over het \"%s\" icoon te slepen."
))

(set #ask-fonts
(cat "\n\nPlaats a.u.b. de \"Amiga Fonts\" diskette Versie�3.1 in "
     "een willekeurige diskette-eenheid."
))

(set #ask-install
(cat "\n\nPlaats a.u.b. de \"Amiga Install\" disk Versie�3.1 in "
     "een willekeurige diskette-eenheid."
))

(set #reorganizing
(cat "Verschillende bestanden worden aangepast aan uw voorgaande"
     "configuratie..."
))

(set #a2090
(cat "Als u gebruik maakt van een A2090 harddisk controller met een "
     "SCSI harddisk, en u start deze op met een boot diskette, dan "
     "moet u het bestand Startup-A2090SCSI uit de Update lade van deze "
     "diskette installeren als uw Startup-Sequence voor de boot diskette."
     "Als u gebruik maakt van een A2090 harddisk controller met een "
     "ST-506 harddisk, en u start deze op met een boot diskette, dan moet "
     "u het bestand Startup-A2090ST506 uit de Update lade van deze diskette "
     "installeren als uw Startup-Sequence voor de boot diskette."
))

(set #reboot
(cat "De installatie van Release 3.1 is afgerond.\n\nOm Release 3.1 te "
     "activeren moet u uw Amiga herstarten.\n\n"
     "Verwijder de \"Amiga Install\" diskette uit de diskette-eenheid, "
     "en selecteer \"Doorgaan\" om de Amiga te herstarten."
))

))


;=============================================================================
; Italian strings

(if (= @language "italiano")
(
(set default_lang 32)

(set #bad-kick
(cat "E' necessario il Kickstart 3.0 per installare il Workbench�3.1!"
))

(set #introduction
(cat "\n\nQuesto programma installa la Versione 3.1 del Sistema Operativo "
     "di Amiga su disco fisso. Pu� essere usato per aggiornare una versione "
     "precedente, per una nuova installazione o per controllare quali "
     "lingue sono disponibili sul disco fisso con la Versione 3.1 gi� "
     "installata."
))

(set #ask-function
(cat "\n\nSelezionare \"Installa Versione 3.1\" per fare una installazione "
     "completa della Versione 3.1, o selezionare \"Aggiorna lingue\" per "
     "modificare le lingue disponibili sul disco fisso con la Versione 3.1 "
     "gi� installata."
))

(set #ask-function-1
(cat "Installa Versione 3.1"
))

(set #ask-function-2
(cat "Aggiorna lingue"
))

(set #ask-function-help
(cat "\nDovete scegliere l'operazione che volete effettuare.\n\n"
     "Selezionando il pulsante \"Installa Versione 3.1\" verr� effettuata "
     "una installazione completa del software di sistema Versione 3.1. Questa "
     "opzione dovrebbe essere scelta quando si vuole aggiornare un "
     "sistema avente versioni precedenti, o quando si vuole installare "
     "la Versione 3.1 su un disco fisso nuovo.\n\n"
     "Amiga pu� operare con lingue differenti. Numerosi file devono essere "
     "copiati sul disco fisso per ogni lingua supportata. "
     "Selezionando il pulsante \"Aggiorna lingue\" potrete controllare "
     "quali lingue sono installate e quali sono disponibili per l'utilizzo "
     "col disco fisso. Questa opzione deve essere scelta solo se "
     "la Versione 3.1 � gi� stata installata.\n\n"
))

(set #confirm-target
(cat "\nLa Versione 3.1 verr� copiata sul disco fisso, qualsiasi versione "
     "precedente verr� sostituita. Se utilizzabili, alcuni file della "
     "versione precedente saranno conservati, inclusa la Startup-Sequence.\n\n"
     "Volete installare la Versione 3.1 nella partizione \"%s\"?"
))

(set #confirm-target-help
(cat "\nIl programma di installazione ha determinato che la Versione 3.1 "
     "dovrebbbe essere installata nella partizione \"%s\". Procedendo con "
     "l'installazione molti file del Workbench in questa partizione saranno "
     "sostituiti da quelli nuovi.\n\n"
     "Alcuni saranno copiati in una zona di sicurezza "
     "prima della loro sostituzione, tra questi la Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nLe lingue della Versione 3.1 disponibili sul disco fisso "
     "verranno aggiornate. I file della versione 3.1 del Workbench sembrano "
     "trovarsi nella partizione \"%s\".\n\n"
     "Questo � corretto?"
))

(set #confirm-target-lang-help
(cat "\nIl programma di installazione ha determinato che la vostra copia "
     "del Workbench si trova nella partizione \"%s\". Continuando con "
     "l'installazione i file delle lingue saranno copiati in essa. "
))

(set #which-disk-lang
(cat "In quale partizione vanno installati i file lingue della Versione 3?"
))

(set #which-disk-lang-help
(cat "\nIn questa sezione potete scegliere in quale partizione vanno "
     "installati i file delle lingue della Versione 3.1. Dovrebbe essere "
     "la stessa dove risiedono i file del Workbench versione 3.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nL'aggiornamento delle lingue della Versione 3.1 � stato completato.\n\n"
     "Per usare queste nuove lingue � necessario riavviare Amiga. Dopo il "
     "riavvio dovrete utilizzare il programma di preferenze \"Locale\" per "
     "scegliere la lingua che volete utilizzare.\n\n"
     "Togliere il disco \"Amiga Install\" dalla unit� a dischetti, e "
     "selezionare il pulsante \"Procedi\" per riavviare Amiga."
))

(set #which-disk
(cat "In quale partizione deve essere installata la Versione�3?"
))

(set #which-disk-help
(cat "\nQuesta sezione vi permette di scegliere in quale partizione devono "
     "essere installati i file del Workbench�3.1. Questa dovrebbe essere "
     "la stessa locazione in cui erano presenti i file del Workbench�1.3 "
     "o del Workbench�2.0.\n\n"
     @askdir-help
))

(set #old-name
(cat "VECCHI"
))

(set #move-old
(cat "\n\nQuesti file devono essere spostati nel cassetto \"%s\"?\n\n%s"
))

(set #move-old-help
(cat "\nInstallando la Versione�3 molti file del Workbench attuale "
     "saranno sostituiti, inclusa la Startup-Sequence. "
     "Per mantenere eventuali vostre modifiche precedenti di "
     "questi file, il processo di installazione li copier� nel cassetto "
     "\"%s\" da dove potranno essere recuperati in un secondo tempo."
))

(set #move-old-1
(cat "Sposta"
))

(set #move-old-2
(cat "Salta"
))

(set #checking-target
(cat "\n\nControllo file obsoleti in \"%s\"..."
))

(set #delete-old
(cat "\n\nMolti file usati dalle versioni precedenti "
     "non sono pi� richiesti nella Versione�3.1. "
     "Questi file possono essere cancellati dal sistema.\n\n"
     "Volete confermare la cancellazione di ogni file singolo, oppure "
     "possono essere tutti cancellati automaticamente?"
))

(set #delete-old-help
(cat "\nLe versioni precedenti hanno numerosi file che non sono "
     "utilizzati nella Versione�3.1. Questi file dovrebbero essere "
     "cancellati per evitare incogruenze e salvare spazio su disco.\n\n"
     "Il pulsante \"Cancella automaticamente\" canceller� tutti i file di"
     "sistema obsoleti senza chiedere la vostra conferma su nessuno di essi. "
     "Il pulsante \"Conferma cancellazione\" forzer� il processo di "
     "installazione a chiedere una conferma prima di cancellare "
     "qualsiasi file. Questo vi permette di conservare specifici file che "
     "non volete cancellare.\n\n"
))

(set #delete-old-1
(cat "Cancella automaticamente"
))

(set #delete-old-2
(cat "Conferma cancellazione"
))

(set #deleting-obsolete
(cat "\n\nCancellazione file obsoleti..."
))

(set #confirm-delete
(cat "\n\nIl file \"%s\" pu� essere cancellato?"
))

(set #confirm-delete-help
(cat "\nIl file \"%s\" non � pi� usato nella Versione 3.1. Potete "
     "cancellare il file col pulsante \"S�\", o conservarlo selezionando il "
     "pulsante \"No\". "
))

(set #confirm-delete-reason-1
(cat "Questo file � incorporato nella ROM Versione 3.1"
))

(set #confirm-delete-reason-2
(cat "Le funzionalit� di questo programma sono state integrate in altri "
     "programmi del Workbench."
))

(set #confirm-delete-reason-3
(cat "Questo programma � stato copiato in una diversa zona del disco "
     "e l� verr� aggiornato."
))

(set #confirm-delete-reason-4
(cat "Questo programma � obsoleto e non dovrebbe essere usato con la "
     "Versione 3.1"
))

(set #confirm-delete-1
(cat "S�"
))

(set #confirm-delete-2
(cat "No"
))

(set #preparing-icons
(cat "\n\nPreparazione icone..."
))

(set #keeping-old
(cat "\n\nCopia di sicurezza di file importanti..."
))

(set #ask-workbench
(cat "\n\nInserire il disco \"Amiga Workbench\" Versione�3.1 in una unit� a dischi."
))

(set #ask-extras
(cat "\n\nInserire il disco \"Amiga Extras\" Versione�3.1 in una unit� a dischi."
))

(set #ask-storage
(cat "\n\nInserire il disco \"Amiga Storage\" Versione�3.1 in una unit� a dischi."
))

(set #which-printer
(cat "Quale programma di controllo della stampante deve essere installato? "
))

(set #which-printer-help
(cat "\nAmiga pu� usare varie stampanti. "
     "I programmi di controllo permettono di usare la stampante voluta. "
     "Per ogni stampante supportata bisogna copiare su disco fisso "
     "il relativo programma di controllo.\n\n"
     "Per ridurre lo spazio occupato su disco da questi programmi, "
     "potete installare solo il programma di controllo che pilota "
     "la vostra stampante.\n\n"
     "Segnate solo i riquadri delle stampanti che saranno collegate "
     "al vostro sistema.\n\n"
     @askoptions-help
))

(set #which-keymap
(cat "Quale mappa di tastiera deve essere installata?"
))

(set #which-keymap-help
(cat "Amiga pu� operare con tipi differenti di tastiera. Un file di mappa "
     "della tastiera permette la corretta interazione della stessa con "
     "Amiga. Scegliete il tipo di tastiera in vostro possesso dalla lista "
     "di quelle disponibili.\n\n"
     "Apporre il visto nella casella vicino alle tastiere che volete "
     "siano disponibili sul vostro sistema.\n\n"
     @askoptions-help
))

(set #which-language
(cat "\nQuali lingue devono essere installate? "
))

(set #which-language-help
(cat "\nAmiga pu� operate con lingue differenti. "
     "Per ogni lingua supportata saranno copiati "
     "numerosi file sul disco fisso.\n\n"
     "Per ridurre lo spazio occupato da questi "
     "file, potete installare solo quelli di una "
     "lingua specifica.\n\n"
     "Segnate solo i riquadri delle lingue che volete "
     "siano disponibili sul vostro sistema.\n\n"
     @askoptions-help
))

(set #ask-locale
(cat "\n\nInserire il disco \"Amiga Locale\" Versione�3.1 in una unit� a dischi. "
))

(set #no-font-space
(cat "\nNon c'� abbastanza spazio su \"%s\" per inserire tutte le serie "
     "di caratteri standard per Amiga.\n\n"
     "Potete copiare le serie di caratteri come ultima operazione "
     "trascinando l'icona del disco \"Amiga Fonts\" Versione 3.1 "
     "sull'icona di \"%s\"."
))

(set #ask-fonts
(cat "\n\nInserire il disco \"Amiga Fonts\" Versione�3.1 in una unit� a dischi. "
))

(set #ask-install
(cat "\n\nInserire il disco \"Amiga Install\" Versione�3.1 in una unit� a dischi. "
))

(set #reorganizing
(cat "\n\nRegolazione della configurazione come da quella precedente..."
))

(set #a2090
(cat "Se state usando un controller per disco fisso A2090 con un disco "
     "fisso SCSI e partite da dischetto, dovrete installare il file "
     "Startup-A2090SCSI, nel cassetto Update di questo disco, come "
     "Startup-Sequence nel dischetto di partenza. "
     "Se state usando un controller per disco fisso A2090 con un disco "
     "fisso ST-506 e partite da dischetto, dovrete installare il file "
     "Startup-A2090ST506, nel cassetto Update di questo disco, come "
     "Startup-Sequence nel dischetto di partenza. "
))

(set #reboot
(cat "\nL'installazione della Versione 3.1 � completa.\n\n"
     "Per abilitare la Versione 3.1, dovrete riavviare Amiga.\n\n"
     "Togliere il disco \"Amiga Install\" dalla unit� a dischetti "
     "e selezionare il pulsante \"Procedi\" per riavviare Amiga."
))

))


;=============================================================================
; Danish strings

(if (= @language "dansk")
(
(set default_lang 1)

(set #bad-kick
(cat "De skal benytte Kickstart 3.0 ved installation af Workbench�3.1"
))

(set #introduction
(cat "\n\nMed dette program kan De installere Workbench 3.1 "
     "p� en harddisk. Programmet kan benyttes til at opdatere en "
     "tidligere Workbench version, til grundinstallation, eller til at "
     "kontrollere hvilke sprog der er tilg�ngelige p� en harddisk, hvor "
     "Workbench 3.1 allerede er installeret."
))

(set #ask-function
(cat "\n\nV�lg \"Installere Workbench 3.1\" for at lave en komplet "
     "installation af Workbench 3.1, eller v�lg \"Opdatering af sprog\" for at "
     "�ndre hvilke sprog der er tilg�ngelige p� en harddisk, hvor "
     "Workbench 3.1 allerede er installeret."
))

(set #ask-function-1
(cat "Installere Workbench 3.1"
))

(set #ask-function-2
(cat "Opdatering af sprog"
))

(set #ask-function-help
(cat "\nDe skal v�lge hvilken operation De �nsker at udf�re.\n\n"
     "Klik p� \"Installere Workbench 3.1\" knappen for at udf�re en komplet "
     "installation af Workbench 3.1 softwaren. V�lg denne mulighed "
     "n�r systemet skal opgraderes fra en tidligere version, eller n�r der "
     "installeres Workbench 3.1 p� en ny harddisk.\n\n"
     "Deres Amiga computer kan kommunikere p� mange forskellige sprog. "
     "Der skal kopieres adskillige filer til Deres harddisk "
     "for hvert sprog De �nsker at benytte.\n\n"
     "Klik p� \"Opdatering af sprog\" knappen for at se hvilke sprog der er "
     "installeret og klar til brug p� harddisken. "
     "V�lg kun denne mulighed n�r Workbench 3.1 allerede er installeret p� "
     "harddisken.\n\n"
))

(set #confirm-target
(cat "Dette program vil opdatere Deres harddisk til Workbench�3.1.\n\n"
     "Deres nuv�rende Workbench filer vil blive overskrevet, "
     "dog vil Startup-Sequence og Shell-Startup filerne "
     "blive bibeholdt.\n\n"
     "Deres Workbench filer synes at v�re placeret i     \n\"%s\" "
     "skuffen.\n\n"
     "�nsker De ogs� Workbench 3.1 opdateringen placeret i denne skuffe?"
))

(set #confirm-target-help
(cat "\nInstallations programmet har fundet ud af at Deres nuv�rende "
     "Workbench findes i skuffen \"%s\".\n\nHvis De forts�tter med "
     "installationen vil de fleste af filerne i denne skuffe blive "
     "erstattet med nyere filer.\nNogle filer bliver f�rst kopieret til "
     "et sikkert sted f�r deres erstatning installeres. Dette omfatter "
     "den nuv�rende Startup-Sequence "
     "og Shell-Startup filerne."
))

(set #confirm-target-lang
(cat "\n\nDe tilg�ngelige Workbench 3.1 sprogfiler p� Deres harddisk vil nu "
     "blive opdateret. Deres Workbench 3.1 filer synes at v�re placeret "
     "i \"%s\" skuffen.\n\n"
     "Er dette korekt?"
))

(set #confirm-target-lang-help
(cat "\nInstallations programmet har fundet Deres nuv�rende Workbench i "
     "skuffen \"%s\". Hvis De forts�tter installationen vil Workbench sprog"
     "filerne blive kopieret dertil."
))

(set #which-disk-lang
(cat "I hvilken skuffe skal Workbench 3.1 sprogfilerne installeres?"
))

(set #which-disk-lang-help
(cat "\nDette afsnit lader Dem v�lge i hvilken harddisk skuffe Workbench 3.1 "
     "sprogfilerne, skal installeres. Disse skal ligge i det samme omr�de, "
     "som Workbench 3.1 filerne.\n\n"
))

(set #reboot-lang
(cat "\nOpdateringen af Workbench 3.1 sprog er nu f�rdig.\n\n"
     "For at benytte disse nye sprog, skal De slukke og t�nde Deres Amiga igen. "
     "Efter genopstart, skal De k�rer \"Locale\" pr�ference editoren og v�lge "
     "det sprog De �nsker at bruge.\n\n"
     "Fjern \"Amiga Install\" disketten fra floppy drevet, "
     "og klik p� \"Forts�tte\" knappen for at resette Deres Amiga."
))

(set #which-disk
(cat "P� hvilken disk/i hvilken skuffe\n�nsker De at installere Workbench�3.1 ?"
))

(set #which-disk-help
(cat "\nHer skal De v�lge i hvilken skuffe eller p� hvilken disk "
     "Workbench 3.1 filerne skal installeres.\n\nNormalt b�r de installeres "
     "samme sted som de nuv�rende Workbench 1.3 eller Workbench 2.0 filer.\n\n"
     @askdir-help
))

(set #old-name
(cat "Gamle_filer"
))

(set #move-old
(cat "\n\nM� de f�lgende filer flyttes til \"%s\" skuffen?\n\n%s"
))

(set #move-old-help
(cat "\nInstallationen af Workbench�3.1 vil erstatte nogle af de "
     "nuv�rende Workbench filer, inklusive Startup-Sequence "
     "og Shell-Startup filer. "
     "For at bibeholde tidligere �ndringer De m�tte have lavet p� disse "
     "filer, kopierer installations programmet dem til \"%s\" skuffen "
     "hvorfra de kan benyttes senere."
))

(set #move-old-1
(cat "Flytte"
))

(set #move-old-2
(cat "Hoppe over denne del"
))

(set #checking-target
(cat "\n\nUnders�ger \"%s\" for overfl�dige filer..."
))

(set #delete-old
(cat "\n\nMange af de filer, der benyttes af systemet under Workbench�1.3 "
     "og Workbench�2.0, er ikke l�ngere n�dvendige i Workbench�3.1. "
     "Disse filer kan slettes fra Deres system.\n\n"
     "�nsker De at blive stillet sp�rgsm�l omkring sletning af hver fil, "
     "eller m� de slettes automatisk?"
))

(set #delete-old-help
(cat "\nWorkbench�1.3 og Workbench�2.0 indeholder adskillige filer som "
     "ikke benyttes i Workbench�3.1. Disse filer b�r slettes for at undg� "
     "rod, og spare plads p� Deres harddisk.\n\n"
     "\"Sp�rge f�r sletning\" knappen tvinger systemet til at sp�rge Dem "
     "om tilladelse f�r sletning af hver enkelt fil. Derved har De mulighed "
     "for at beholde bestemte filer, som De ikke �nsker at slette.\n\n"
     "\"Slette automatisk\" knappen giver systemet lov til at slette filerne, "
     "uden f�rst at bede om Deres tilladelse p� nogen af filerne."
))

(set #delete-old-1
(cat "Slette automatisk"
))

(set #delete-old-2
(cat "Sp�rge f�r sletning"
))

(set #deleting-obsolete
(cat "\n\nSletter overfl�dige filer"
))

(set #confirm-delete
(cat "\n\nM� filen \"%s\" slettes?"
))

(set #confirm-delete-help
(cat "\nInstallations programmet har fundet ud af, at filen \"%s\" ikke "
     "har nogen funktion under Workbench 3.1. De kan slette filen ved at "
     "v�lge \"Ja\" knappen, eller gemme filen ved at v�lge \"Nej\" knappen."
))

(set #confirm-delete-reason-1
(cat "Denne fil er nu integreret i Kickstart 3.1 ROM�en."
))

(set #confirm-delete-reason-2
(cat "Dette programs funktioner er nu integreret i andre Workbench "
     "programmer."
))

(set #confirm-delete-reason-3
(cat "Dette program er flytte til et andet sted og vil blive opdateret der."
))

(set #confirm-delete-reason-4
(cat "Dette program er overfl�digt og m� ikke bruges sammen med "
     "Workbench 3.1."
))

(set #confirm-delete-1
(cat "Ja"
))

(set #confirm-delete-2
(cat "Nej"
))

(set #preparing-icons
(cat "\n\nKlarg�r ikoner..."
))

(set #keeping-old
(cat "\n\nLaver backup kopi af vigtige filer..."
))

(set #ask-workbench
(cat "\n\nInds�t venligst \"Amiga Workbench\" disken Version�3.1 i et floppy drev."
))

(set #ask-extras
(cat "\n\nInds�t venligst \"Amiga Extras\" disken Version�3.1 i et floppy drev."
))

(set #ask-storage
(cat "\n\nInds�t venligst \"Amiga Storage\" disken Version�3.1 i et floppy drev."
))

(set #which-printer
(cat "Hvilke(n) printer-driver(e) skal installeres?"
))

(set #which-printer-help
(cat "\nDeres Amiga kan kommunikere med mange forskellige printere. "
     "Printer-drivere tilpasser Amiga til en given printer. "
     "Der skal kopieres en printer-driver til harddisken for hver "
     "printer De �nsker i anvendelse.\n\n"
     "For at reducere pladsen, der optages af printer-drivere, "
     "kan De v�lge kun at kopiere printer-drivere til de printere, "
     "De �nsker at benytte.\n\n"
     "Klik p� knapperne ud for hver af de printere De �nsker at have til r�dighed "
     "p� Deres system. Et 'flueben' indikerer en valgt printer.\n\n"
     @askoptions-help
))

(set #which-keymap
(cat "Hvilken type tastatur skal installeres?"
))

(set #which-keymap-help
(cat "\nDeres Amiga computer kan arbejde med mange forskellige tastatur typer. "
     "En tastaturtype fil beskriver Deres tastatur for Amiga computeren, s� "      "den kan arbejde korekt med det. V�lg den tastaturtype De har udfra "      "listens valgmuligheder.\n\n"
     "Klik p� knapperne ud for den �nskede tastaturtype. Et �flueben� "
     "indikerer den valgte tatstaturtype."
))

(set #which-language
(cat "\nHvilke(t) sprog �nskes installeret?"
))

(set #which-language-help
(cat "\nAmiga kan kommunikere p� mange forskellige sprog. "
     "Der skal kopieres adskillige filer til Deres harddisk "
     "for hvert sprog De �nsker at benytte.\n\n"
     "For at reducere pladsen, der optages af sprog-filer, "
     "kan De v�lge kun at kopiere sprog, som De �nsker at benytte.\n\n"
     "Klik p� knapperne ud for hvert sprog De �nsker at have til r�dighed "
     "p� Deres system. Et 'flueben' indikerer et valgt sprog.\n\n"
     @askoptions-help
))

(set #ask-locale
(cat "\n\nInds�t venligst \"Amiga Locale\" disken Version�3.1 i et floppy drev."
))

(set #no-font-space
(cat "\nDer er i �jeblikket ikke plads nok p� \"%s\" til standard Amiga "
     "fontene.\n\n"
     "Amiga fontene vil derfor ikke blive kopieret.\n\n"
     "De kan kopiere fontene senere ved at tr�kke ikonerne fra "
     "\"Amiga Fonts\" disken Workbench 3.1 ovenp� \"%s\" ikonet."
))

(set #ask-fonts
(cat "\n\nInds�t venligst \"Amiga Fonts\" disken Version�3.1 i et floppy drev."
))

(set #ask-install
(cat "\n\nInds�t venligst \"Amiga Install\" disken Version�3.1 i et floppy drev."
))

(set #reorganizing
(cat "\n\nJusterer filerne til at passe i Deres tidligere konfiguration..."
))

(set #a2090
(cat "Hvis De benytter en A2090 harddisk controller sammen med en SCSI "
     "harddisk og booter fra en diskette, skal De installerer filen "
     "Startup-A2090SCSI fra Update skuffen p� denne disk, som Deres "
     "Startup-Sequence fil p� boot disketten. "
     "Hvis De benytter A2090 harddisk controlleren sammen med en ST-506 "
     "harddisk og booter fra en diskette, skal De installerer filen "
     "Startup-S2090ST506 fra Update skuffen p� denne disk, som Deres "
     "Startup-Sequence fil p� boot disketten. "
))

(set #reboot
(cat "\nInstallationen af Workbench 3.1 er nu f�rdig.\n\n"
     "For at aktiverer Workbench 3.1, skal De slukke/t�nde Amiga "
     "computeren.\n\n"
     "Fjern \"Amiga Install\" disketten fra diskette drevet, og klik "
     "p� \"Forts�tte\" knappen for at reboote Deres Amiga computer."
))

))


;=============================================================================
; Swedish strings

(if (= @language "svenska")
(
(set default_lang 512)

(set #bad-kick
(cat "Du m�ste anv�nda Kickstart 3.0 f�r att kunna installera Workbench�3.1"
))

(set #introduction
(cat "\n\nDetta program l�ter dig installera Release 3.1 av Amiga Operativsystemet "
     "p� en h�rddisk. Det kan anv�ndas f�r att uppgradera en �ldre h�rddisk, "
     "installera fr�n b�rjan, eller f�r att styra vilka spr�k som skall "
     "g�ras tillg�ngliga p� en h�rddisk som redan har Release 3.1 installerad."
))

(set #ask-function
(cat "\n\n\V�lj \"Installera Release 3.1\" f�r att g�ra en komplett installation "
     "av Release 3.1, eller v�lj \"Uppdatera spr�k\" f�r att �ndra vilka "
     "spr�k som skall vara tillg�ngliga p� en h�rddisk med ett redan "
     "installerat system."
))

(set #ask-function-1
(cat "Installera Release 3.1"
))

(set #ask-function-2
(cat "Uppdatera spr�k"
))

(set #ask-function-help
(cat "\nDu m�ste v�lja vilken funktion du vill utf�ra.\n\n"
     "Om du v�ljer \"Installera Release 3.1\" s� kan du utf�ra en komplett "
     "installation av Release 3.1 Workbench. Du skall v�lja detta alternativ "
     "n�r du uppgraderar fr�n en tidigare version av operativsystemet, "
     "eller n�r du skall installera Release 3.1 p� en ny h�rddisk.\n\n"
     "Din Amiga kan anv�ndas p� flera olika spr�k. Flera filer m�ste kopieras "
     "till din h�rddisk f�r varje spr�k du vill anv�nda. V�lj \"Uppdatera "
     "spr�k\" f�r att styra vilka spr�k du vill ha tillg�ngliga p� din "
     "h�rddisk. Detta alternativ skall bara v�ljas d� du redan har "
     "installerat Release 3.\n\n"
))

(set #confirm-target
(cat "\n\nDetta program uppdaterar din h�rddisk till Release 3.1. Om du "
     "har n�gon tidigare version av operativsystemet p� din h�rddisk "
     "kommer denna att skrivas �ver. Vissa viktiga filer (inklusive "
     "startup-Sequence) kommer att sparas.\n\n"
     "Vill du installera Release 3.1 i l�dan \"%s\"?"
))

(set #confirm-target-help
(cat "\nInstalleraren har funnit att Release 3.1 b�r installeras i l�dan "
     "\"%s\". Om du forts�tter med installationen kommer de flesta av "
     "operativsystemets filer i denna l�da att skrivas �ver.\n\nN�gra "
     "filer kommer att kopieras till ett s�kert st�lle f�rst, bl.a "
     "Startup-Sequence."
))

(set #confirm-target-lang
(cat "De tillg�ngliga Release 3-spr�ken p� din h�rddisk kommer nu att "
     "uppdateras. Dina Release 3.1 Workbench-filer f�refaller att finnas "
     "p� partitionen \"%s\".\n\n�r detta korrekt?"
))

(set #confirm-target-lang-help
(cat "\nInstallationsprogrammet har fastslagit att din nuvarande Workbench "
     "finns p� partitionen \"%s\". Om du forts�tter med installationen "
     "kommer spr�kfilerna att kopieras dit."
))

(set #which-disk-lang
(cat "P� vilken partition skall Release 3.1 spr�kfilerna placeras?"
))

(set #which-disk-lang-help
(cat "\nDetta avsnitt l�ter dig v�lja p� vilken h�rddiskspartition du vill "
     "placera spr�kfilerna f�r Release 3.1. Dessa skall ligga p� samma "
     "partition som Release 3.1 Workbench-filerna.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nUppdateringen av spr�ken f�r Release 3.1 �r nu komplett.\n\n"
     "F�r att anv�nda de nya spr�ken m�ste du starta om din Amiga. Efter "
     "omstart m�ste du anv�nda Prefs-editorn \"Locale\" f�r att v�lja det "
     "eller de spr�k du vill anv�nda.\n\n"
     "Ta ur disketten \"Amiga Install\" ur diskettstationen och v�lj "
     "\"Forts�tt\" f�r att starta om Amigan."
))

(set #which-disk
(cat "P� vilken enhet eller i vilken l�da skall Release 3.1 installeras?"
))

(set #which-disk-help
(cat "\nH�r kan du v�lja vilken enhet (eller l�da) filerna f�r Release 3.1 "
     "skall installeras p�. Normalt (i n�stan alla fall) skall detta vara "
     "samma st�lle som d�r de tidigare filerna f�r Workbench 1.3, 2.0 "
     "eller 2.1 finns.\n\n"
     @askdir-help
))

(set #old-name
(cat "OLD"
))

(set #move-old
(cat "\n\nKan f�ljande filer flyttas till l�dan \"%s\"?\n\n%s"
))

(set #move-old-help
(cat "Installation av Release 3.1 skriver �ver de flesta av dina "
     "nuvarande Workbench-filer, bl.a Startup-Sequence. "
     "F�r att bevara alla �ndringar du tidigare gjort av dessa filer "
     "s� kopierar Installeraren dem till l�dan \"%s\" d�r du senare kan "
     "h�mta dem."
))

(set #move-old-1
(cat "Flytta"
))

(set #move-old-2
(cat "Hoppa �ver"
))

(set #checking-target
(cat "Kontrollerar \"%s\" efter f�r�ldrade filer..."
))

(set #delete-old
(cat "\n\nM�nga filer som anv�nds av systemet under tidigare versioner "
     "av operativsystemet beh�vs inte l�ngre under Release 3.1. "
     "Dessa filer kommer nu att raderas; vill du tillfr�gas innan filerna "
     "raderas eller vill du att de skall raderas automatiskt?"
))

(set #delete-old-help
(cat "\nTidigare versioner av operativsystemet har inneh�llit ett flertal "
     "filer som inte l�ngre beh�vs under Release 3.1. Dessa filer b�r "
     "raderas f�r att undvika f�rvirring och spara utrymme p� h�rddisken.\n\n"
     "Knappen \"Radera automatiskt\" g�r s� att Installeraren automatiskt "
     "raderar de gamla filerna, utan att be om tillst�nd f�rst."
     "Knappen \"Fr�ga innan radering\" tvingar Installeraren att be om "
     "till�telse innan den raderar varje fil. Detta ger dig m�jlighet att "
     "spara speciella filer som du vill ha kvar.\n\n"
))

(set #delete-old-1
(cat "Radera automatiskt"
))

(set #delete-old-2
(cat "Fr�ga innan radering"
))

(set #deleting-obsolete
(cat "\n\nRaderar f�r�ldrade filer"
))

(set #confirm-delete
(cat "\n\nOK att radera filen \"%s\"?"
))

(set #confirm-delete-help
(cat "\nInstalleraren har funnit att filen \"%s\" inte l�ngre beh�vs "
     "under Release 3.1. Du kan radera filen genom att v�lja \"Ja\" "
     "eller spara filen genom v�lja \"Nej\"."
))

(set #confirm-delete-reason-1
(cat "Denna fil �r numer en del av Release 3.1 ROM (Kickstart)."
))

(set #confirm-delete-reason-2
(cat "Funktionen hos detta program �r numer en del av andra Workbench-"
     "program."
))

(set #confirm-delete-reason-3
(cat "Detta program har flyttats till en annan position, och den "
     "nya versionen kommer att placeras d�r."
))

(set #confirm-delete-reason-4
(cat "Det h�r programmet �r f�r�ldrat och skall inte anv�ndas med Release "
     "3.1"
))

(set #confirm-delete-1
(cat "Ja"
))

(set #confirm-delete-2
(cat "Nej"
))

(set #preparing-icons
(cat "\n\nSt�ller in symboler..."
))

(set #keeping-old
(cat "\n\nG�r backup-kopior av viktiga filer..."
))

(set #ask-workbench
(cat "\n\nS�tt i disketten \"Amiga Workbench\" version�3.1 i valfri diskettstation."
))

(set #ask-extras
(cat "\n\nS�tt i disketten \"Amiga Extras\" version�3.1 i valfri diskettstation."
))

(set #ask-storage
(cat "\n\nS�tt i disketten \"Amiga Storage\" version�3.1 i valfri diskettstation."
))

(set #which-printer
(cat "Vilka drivrutiner f�r skrivare skall installeras?"
))

(set #which-printer-help
(cat "\nAmiga kan anv�nda m�nga olika modeller av skrivare. "
     "Skrivardrivrutiner �r filer som l�ter Amigan anpassa sig till "
     "en viss skrivare. Man m�ste kopiera en skrivardrivrutin till "
     "h�rddisken f�r varje skrivare som skall st�djas.\n\n"
     "F�r att minimera anv�nt h�rddiskutrymme kan du v�lja vilka "
     "drivrutiner du vill kopiera.\n\n"
     "Markera bara rutorna framf�r skrivarna du kan komma att "
     "vilja anv�nda.\n\n"
     @askoptions-help
))

(set #which-keymap
(cat "Vilka tangentbordsupps�ttningar skall installeras?"
))

(set #which-keymap-help
(cat "\nAmigan kan anv�ndas med flera olika typer av tangentbord. En "
     "tangentbordsupps�ttningsfil (keymap-fil) beskriver tangentbordets "
     "utseende f�r operativsystemet. V�lj typen av tangentbord du har "
     "fr�n listan.\n\n"
     "Markera bara rutorna framf�r de tangentbord du kan komma att "
     "vilja anv�nda.\n\n"
     @askoptions-help
))

(set #which-language
(cat "\nVilka spr�k skall installeras?"
))

(set #which-language-help
(cat "\nAmigan kan arbeta med flera olika spr�k. Ett flertal filer "
     "m�ste kopieras till h�rddisken f�r varje spr�k du vill kunna "
     "anv�nda.\n\n"
     "F�r att minimera h�rddiskutrymmet f�r spr�kfiler kan du "
     "v�lja att bara installera filerna f�r vissa spr�k.\n\n"
     "Markera bara rutorna f�r de spr�k du �nskar installera "
     "i din dator.\n\n"
     @askoptions-help
))

(set #ask-locale
(cat "\n\nS�tt i disketten \"Amiga Locale\" version�3.1 i valfri diskettstation."
))

(set #no-font-space
(cat "\nDet finns inte utrymme nog p� \"%s\" f�r att installera "
     "standardupps�ttningen med Amiga typsnitt.\n\n"
     "Amiga typsnittsupps�ttningen kommer inte att kopieras.\n\n"
     "Du kan kopiera typsnitten senare genom att sl�ppa symbolen f�r "
     "\"Amiga Fonts\" disketten Version 3.1 ovanp� symbolen \"%s\"."
))

(set #ask-fonts
(cat "\n\nS�tt in disketten \"Amiga Fonts\" version�3.1 i valfri diskettstation."
))

(set #ask-install
(cat "\n\nS�tt in disketten \"Amiga Install\" Version�3.1 i valfri diskettstation."
))

(set #reorganizing
(cat "Justerar filer f�r att passa din tidigare konfiguration..."
))

(set #a2090
(cat "Om du anv�nder en A2090 h�rddisk-controller med en SCSI h�rddisk "
     "och startar datorn fr�n en diskett skall du installer filen "
     "Startup-A2090SCSI fr�n Update-l�dan p� denna diskett som "
     "Startup-Sequence p� din uppstartningsdiskett.\n"
     "Om du anv�nder en A2090 h�rddisk-controller med en ST-506 h�rddisk "
     "och startar datorn fr�n en diskett skall du installer filen "
     "Startup-A2090ST506 fr�n Update-l�dan p� denna diskett som "
     "Startup-Sequence p� din uppstartningsdiskett.\n"
))

(set #reboot
(cat "\nInstallationen av Release 3.1 �r nu komplett.\n\n"
     "F�r att anv�nda Release 3.1 m�ste du starta om din Amiga.\n\n"
     "Ta ur disketten \"Amiga Install\" ur diskettenheten och v�lj "
     "\"Forts�tt\" f�r att starta om din Amiga."
))

))


;=============================================================================
; Norwegian strings

(if (= @language "norsk")
(
(set default_lang 128)

(set #bad-kick
(cat "Du m� ha Kickstart 3.0 for � installere Workbench�3.1"
))

(set #introduction
(cat "\n\nDette programmet lar deg installere versjon 3.1 av Amigas "
     "operativsystem p� harddisk. Det kan benyttes til � oppgradere en eldre "
     "utgave, til � installere fra grunnen av, eller til � endre hvilke spr�k "
     "som skal kunne benyttes p� en harddisk som allerede inneholder versjon 3.1"
))

(set #ask-function
(cat "\n\nVelg \"Installer versjon 3.1\" for � gjennomf�re en komplett "
     "installasjon av versjon 3.1, eller velg \"Oppdater spr�k\" for � endre "
     "hvilke spr�k som skal kunne benyttes p� en maskin som allerede "
     "inneholder versjon 3.1"
))

(set #ask-function-1
(cat "Installer versjon 3.1"
))

(set #ask-function-2
(cat "Oppdater spr�k"
))

(set #ask-function-help
(cat "\nDu m� velge hvilken operasjon du vil utf�re.\n\n"
     "Ved � velge knappen merket \"Installer versjon 3.1\" kan du f� utf�rt en "
     "komplett installasjon av utgave 3.1 av Workbench. Dette alternativet skal "
     "velges n�r du vil oppgradere fra en tidligere utgave, eller n�r du skal "
     "installere versjon 3.1 p� en ny harddisk.\n\n"
     "Amigaen kan operere p� mange forskjellige spr�k. "
     "For hvert spr�k som skal kunne benyttes, m� det "
     "kopieres inn mange filer p� harddisken. Ved � velge "
     "\"Oppdater spr�k\" kan du bestemme hvilke spr�k som "
     "skal kunne benyttes p� din maskin. Denne operasjonen kan bare velges "
     "hvis versjon 3.1 allerede er installert.\n\n"
))

(set #confirm-target
(cat "\nWb 3.1 vil n� bli installert p� harddisken din. Filene fra en "
     "eldre utgave vil bli erstattet. Eventuelle viktige filer i den "
     "gamle utgaven vil bli beholdt, herunder din Startup-Sequence.\n\n"
     "�nsker du � installere Workbench 3.1 p� \"%s\"-partisjonen?"
))

(set #confirm-target-help
(cat "\nInstalleringsprogrammet har funnet ut at utgave 3.1 antakelig skal "
     "installeres p� \"%s\"-partisjonen p� harddisken. Hvis du fortsetter "
     "denne prosedyren, vil eventuelle Workbench-filer p� denne partisjonen "
     "bli erstattet med nye.\n\n"
     "Noen filer vil f�rst bli kopiert til et sikkert sted f�r de nye versjonene "
     "blir installert, herunder Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nDe tilgjengelige spr�kene for Workbench vil n� bli oppdatert. "
     "Filene for versjon 3.1 av Workbench ser ut til � ligge p� "
     "\"%s\"-partisjonen p� harddisken.\n\n"
     "Er dette korrekt?"
))

(set #confirm-target-lang-help
(cat "\nInstallasjonsprogrammet har funnet ut at din kopi av Workbench "
     "ligger p� \"%s\"-partisjonen p� harddisken. Hvis du fortsetter med "
     "installasjonen, vil spr�kfilene for Workbench bli kopiert dit. "
))

(set #which-disk-lang
(cat "P� hvilken partisjon skal spr�kfilene til utgave 3.1 bli installert?"
))

(set #which-disk-lang-help
(cat "\nHer kan du velge hvilken harddisk-partisjon spr�kfilene for "
     "versjon 3.1 skal installeres p�. De b�r ligge p� samme partisjon som "
     "Workbench-filene for utgave 3.1\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nOppdateringen av spr�kfilene for Workbench 3.1 er n� ferdig.\n\n"
     "For � benytte disse spr�kene, m� du starte din Amiga p� nytt. Etter "
     "oppstarten, m� du kj�re \"Locale\"-programmet for � velge "
     "spr�k.\n\n"
     "Ta ut \"Amiga Install\"-disketten fra diskstasjonen, og velg "
     "\"Fortsett\"-knappen for � starte din Amiga om igjen."
))

(set #which-disk
(cat "P� hvilken partisjon �nsker du � installere Workbench�3.1 ?"
))

(set #which-disk-help
(cat "Her skal du velge hvilken partisjon filene til versjon 3.1 skal "
     "installeres p�. Normalt skal de plasseres p� samme sted som de "
     "n�v�rende filene for Wb1.3, Wb2.0 eller Wb2.1 .\n\n"
     @askdir-help
))

(set #old-name
(cat "Gamle_filer"
))

(set #move-old
(cat "\n\nKan f�lgende filer flyttes til \"%s\"-skuffen?\n\n%s"
))

(set #move-old-help
(cat "\nInstallasjonen av Workbench�3.1 vil erstatte de fleste "
     "Workbench-filene, inklusive Startup-Sequence. "
     "For � beholde tidligere endringer som du kan ha utf�rt i "
     "disse filene, kopieres de til \"%s\"-"
     "skuffen, hvor du senere kan finne dem."
))

(set #move-old-1
(cat "Flytt"
))

(set #move-old-2
(cat "Hopp over"
))

(set #checking-target
(cat "\n\nGjennoms�ker \"%s\" for un�dvendige filer..."
))

(set #delete-old
(cat "\nMange av filene systemet benyttet under tidligere "
     "utgaver, er ikke lenger n�dvendige for Workbench�3.1. "
     "Disse filene kan slettes fra systemet.\n\n"
     "�nsker du � f� sp�rsm�l om sletting for hver enkelt fil, "
     "eller skal de slettes automatisk?"
))

(set #delete-old-help
(cat "Tidligere versjoner har mange filer som det ikke lenger er bruk for "
     "under Workbench�3.1. Disse b�r slettes for � ung� rot, "
     "og for � spare plass p� harddisken.\n\n"
     "Knappen \"Slett automatisk\" s�rger for � slette disse filene uten "
     "� sp�rre deg om lov f�rst."
     "Knappen \"Sp�r f�r sletting\" vil s�rge for at installasjonsprogrammet "
     "sp�r deg om lov hver gang en fil skal slettes. Dette gj�r at "
     "du kan beholde spesielle filer som du ikke �nsker � slette.\n\n"
))

(set #delete-old-1
(cat "Slett automatisk"
))

(set #delete-old-2
(cat "Sp�r f�r sletting"
))

(set #deleting-obsolete
(cat "\n\nSletter un�dvendige filer"
))

(set #confirm-delete
(cat "\n\nKan filen \"%s\" slettes?"
))

(set #confirm-delete-help
(cat "\nFilen \"%s\" er ikke n�dvendig for Workbench 3.1 . Du kan slette "
     "denne filen ved � "
     "velge \"Ja\"-knappen, eller beholde den ved � velge \"Nei\"."
))

(set #confirm-delete-reason-1
(cat "Denne filen er n� integrert i versjon 3.1 ROM-brikken."
))

(set #confirm-delete-reason-2
(cat "Dette programmets funksjon er n� integrert i andre Workbench-programmer."
))

(set #confirm-delete-reason-3
(cat "Dette programmet ble flyttet til et annet sted, og vil bli oppdatert "
     "der."
))

(set #confirm-delete-reason-4
(cat "Dette programmet er un�dvendig, og skal ikke benyttes med versjon 3.1"
))

(set #confirm-delete-1
(cat "Ja"
))

(set #confirm-delete-2
(cat "Nei"
))

(set #preparing-icons
(cat "\n\nKlargj�r ikonene..."
))

(set #keeping-old
(cat "\n\nLager sikkerhetskopier av viktige filer..."
))

(set #ask-workbench
(cat "\n\nSett inn disketten \"Amiga Workbench\" Version�3.1 i en diskettstasjon."
))

(set #ask-extras
(cat "\n\nSett inn disketten \"Amiga Extras\" Version�3.1 i en diskettstasjon."
))

(set #ask-storage
(cat "\n\nSett inn disketten \"Amiga Storage\" Version�3.1 i en diskettstasjon."
))

(set #which-printer
(cat "Hvilke skriverdefinisjoner skal installeres?"
))

(set #which-printer-help
(cat "Amiga kan benytte mange forskjellige skrivere. "
     "Skriverdefinisjoner er filer som tilpasser din Amiga til "
     "en skriver. Det m� finnes en skriverdefinisjon p� "
     "harddisken for hver skrivertype som skal benyttes.\n\n"
     "For � redusere plassen som skriverdefinisjonene opptar, "
     "kan du velge ut de du trenger.\n\n"
     "Sett en hake ved de skriverne du vil "
     "benytte til maskinen din.\n\n"
     @askoptions-help
))

(set #which-keymap
(cat "Hvilken tastaturdefinisjon skal installeres?"
))

(set #which-keymap-help
(cat "Amiga kan opereres fra mange forskjellige tastaturer. "
     "En tastaturdefinisjon forklarer maskinen hvordan den skal reagere p� "
     "de forskjellige tastene. Velg tastaturtype fra listen over "
     "tilgjengelige alternativer.\n\n"
     "Sett en hake ved de tastaturfilene du vil ha tilgjengelige "
     "p� systemet ditt.\n\n"
     @askoptions-help
))

(set #which-language
(cat "\nHvilke spr�k skal installeres?"
))

(set #which-language-help
(cat "Amiga kan benytte mange forskjellige spr�k. "
     "Mange filer m� kopieres til harddisken for hvert "
     "spr�k som du installerer.\n\n"
     "For � redusere plassen som benyttes til spr�kfiler, "
     "kan du velge ut de spr�kene du vil ha tilgjengelige.\n\n"
     "Sett en hake ved de spr�kene du �nsker � ha "
     "tilgjengelige p� din maskin.\n\n"
     @askoptions-help
))

(set #ask-locale
(cat "\n\nSett inn disketten \"Amiga Locale\" Version�3.1 i en diskettstasjon."
))

(set #no-font-space
(cat "\nDet er for �yeblikket ikke plass p� \"%s\" til standard Amiga "
     "skrifttyper.\n\n"
     "Amiga skrifttyper vil ikke bli kopiert inn.\n\n"
     "Du kan kopiere skrifttypene senere ved f�rst � lage mer plass, "
     "for deretter � trekke ikonet til "
     "diketten \"Amiga Fonts\" Version 3.1 over \"%s\"-ikonet."
))

(set #ask-fonts
(cat "\n\nSett inn disketten \"Amiga Fonts\" Version�3.1 i en diskettstasjon."
))

(set #ask-install
(cat "\n\nSett inn disketten \"Amiga Install\" Version�3.1 i en diskettstasjon."
))

(set #reorganizing
(cat "\n\nJusterer filene s� de passer til din tidligere konfigurasjon..."
))

(set #a2090
(cat "Hvis du bruker en A2090 harddisk kontroller med SCSI harddisk og "
     "starter maskinen fra diskett, m� du installere filen Startup-A2090SCSI "
     "fra Update-katalogen p� denne disketten som Startup-Sequence "
     "p� disketten du starter maskinen fra."
     "Hvis du bruker en A2090 harddisk kontroller med en ST-506 harddisk og "
     "starter maskinen fra diskett, m� du installere filen Startup-A2090ST506 "
     "fra Update-katalogen p� denne disketten som Startup-Sequence "
     "p� disketten du starter maskinen fra."
))

(set #reboot
(cat "\nInstallasjonen av versjon 3.1 er n� ferdig.\n\n"
     "For � benytte versjon 3.1, m� du starte din Amiga om igjen.\n\n"
     "Ta ut \"Amiga Install\"-disketten fra diskstasjonen, "
     "og velg \"Fortsett\"-knappen for � starte din Amiga p� nytt."
))

))


;=============================================================================
; update existing files
;
; old_dir - name of directory where old files are
; new_dir - name of directory where new files are
; target_dir - name of directory where files from "new_dir" found in "old_dir"
;	       should be put

(procedure UPDATE
(
    (if (exists old_dir)
    (
	(foreach old_dir "~(#?.info)"
	(
	    (if (= (exists (tackon new_dir @each-name)) 1)
	    (
		(delete (tackon old_dir @each-name))
		(delete (cat (tackon old_dir @each-name) ".info"))

		(rename (tackon new_dir @each-name) (tackon target_dir @each-name))
		(rename (cat (tackon new_dir @each-name) ".info") (cat (tackon target_dir @each-name) ".info"))
	    ))
	))
    ))
))


;=============================================================================
; procedure to copy the language drivers and catalogs

(procedure DOLOCALE
(
    (set n 0)
    (while (set language (select n "dansk"
				   "deutsch"
				   "english"
				   "espa�ol"
				   "fran�ais"
				   "italiano"
				   "nederlands"
				   "norsk"
				   "portugu�s"
				   "svenska"
				   ""
			 )
	   )
    (
	(if (IN lang n)
	(
	    (if (<> 2 n)
	    (
		(copyfiles (source (cat (tackon localePath "Languages") "/" language ".language"))
			   (dest (tackon target "Locale/Languages"))
		)
	    ))

	    (makedir (tackon target (cat "Locale/Catalogs/" language)))
	    (copyfiles (source (cat (tackon localePath "Catalogs") "/" language "/Sys"))
		       (dest (tackon target (cat "Locale/Catalogs/" language "/Sys")))
		       (all)
	    )

	    (makedir (tackon target (cat "Locale/Help/" language)))
	    (copyfiles (source (cat (tackon localePath "Help") "/" language "/Sys"))
		       (dest (tackon target (cat "Locale/Help/" language "/Sys")))
		       (all)
	    )
	)
	(
	    (delete (tackon target (cat "Locale/Languages/" language ".language")))

	    (run (cat "Delete QUIET ALL \"" (tackon target (cat "Locale/Catalogs/" language)) "\""))
	    (run (cat "Delete QUIET ALL \"" (tackon target (cat "Locale/Help/" language)) "\""))
	))
	(set n (+ n 1))
    ))

    (makeassign "Locale3.1")
))


;=============================================================================
; procedure to call on exit...

(procedure TERMINATE (makeassign "Locale3.1")
		     (makeassign "Fonts3.1")
		     (makeassign "Workbench3.1")
		     (makeassign "Extras3.1")
		     (makeassign "Storage3.1")
		     (makeassign "Install3.1")
)


;=============================================================================
; termination stuff

(onerror (TERMINATE))


;=============================================================================
; make sure we are running under V39

(if (< (/ (getversion) 65536) 39)
(
    (abort #bad-kick)
))


;=============================================================================
; do a bit of set up...

(set askForWorkbench FALSE)
(set askForExtras    FALSE)
(set askForInstall   FALSE)
(set askForLocale    FALSE)
(set askForFonts     FALSE)
(set askForStorage   FALSE)
(set hdDisks FALSE)
(set found FALSE)

(set sourcePath (expandpath (pathonly @icon)))
(set sourcePath (pathonly (pathonly sourcePath)))

(if sourcePath
(
    (if (exists (set workbenchPath (tackon sourcePath "Workbench3.1")))
    (
	(if (exists (set extrasPath (tackon sourcePath "Extras3.1")))
	(
	    (if (exists (set installPath (tackon sourcePath "Install3.1")))
	    (
		(if (exists (tackon installPath "Storage"))
		(
		    ; running with a high-density build
		    (set hdDisks TRUE)

		    (set localePath	(tackon workbenchPath "Locale"))
		    (set fontPath	(tackon extrasPath    "Fonts"))
		    (set storagePath    (tackon installPath   "Storage"))
		)
		(
		    ; running with a low-density build
		    (set localePath	(tackon sourcePath "Locale"))
		    (set fontPath	(tackon sourcePath "Fonts"))
		    (set storagePath    (tackon sourcePath "Storage3.1"))
		))

		(set found TRUE)
	    ))
	))
    ))
))

(if (= found FALSE)
(
    (set askForWorkbench TRUE)
    (set askForExtras    TRUE)
    (set askForInstall   TRUE)

    (set workbenchPath "Workbench3.1:")
    (set extrasPath    "Extras3.1:")
    (set installPath   "Install3.1:")

    (if (exists "Install3.1:Storage")
    (
	; running with a high-density disk set
	(set hdDisks TRUE)

	(set localePath	    (tackon workbenchPath "Locale"))
	(set fontPath	    (tackon extrasPath    "Fonts"))
	(set storagePath    (tackon installPath   "Storage"))
    )
    (
	; running with a low-density disk set
	(set askForLocale    TRUE)
	(set askForFonts     TRUE)
	(set askForStorage   TRUE)

	(set localePath    "Locale:")
	(set fontPath      "Fonts:")
	(set storagePath   "Storage3.1:")
    ))
))

(delopts "oknodelete" "force" "askuser")
(run (cat "Resident " (tackon installPath "C/IconPos PURE")) (safe))
(run (cat "Resident " (tackon installPath "C/Delete PURE")) (safe))
(run (cat "Resident " (tackon installPath "C/Reboot PURE")) (safe))

(run (cat (tackon installPath "C") "/GuessBootDev >ENV:BootDev Install3.1:") (safe))
(set old_wb (getenv "BootDev"))
(set old_wb (substr old_wb 0 (- (strlen old_wb) 1)))
(set old_wb (expandpath old_wb))
(user 2)


;=============================================================================
; what do you want to do?

(message #introduction)

(if (askbool (prompt #ask-function)
	     (help #ask-function-help)
	     (choices #ask-function-1 #ask-function-2)
	     (default 0)
    )
(

;******************************************************************
;******************* FULL INSTALL *********************************
;******************************************************************

;=============================================================================
; say hi!

(user 0)
(welcome)
(set old_level @user-level)

;=============================================================================
; get target directory where update is to be installed

(if (= (strlen old_wb) 0)
(
    (user 2)
    (set target (askdir (prompt #which-disk)
			(help #which-disk-help)
			(default "")
			(disk)
		)
    )
    (user old_level)
)
(
    (if (askbool (prompt (#confirm-target old_wb))
		 (help (#confirm-target-help old_wb))
		 (default 1)
	)
    (
	(set target old_wb)
    )
    (
	(set target (askdir (prompt #which-disk)
			    (help #which-disk-help)
			    (default old_wb)
			    (disk)
		    )
	)
    ))
))

(set @default-dest target)
(set stash_old (tackon target #old-name))


;=============================================================================
; which languages should be installed?

(user 2)
(set lang (askoptions (prompt #which-language)
		      (help #which-language-help)
		      (choices "Dansk" "Deutsch" "English" "Espa�ol" "Fran�ais" "Italiano" "Nederlands" "Norsk" "Portugu�s" "Svenska")
		      (default default_lang)
	  )
)
(user old_level)


;=============================================================================
; which printer drivers should be installed?

(user 2)
(set prt (askoptions (prompt #which-printer)
		     (help #which-printer-help)
		     (choices "CalComp Printers"
			      "Canon BJ10"
			      "CBM MPS1000"
			      "Diablo 630"
			      "Epson Printers"
			      "Howtek PixelMaster"
			      "HP Printers"
			      "ImageWriter II"
			      "NEC PinWriter"
			      "Okimate Printers"
			      "PostScript"
			      "Seiko Printers"
			      "Sharp JX-730"
			      "Tektronix Printers"
			      "Toshiba Printers"
			      "Xerox 4020"
		     )
		     (default 0)
	  )
)
(user old_level)


;=============================================================================
; which keymaps should be installed?

(user 2)
(set km (askoptions (prompt #which-keymap)
		    (help #which-keymap-help)
		    (choices "American"
			     "British"
			     "Canadien Fran�ais"
			     "Dansk"
			     "Deutsch"
			     "Dvorak"
			     "Espa�ol"
			     "Fran�ais"
			     "Italiana"
			     "Norsk"
			     "Portugu�s"
			     "Schweiz"
			     "Suisse"
			     "Svenskt"
		    )
		    (default 0)
	  )
)
(user old_level)


;=============================================================================
; get the version of what is currently installed

(set old_version 0)
(if (exists (tackon target "Libs/version.library"))
(
    (set old_version (/ (getversion (tackon target "Libs/version.library")) 65536))
))


;=============================================================================
; make some new drawers

(makedir (tackon target "Prefs"))
(makedir (tackon target "Prefs/Env-Archive"))
(makedir (tackon target "Prefs/Env-Archive/Sys"))
(makedir (tackon target "Fonts"))
(makedir (tackon target "Expansion"))
(makedir (tackon target "WBStartup"))
(makedir (tackon target "Locale"))
(makedir (tackon target "Locale/Catalogs"))
(makedir (tackon target "Locale/Languages"))
(makedir (tackon target "Locale/Countries"))
(makedir (tackon target "Locale/Help"))
(makedir (tackon target "Classes"))
(makedir (tackon target "Classes/Gadgets"))
(makedir (tackon target "Classes/DataTypes"))
(makedir (tackon target "Classes/Images"))
(makedir (tackon target "Devs"))
(makedir (tackon target "Devs/Monitors"))
(makedir (tackon target "Devs/DataTypes"))
(makedir (tackon target "Devs/DOSDrivers"))
(makedir (tackon target "Devs/Printers"))
(makedir (tackon target "Devs/Keymaps"))
(makedir (tackon target "Storage") (infos))
(makedir (tackon target "Storage/DOSDrivers"))
(makedir (tackon target "Storage/Printers"))
(makedir (tackon target "Storage/Monitors"))
(makedir (tackon target "Storage/Keymaps"))
(makedir (tackon target "Storage/DataTypes"))

(complete 2)


;=============================================================================
; rename some old files so as not to overwrite them...

(set startup-sequence (tackon target "S/Startup-Sequence"))
(set startupII        (tackon target "S/StartupII"))
(set cli-startup      (tackon target "S/CLI-Startup"))
(set cd0              (tackon target "Devs/DOSDrivers/CD0"))
(set cd0-b            (tackon target "Storage/DOSDrivers/CD0"))

(set oldfiles "")
(if (exists startup-sequence)
(
    (set oldfiles (cat oldfiles startup-sequence "����\n"))
))

(if (exists startupII)
(
    (set oldfiles (cat oldfiles startupII "�����������\n"))   ; these are hard spaces
))

(if (exists cli-startup)
(
    (set oldfiles (cat oldfiles s/cli-startup "���������\n"))   ; these are hard spaces
))

(if (exists cd0)
(
    (set oldfiles (cat oldfiles cd0 "���\n"))
))

(if (exists cd0-b)
(
    (set oldfiles (cat oldfiles cd0-b "\n"))
))

(if (> (strlen oldfiles) 0)
(
    (if (askbool (prompt (#move-old stash_old oldfiles))
		 (help (#move-old-help stash_old))
		 (choices #move-old-1 #move-old-2)
		 (default 1)
	)
    (
	(makedir stash_old (infos))
	(makedir (tackon stash_old "S"))

	(if (exists startup-sequence)
	(
	    (delete (tackon stash_old "S/Startup-Sequence"))
	    (rename startup-sequence (tackon stash_old "S/Startup-Sequence"))
	))

	(if (exists startupII)
	(
	    (delete (tackon stash_old "S/StartupII"))
	    (rename startupII (tackon stash_old "S/StartupII"))
	))

	(if (exists cli-startup)
	(
	    (delete (tackon stash_old "S/CLI-Startup"))
	    (rename cli-startup (tackon stash_old "S/CLI-Startup"))
	))

	(if (exists cd0)
	(
            (makedir (tackon stash_old "Devs") (infos))
            (makedir (tackon stash_old "Devs/DOSDrivers") (infos))

	    (delete (tackon stash_old "Devs/DOSDrivers/CD0"))
	    (delete (tackon stash_old "Devs/DOSDrivers/CD0.info"))
	    (rename cd0 (tackon stash_old "Devs/DOSDrivers/CD0"))
	    (rename cd0 (tackon stash_old "Devs/DOSDrivers/CD0.info"))
	))

	(if (exists cd0-b)
	(
            (makedir (tackon stash_old "Storage") (infos))
            (makedir (tackon stash_old "Storage/DOSDrivers") (infos))

	    (delete (tackon stash_old "Storage/DOSDrivers/CD0"))
	    (delete (tackon stash_old "Storage/DOSDrivers/CD0.info"))
	    (rename cd0-b (tackon stash_old "Storage/DOSDrivers/CD0"))
	    (rename cd0-b (tackon stash_old "Storage/DOSDrivers/CD0.info"))
	))
    ))
))

(complete 4)


;=============================================================================
; delete files that are no longer needed

(set first TRUE)

(working (#checking-target target))

(set n 0)
(while (set thisfile (select n "0.info"
			       "2Preferences"
			       "0Shell.info"
			       "0C/.info"
			       "1C/Ask"
			       "1C/CD"
			       "2C/DiskDoctor"
			       "1C/Echo"
			       "1C/Else"
			       "1C/EndCLI"
			       "1C/EndIf"
			       "1C/EndSkip"
			       "1C/FailAt"
			       "1C/Fault"
			       "1C/FF"
			       "1C/GetEnv"
			       "1C/If"
			       "1C/Lab"
			       "1C/NewCLI"
			       "1C/NewShell"
			       "1C/Path"
			       "1C/Prompt"
			       "1C/Quit"
			       "1C/Resident"
			       "1C/Run"
			       "1C/SetEnv"
			       "1C/Skip"
			       "1C/Stack"
			       "1C/Why"
			       "0Libs/.info"
			       "1Libs/icon.library"
			       "1Libs/info.library"
			       "0Devs/.info"
			       "1Devs/Keymaps/usa1"
			       "4Devs/Keymaps/is"
			       "1Devs/ramdrive.device"
			       "0L/.info"
			       "1L/Disk-Validator"
			       "1L/Newcon-Handler"
			       "2L/Pipe-Handler"
			       "1L/Ram-Handler"
			       "1L/Shell-Seg"
			       "0Monitors/.info"
			       "0Monitors.info"
			       "0Prefs/.info"
			       "2Prefs/CopyPrefs"
			       "2Prefs/Preferences"
			       "2Prefs/WBConfig"
			       "0Prefs/Env-Archive/Sys/keyboard"
			       "0Prefs/Presets/.info"
			       "0System/.info"
			       "2System/AddMonitor"
			       "2System/BindMonitor"
			       "0System/CLI.info"
			       "0System/DiskCopy.info"
			       "1System/FastMemFirst"
			       "2System/Fountain"
			       "3System/InitPrinter"
			       "4System/MergeMem"
			       "2System/SetMap"
			       "0Utilities/.info"
			       "3Utilities/AutoPoint"
			       "3Utilities/Blanker"
			       "3Utilities/Calculator"
			       "4Utilities/ClockPtr"
			       "3Utilities/Cmd"
			       "2Utilities/Display"
			       "2Utilities/DoubleClick"
			       "3Utilities/Exchange"
			       "3Utilities/Fkey"
			       "3Utilities/GraphicDump"
			       "2Utilities/IconEd"
			       "2Utilities/IHelp"
			       "2Utilities/InstallPrinter"
			       "0Utilities/More.info"
			       "3Utilities/NoCapsLock"
			       "4Utilities/Notepad"
			       "3Utilities/PrintFiles"
			       "0Expansion/.info"
			       "0WBStartup/.info"
			       "2WBStartup/IHelp"
			       "4WBStartup/Mode_Names"
			       "0Tools/.info"
			       "3Tools/AutoPoint"
			       "3Tools/Blanker"
			       "3Tools/ClickToFront"
			       "0Tools/Commodities/.info"
			       "2Tools/Commodities/IHelp"
			       "2Tools/DoubleClick"
			       "3Tools/Exchange"
			       "3Tools/Fkey"
			       "4Tools/FreeMap"
			       "4Tools/IconMerge"
			       "2Tools/IHelp"
			       "2Tools/KeyToy"
			       "2Tools/KeyToy2000"
			       "3Tools/NoCapsLock"
			       "2Tools/Palette"
			       "0MonitorStore/.info"
			       "3MonitorStore/A2024"
			       "3MonitorStore/NTSC"
			       "3MonitorStore/PAL"
			       "3MonitorStore/Multiscan"
			       "0MonitorStore"
			       "2Fonts/_bullet/if.ss"
			       ""
		     )
       )
(
    (set n (+ n 1))

    (set reason (substr thisfile 0 1))
    (set thisfile (substr thisfile 1))

    (if (exists (tackon target thisfile))
    (
	(if (= first TRUE)
	(
	    (set query FALSE)
	    (if (>= @user-level 1)
	    (
		(set query TRUE)
		(if (askbool (prompt #delete-old)
			     (help #delete-old-help)
			     (choices #delete-old-1 #delete-old-2)
			     (default 1)
		    )
		(
		    (set query FALSE)
		    (working #deleting-obsolete)
		)
		(
		    (working (#checking-target target))
		))
	    ))
	    (set first FALSE)
	))

	(if (= query TRUE)
	(
	    (if (= (fileonly thisfile) ".info")
	    (
		(delete (tackon target thisfile))
	    )
	    (
		; these two lines serve to convert "reason" from a string to a number
		(set reason (+ reason 1))
		(set reason (- reason 1))

		(set reason (select reason ""
					   #confirm-delete-reason-1
					   #confirm-delete-reason-2
					   #confirm-delete-reason-3
					   #confirm-delete-reason-4
			    )
		)

		(if (askbool (prompt (cat (#confirm-delete (tackon target thisfile))
					  "\n\n"
					  reason
				     )
			     )
			     (help (#confirm-delete-help (tackon target thisfile)))
			     (choices #confirm-delete-1 #confirm-delete-2)
			     (default 1)
		    )
		(
		    (delete (tackon target thisfile))
		    (delete (cat (tackon target thisfile) ".info"))
		))

		(working (#checking-target target))
	    ))
	)
	(
	    (delete (tackon target thisfile))
	    (delete (cat (tackon target thisfile) ".info"))
	))
    )
    (
	(delete (cat (tackon target thisfile) ".info"))
    ))
))

(set n 0)
(while (set thisfile (select n "CalComp_ColorMaster"
			       "CalComp_ColorMaster2"
			       "CanonBJ10"
			       "CBM_MPS1000"
			       "Diablo_630"
			       "EpsonQ"
			       "EpsonX"
			       "EpsonXOld"
			       "Howtek_Pixelmaster"
			       "HP_DeskJet"
			       "HP_DeskJetOld"
			       "HP_LaserJet"
			       "HP_PaintJet"
			       "HP_ThinkJet"
			       "ImagewriterII"
			       "NEC_Pinwriter"
			       "Okidata_293I"
			       "Okidata_92"
			       "Okimate_20"
			       "PostScript"
			       "Seiko_5300"
			       "Seiko_5300a"
			       "Sharp_JX-730"
			       "Tektronix_4693D"
			       "Tektronix_4696"
			       "Toshiba_P351C"
			       "Toshiba_P351SX"
			       "Xerox_4020"
			       ""
		     )
       )
(
    (set n (+ n 1))

    (delete (tackon target (cat "Devs/Printers/" thisfile)))
    (delete (cat (tackon target (cat "Devs/Printers/" thisfile)) ".info"))

    (delete (tackon target (cat "Storage/Printers/" thisfile)))
    (delete (cat (tackon target (cat "Storage/Printers/" thisfile)) ".info"))
))

(set n 0)
(while (set thisfile (select n	"cdn"
				"ch1"
				"ch2"
				"d"
				"dk"
				"e"
				"f"
				"gb"
				"i"
				"n"
				"po"
				"s"
				"usa0"
				"usa2"
				"usa3"
				""
		     )
       )
(
    (set n (+ n 1))

    (delete (tackon target (cat "Devs/Keymaps/" thisfile)))
    (delete (cat (tackon target (cat "Devs/Keymaps/" thisfile)) ".info"))

    (delete (tackon target (cat "Storage/Keymaps/" thisfile)))
    (delete (cat (tackon target (cat "Storage/Keymaps/" thisfile)) ".info"))
))

(complete 8)


;=============================================================================
; unstick any icons on the target

(working #preparing-icons)

(set listInfos (cat "list \"" target "\" PAT=#?.info ALL LFORMAT \"IconPos >NIL: *\"%p%n*\" -1 -1\" >T:InstallTemp"))
(run listInfos)
(execute "T:InstallTemp")
(delete "T:InstallTemp")

(complete 10)


;=============================================================================
; now keep certain files that we want to restore later

(if (> old_version 35)
(
    (working #keeping-old)

    (makedir "T:KEEP")
    (makedir "T:KEEP/Prefs")
    (makedir "T:KEEP/Prefs/Env-Archive")
    (makedir "T:KEEP/Prefs/Env-Archive/Sys")
    (makedir "T:KEEP/Utilities")
    (makedir "T:KEEP/WBStartup")
    (makedir "T:KEEP/Tools")
    (makedir "T:KEEP/Tools/Commodities")
    (makedir "T:KEEP/Devs")
    (makedir "T:KEEP/Devs/DOSDrivers")
    (makedir "T:KEEP/Devs/Monitors")
    (makedir "T:KEEP/S")

    (set n 0)
    (while (set keep (select n "Disk.info"
			       "Prefs/Env-Archive/Sys/wbconfig.prefs"
			       "Prefs/Printer.info"
			       "S/Ed-Startup"
			       "S/BRUtab"
			       "S/HDBackup.config"
			       "S/Shell-Startup"
			       "System/Shell.info"
			       "Utilities/Clock.info"
			       "Utilities/MultiView.info"
			       "Tools/IconEdit.info"
			       "Tools/Commodities/Blanker.info"
			       "Tools/Commodities/ClickToFront.info"
			       "Tools/Commodities/CrossDOS.info"
			       "Tools/Commodities/Exchange.info"
			       "Tools/Commodities/FKey.info"
			       "WBStartup/Blanker.info"
			       "WBStartup/ClickToFront.info"
			       "WBStartup/Clock.info"
			       "WBStartup/CrossDOS.info"
			       "WBStartup/Exchange.info"
			       "WBStartup/FKey.info"
			       "WBStartup/Shell.info"
			       "WBStartup/Cmd.info"
			       "WBStartup/Multiview.info"
			       ""
		     )
	   )
    (
	(if (exists (tackon target keep))
	(
	    (copyfiles (source (tackon target keep))
		       (dest (tackon "T:KEEP" (pathonly keep)))
		       (nogauge)
	    )
	))
	(set n (+ n 1))
    ))
))

(complete 12)


;=============================================================================
; now copy the Workbench disk

(if (= askForWorkbench TRUE)
(
    (askdisk (prompt #ask-workbench)
	     (help @askdisk-help)
	     (dest "Workbench3.1")
    )
))

(copyfiles (source workbenchPath)
	   (dest target)
	   (pattern "~(Locale)")
)

(complete 30)


;=============================================================================
; now copy the Locale disk

(if (= askForLocale TRUE)
(
    (askdisk (prompt #ask-locale)
	     (help @askdisk-help)
	     (dest "Locale")
	     (newname "Locale3.1")
    )
))

(copyfiles (source (tackon localePath "Countries"))
	   (dest (tackon target "Locale/Countries"))
	   (all)
)

(DOLOCALE)

(complete 45)


;=============================================================================
; now copy the Extras disk

(if (= askForExtras TRUE)
(
    (askdisk (prompt #ask-extras)
	     (help @askdisk-help)
	     (dest "Extras3.1")
    )
))

(copyfiles (source extrasPath)
	   (dest target)
	   (pattern "~(Fonts)")
)

(complete 60)


;=============================================================================
; now copy the Fonts disk

(set existingSize 0)
(set n 0)
(while (set fontName (select n "courier"
			       "diamond"
			       "emerald"
			       "garnet"
			       "helvetica"
			       "opal"
			       "ruby"
			       "sapphire"
			       "times"
			       "topaz"
			       "_bullet"
			       "_bullet_outlines"
			       ""
		     )
       )
(
    (if (exists (tackon target (cat "Fonts/" fontName)))
    (
	(set existingSize (+ existingSize (select n 60
						    30
						    35
						    25
						    70
						    20
						    35
						    30
						    60
						    10
						   685
						   575
					  )
			  )
	)
    ))
    (set n (+ n 1))
))

(set doFonts TRUE)
(if (< existingSize 1625)
(
    (if (< (/ (getdiskspace target) 512) (- 1625 existingSize))
    (
	(set doFonts FALSE)
	(message (#no-font-space target target target))
    ))
))

(if (= doFonts TRUE)
(
    (if (= askForFonts TRUE)
    (
	(askdisk (prompt #ask-fonts)
		 (help @askdisk-help)
		 (dest "Fonts")
		 (newname "Fonts3.1")
	)
    ))

    (copyfiles (source fontPath)
	       (dest (tackon target "Fonts"))
	       (all)
    )
))

(makeassign "Fonts3.1")
(complete 75)


;=============================================================================
; now copy stuff from the Storage disk

(if (= hdDisks TRUE)
(
    (if (= askForInstall TRUE)
    (
        (askdisk (prompt #ask-install)
                 (help @askdisk-help)
                 (dest "Install3.1")
        )
    ))
)
(
    (if (= askForStorage TRUE)
    (
        (askdisk (prompt #ask-storage)
                 (help @askdisk-help)
                 (dest "Storage3.1")
        )
    ))
))

(copyfiles (source storagePath)
	   (dest (tackon target "Storage"))
	   (pattern "#?.info")
)

(copyfiles (source (tackon storagePath "Monitors"))
	   (dest (tackon target "Storage/Monitors"))
	   (pattern "#?")
)

(copyfiles (source (tackon storagePath "DOSDrivers"))
	   (dest (tackon target "Storage/DOSDrivers"))
	   (pattern "#?")
)

(complete 79)


;=============================================================================
; do the printer driver schtick

(set n 0)
(set printerpat "(%")
(while (set printer (select n "CalComp_#?"
			      "CanonBJ10#?"
			      "CBM_MPS1000#?"
			      "Diablo_630#?"
			      "Epson#?"
			      "Howtek_Pixelmaster#?"
			      "HP#?"
			      "ImagewriterII#?"
			      "NEC_Pinwriter#?"
			      "Oki#?"
			      "PostScript#?"
			      "Seiko#?"
			      "Sharp_JX-730#?"
			      "Tektronix#?"
			      "Toshiba#?"
			      "Xerox_4020#?"
			      ""
		     )
       )
(
    (if (IN prt n)
    (
	(set printerpat (cat printerpat "|" printer))
    ))
    (set n (+ n 1))
))

(copyfiles (source (tackon storagePath "Printers"))
	   (dest (tackon target "Devs/Printers"))
	   (pattern (cat printerpat ")"))
)

(complete 82)


;=============================================================================
; do the keymap schtick

(set n 0)
(set keymappat "(%")
(while (set keymap (select n    "usa[23]"
				"gb#?"
				"cdn#?"
				"dk#?"
				"(d|d.info)"
				"usa2#?"
				"e#?"
				"f#?"
				"i#?"
				"n#?"
				"po#?"
				"ch2#?"
				"ch1#?"
				"s#?"
				""
		   )
       )
(
    (if (IN km n)
    (
	(set keymappat (cat keymappat "|" keymap))
    ))
    (set n (+ n 1))
))

(copyfiles (source (tackon storagePath "Keymaps"))
	   (dest (tackon target "Devs/Keymaps"))
	   (pattern (cat keymappat ")"))
)

(complete 85)


;=============================================================================
; we need the install disk back so we can copy some stuff from it

(if (= askForInstall TRUE)
(
    (askdisk (prompt #ask-install)
	     (help @askdisk-help)
	     (dest "Install3.1")
    )
))

(copyfiles (source (tackon installPath "Update/Disk.info"))
	   (dest target)
	   (nogauge)
)


;=============================================================================
; copy some extra stuff for HD folks...

(copyfiles (source (tackon installPath "HDTools"))
	   (dest (tackon target "Tools"))
	   (pattern "(hd|bru)#?")
)

(copyfiles (source (tackon installPath "HDTools/S"))
	   (dest (tackon target "S"))
	   (all)
	   (nogauge)
)

; oooo, an '040!
(copyfiles (source (tackon installPath "Libs/68040.library"))
	   (dest (tackon target "Libs"))
)

(if (exists (tackon target "C/MapROM"))
(
    (copyfiles (source (tackon installPath "Update/Startup-MapROM"))
	       (dest (tackon target "S"))
	       (newname "Startup-Sequence")
	       (nogauge)
    )
)
(
    (copyfiles (source (tackon installPath "Update/Startup-HardDrive"))
	       (dest (tackon target "S"))
	       (newname "Startup-Sequence")
	       (nogauge)
    )
))

(complete 88)


;=============================================================================
; update L:FastFileSystem

(if (exists (tackon target "L/FastFileSystem"))
(
    (copyfiles (source (tackon installPath "L/FastFileSystem"))
	       (dest (tackon target "L"))
    )
))

(complete 89)


;=============================================================================
; update A2232 serial driver
;
(if (exists (tackon target "Devs/oldser.device"))
(
    (copyfiles (source (tackon installPath "Update/oldser.device"))
	       (dest (tackon target "Devs"))
    )

    (copyfiles (source (tackon installPath "Update/serial.device"))
	       (dest (tackon target "Devs"))
    )

    (copyfiles (source (tackon installPath "Update/aux-handler"))
	       (dest (tackon target "L"))
    )

    (tooltype (dest (tackon target "Prefs/Serial"))
	      (settooltype "UNIT" "")
    )
))

(complete 90)


;=============================================================================
; if a 2090a, update their original SetPatch and S-S
;
(set oldHD (run (tackon installPath "C/Check2090") (safe)))

; check if A2090-A
(if (= oldHD 1)
(
    (copyfiles (source (tackon installPath "C/SetPatch"))
	       (dest "BOOT:C")
	       (nogauge)
    )

    (rename "BOOT:S/Startup-Sequence" "BOOT:S/OLD-Startup-Sequence")

    (copyfiles (source (tackon installPath "Update/Startup-2090A"))
	       (dest "BOOT:S")
	       (newname "Startup-Sequence")
	       (nogauge)
    )
))

(complete 91)


;=============================================================================
; now update certain files that were already installed in the system

(working #reorganizing)

(set old_dir (tackon target "Devs/Monitors"))
(set new_dir (tackon target "Storage/Monitors"))
(set target_dir (tackon target "Devs/Monitors"))
(UPDATE)

(set old_dir (tackon target "Monitors"))
(set new_dir (tackon target "Storage/Monitors"))
(set target_dir (tackon target "Devs/Monitors"))
(UPDATE)
(delete (tackon target "Monitors"))

(set old_dir (tackon target "Devs/DOSDrivers"))
(set new_dir (tackon target "Storage/DOSDrivers"))
(set target_dir (tackon target "Devs/DOSDrivers"))
(UPDATE)

(set old_dir (tackon target "WBStartup"))
(set new_dir (tackon target "Utilities"))
(set target_dir (tackon target "WBStartup"))
(UPDATE)

(set old_dir (tackon target "WBStartup"))
(set new_dir (tackon target "System"))
(set target_dir (tackon target "WBStartup"))
(UPDATE)

(set old_dir (tackon target "WBStartup"))
(set new_dir (tackon target "Tools"))
(set target_dir (tackon target "WBStartup"))
(UPDATE)

(set old_dir (tackon target "WBStartup"))
(set new_dir (tackon target "Tools/Commodities"))
(set target_dir (tackon target "WBStartup"))
(UPDATE)

(set frequency (database "vblank"))
(if (= frequency "60")
(
    (rename (tackon target "Storage/Monitors/NTSC") (tackon target "Devs/Monitors/NTSC"))
    (rename (tackon target "Storage/Monitors/NTSC.info") (tackon target "Devs/Monitors/NTSC.info"))
)
(
    (rename (tackon target "Storage/Monitors/PAL") (tackon target "Devs/Monitors/PAL"))
    (rename (tackon target "Storage/Monitors/PAL.info") (tackon target "Devs/Monitors/PAL.info"))
))

(makeassign "NEWWB" target)
(run (tackon installPath "C/UpdateWBFiles"))
(makeassign "NEWWB")

(complete 94)


;=============================================================================
; copy back stuff we had preserved

(if (exists "T:KEEP")
(
    (copyfiles (source "T:KEEP")
	       (dest target)
	       (all)
	       (nogauge)
    )
    (run "Delete QUIET ALL T:KEEP")
))

(complete 97)


;=============================================================================
; Clean up that mess young man!
;
(working #positioning-icons)

(run (cat "IconPos >NIL: \"" target "Prefs\"     12 20\n"))
(run (cat "IconPos >NIL: \"" target "Prefs/Printer\"  160 48\n"))

(run (cat "IconPos >NIL: \"" target "Utilities\" 98 4\n"))
(run (cat "IconPos >NIL: \"" target "Utilities/Clock\" 91 11\n"))
(run (cat "IconPos >NIL: \"" target "Utilities/MultiView\" 7 4\n"))

(run (cat "IconPos >NIL: \"" target "Tools\"                           98 38\n"))
(run (cat "IconPos >NIL: \"" target "Tools/IconEdit\"                 111 4\n"))
(run (cat "IconPos >NIL: \"" target "Tools/Commodities/Blanker\"        8 84\n"))
(run (cat "IconPos >NIL: \"" target "Tools/Commodities/ClickToFront\"  99 4\n"))
(run (cat "IconPos >NIL: \"" target "Tools/Commodities/CrossDOS\"      99 44\n"))
(run (cat "IconPos >NIL: \"" target "Tools/Commodities/Exchange\"       8  4\n"))
(run (cat "IconPos >NIL: \"" target "Tools/Commodities/FKey\"          99 84\n"))

(run (cat "IconPos >NIL: \"" target "System\"    184 4\n"))
(run (cat "IconPos >NIL: \"" target "WBStartup\" 184 38\n"))
(run (cat "IconPos >NIL: \"" target "Devs\"      270 4\n"))
(run (cat "IconPos >NIL: \"" target "Storage\"   270 38 DXPOS 480 DYPOS 77 DWIDTH 107 DHEIGHT 199\n"))
(run (cat "IconPos >NIL: \"" target "Storage/Monitors\"   10 106 DXPOS 480 DYPOS 77 DWIDTH 107 DHEIGHT 199\n"))
(run (cat "IconPos >NIL: \"" target "Storage/Printers\"   10 140 DXPOS 480 DYPOS 77 DWIDTH 107 DHEIGHT 199\n"))
(run (cat "IconPos >NIL: \"" target "Expansion\" 356 20\n"))
(run (cat "IconPos >NIL: \"" target "Disk\"      DXPOS 28 DYPOS 29 DWIDTH 452 DHEIGHT 93\n"))

(complete 100)


;=============================================================================
; Let me tell you a secret...

(if (>= oldHD 2)
(
    (message #a2090)
))


;=============================================================================
; "Say goodnight Gracy."

(user 2)
(message #reboot)
(run "Reboot")

)
(

;******************************************************************
;******************** LANGUAGE UPDATE *****************************
;******************************************************************

;=============================================================================
; get target directory where update is to be installed

(if (askbool (prompt (#confirm-target-lang old_wb))
	     (help (#confirm-target-lang-help old_wb))
	     (default 1)
    )
(
    (set target old_wb)
)
(
    (set target (askdir (prompt #which-disk-lang)
			(help #which-disk-lang-help)
			(default old_wb)
			(disk)
		)
    )
))

(set @default-dest target)
(set stash_old (tackon target #old-name))

(set lang (askoptions (prompt #which-language)
		      (help #which-language-help)
		      (choices "Dansk" "Deutsch" "English" "Espa�ol" "Fran�ais" "Italiano" "Nederlands" "Norsk" "Portugu�s" "Svenska")
		      (default default_lang)
	  )
)

(if (= askForLocale TRUE)
(
    (askdisk (prompt #ask-locale)
	     (help @askdisk-help)
	     (dest "Locale")
	     (newname "Locale3.1")
    )
))

(DOLOCALE)

(complete 100)


;=============================================================================
; "Say goodnight Gracy."
;
(message #reboot-lang)
(run "Reboot")

))
