; $Date: 94/03/01 16:43:53 $ $Revision: 42.0 $
; Script to install CD1200

(complete 0)

;=============================================================================
; English strings

(if (= @language "english")
(
(set default_lang 4)

(set #bad-kick
(cat "You must be using Kickstart 3.0 to install Workbench 3.1"
))

(set #introduction
(cat "\n\nThis program lets you install CD1200 tools and utilities "
     "on a hard drive. It can be used to upgrade an older release, "
     "to install from scratch, or to control which languages are available "
     "on a hard drive with the programs already installed."
))

(set #ask-function
(cat "\n\nSelect \"Install CD1200\" to do a complete installation "
     "or select \"Update Languages\" to modify which languages are "
     "available on a hard drive with the programs already installed."
))

(set #ask-function-1
(cat "Install CD1200"
))

(set #ask-function-2
(cat "Update Languages"
))

(set #ask-function-help
(cat "\nYou must choose which operation you would like to "
     "perform.\n\n"
     "Selecting the \"Install CD1200\" gadget lets you perform a complete "
     "installation of the CD1200 software. This option should "
     "be chosen when "
     "installing the software on a new hard drive unit.\n\n"
     "The Amiga can be operated in many different "
     "languages. Several files must be copied to your "
     "hard drive for each language supported. Selecting the "
     "\"Update Languages\" gadget lets you control which "
     "languages are installed and available for use on your hard drive. "
     "This option should only be chosen when the CD1200 software has already "
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
(cat "In which partition should the Release 3 language files be installed?"
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
(cat "In which partition should Release 3 be installed?"
))

(set #which-disk-help
(cat "\nThis section lets you choose on which hard drive partition the "
     "Release 3.1 files will be installed. These should normally go "
     "in the same location as the current Release 1.3, Release 2, or "
     "Release 2.1 files.\n\n"
     @askdir-help
))

(set #preparing-icons
(cat "\n\nPreparing icons..."
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

(set #reboot
(cat "\nThe installation of Release 3.1 is now complete.\n\n"
     "To enable Release 3.1, you must reboot your Amiga.\n\n"
     "Remove the \"Amiga Install\" disk from the floppy drive, "
     "and select the \"Proceed\" gadget to reboot your Amiga."
))

))


;=============================================================================
; Portugues strings

(if (= @language "português")
(
(set default_lang 256)

(set #bad-kick
(cat "Percisa de estar a usar o Kickstart 3.0 para instalar o Workbench 3.1"
))

(set #introduction
(cat "\n\nEste programa permite-lhe instalar a Versão 3.1 do Sistema Operativo "
     "do Amiga no seu disco rígido. Pode ser usado para fazer a substituição "
     "de uma versão anterior, para instalar do zero ou para controlar as "
     "linguagens que estão á sua disposição num disco rígido com a Versão 3.1 "     "já instalada."
     "já instalada."
))

(set #ask-function
(cat "\n\nSeleccione \"Instalar Versão 3.1\" para fazer uma instalação da "
     "Versão 3.1, ou seleccione \"Mudar Linguagens\" para modificar as "
     "linguagens que estão á sua disposição num disco rígido com a Versão 3.1 "
     "já instalada."
))

(set #ask-function-1
(cat "Instalar Versão 3.1"
))

(set #ask-function-2
(cat "Mudar Linguagens"
))

(set #ask-function-help
(cat "\nTerá que escolher qual é a operação que quer realizar.\n\n"
     "Seleccionando o botão \"Instalar Versão 3.1\" vai fazer uma instalação "
     "completa da Versão 3.1 do software do Workbench. Esta opção terá que ser "
     "escolhida quando se está a fazer a substituição de versões anteriores, "
     "ou se está a instalar a Versão 3.1 a partir do zero.\n\n"
     "O Amiga pode ser operado em muitas linguagens diferentes. "
     "Alguns ficheiros têm que ser copiados para o seu disco rígido, por cada "
     "uma das linguagens a serem suportadas pelo seu sistema. A selecção do "
     "botão \"Mudar Linguagens\" vai permitir-lhe controlar quais as "
     "linguagens que estão instaladas e prontas a serem usadas, no seu"
     "disco rígido. "
     "Esta opção deve ser usada apenas após a instalação da Versão 3.\n\n"
))

(set #confirm-target
(cat "\n\nEste programa vai actualizar o seu Hard-Disk para o Workbench 3.1. "
     "Os ficheiros do Workbench anterior serão apagados, embora a sua "
     "Startup-Sequence sejam preservadas. "
     "Os ficheiros do seu Workbench parecem estar na gaveta \"%s\".\n\n"
     "Quer actualizar os ficheiros na gaveta \"%s\"?"
))

(set #confirm-target-help
(cat "\nO programa de instalação determinou que a sua cópia actual do "
     "Workbench está na gaveta chamada \"%s\". Prosseguir com a instalção "
     "vai implicar a substituição dos ficheiros existentes por outros "
     "novos.\n\nAlguns destes serão copiados para um lugar seguro antes "
     "de serem substituidos. Eles são "
     "Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nAs linguagens da Versão 3.1 disponíveis seram mudadas agora."
     "Os ficheiros do seu Workbench Versão 3.1 parecem estar localizados "
     "na partição \"%s\".\n\n"
     "Está correcto?"
))

(set #confirm-target-lang-help
(cat "\nO programa de instalação determinou que a sua cópia corrente "
     "do Workbench se encontra na partição chamada \"%s\". A continuação "
     "do processo de instalação vai fazer com que os ficheiros de linguagem "
     "do Workbench sejam copiados para aí. "
))

(set #which-disk-lang
(cat "Para que partição deverão ser copiados os ficheiros de linguagem da Versão 3?"
))

(set #which-disk-lang-help
(cat "\nEsta parte permite-lhe escolher em que partição deverão ser instalados "
     "os ficheiros de linguagem da Versão 3.1. Estes deverão ir para o mesmo "
     "sítio onde se encontram os ficheiros da Versão 3.1 do Workbench.\n\n"
))

(set #reboot-lang
(cat "\nO processo de mudança das linguagens da Versão 3.1 está completo.\n\n"
     "Para poder usar as novas lingagens, terá que reinicializar o seu Amiga. "
     "Depois do arranque do computador terá que executar o programa de "
     "preferências \"Locale\" e escolher a linguagem que quizer utilizar.\n\n"
     "Retire a diskette \"Amiga Install\" da drive e seleccione \"Continuar\""
     "Para reinicializar o seu Amiga."
))

(set #which-disk
(cat "Em que disco ou gaveta se vai instalar o Workbench 3.1?"
))

(set #which-disk-help
(cat "\nEsta secção permite-lhe escolher em que gaveta ou disco os "
     "ficheiros do Workbench 3.1 serão instalados. Este normalmente irão "
     "para o mesmo lugar dos ficheiros do  Workbench 1.3 ou Workbench 2.0.\n\n"
     @askdir-help
))

(set #preparing-icons
(cat "\n\nPreparando os icones..."
))

(set #which-language
(cat "\nQue linguagens deverão ser instaladas?"
))

(set #which-language-help
(cat "\nO Amiga pode ser operado em muitas linguagens "
     "diferentes. Alguns ficheiros têm de ser copiados "
     "para o seu disco rígido por cada linguagem suportada.\n\n"
     "Para reduzir o consumo de espaço no disco pelas linguagens "
     "poderá apenas escolher aquelas que lhe serão úteis.\n\n"
     "Para isto basta apenas marcar as caixas com os nomes das "
     "linguagens para que estas sejam instaladas.\n\n"
     @askoptions-help
))

(set #reboot
(cat "\nO processo de instalação da Versão 3.1 está completo.\n\n"
     "Para a poder usar terá que reinicializar o seu Amiga.\n\n"
     "Retire a diskette \"Amiga Install\" da drive e seleccione "
     "o botão \"Continuar\" para reinicializar o seu Amiga."
))

))


;=============================================================================
; Spanish strings

(if (= @language "español")
(
(set default_lang 8)

(set #bad-kick
(cat "Necesita usar el Kickstart 3.0 para instalar el Workbench 3.1"
))

(set #introduction
(cat "\n\nEste programa le permite instalar la Versión 3.1 del Sist. Operativo "
     "de Amiga en el disco duro. Se puede utilizar para actualizar su "
     "sistema, instalar por primera vez o  controlar qué idiomas hay "
     "disponibles en el disco duro con la Versión 3.1 ya instalada."
))

(set #ask-function
(cat "\n\nElija \"Instalar Versión 3.1\" para instalar por completo la "
     "Versión 3.1, o \"Actualizar Idiomas\" para modificar los idiomas "
     "disponibles en el disco duro con la Versión 3.1 ya instalada."
))

(set #ask-function-1
(cat "Instalar Versión 3.1"
))

(set #ask-function-2
(cat "Actualizar Idiomas"
))

(set #ask-function-help
(cat "\nDebe elegir la operación que desea realizar.\n\n"
     "Si elige \"Instalar Versión 3.1\", llevará a cabo la instalación "
     "completa del Workbench Versión 3.1. Debe utilizar esta opción al "
     "actualizar la versión de su sistema, o cuando tenga que cargar "
     "la Versión 3.1 en un disco duro nuevo.\n\n"
     "Amiga se puede utilizar en diferentes idiomas. Hay que copiar "
     "algunos ficheros al disco duro para cada idioma. Si elige la "
     "opción \"Actualizar Idiomas\" podrá controlar qué idiomas están "
     "instalados y disponibles en el disco duro. Esta opción se debe "
     "usar cuando la Versión 3.1 ya ha sido instalada.\n\n"
))

(set #confirm-target
(cat "\nse va a instalar la Versión 3.1 en el disco duro. Se borrará "
     "la versión anterior del sistema operativo del disco duro."
     "Algunos ficheros de la versión anterior, no se borrarán, como "
     "el fichero Startup-Sequence.\n\n"
     "¿Quiere instalar la Versión 3.1 en la partición \"%s\"?"
))

(set #confirm-target-help
(cat "\nEl instalador ha determinado que la Versión 3.1 se instalará "
     "en la partición \"%s\". El continuar con la instalación hará "
     "que la mayoría de los ficheros de este directorio se reemplacen "
     "por los nuevos.\n\nAlgunos ficheros se copiarán a un lugar seguro "
     "antes de reemplazarlos, incluyendo el Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nLos idiomas disponibles de la Versión 3.1 en el disco duro ya"
     "se han actualizado. Los ficheros del Workbench 3.1 parece que están "
     "en la partición \"%s\" \n\n"
     "¿ Es correcto ?"
))

(set #confirm-target-lang-help
(cat "\nSu copia actual del Workbench está en la partición \"%s\". "
     "Si continúa con la instalación, los ficheros de idiomas del "
     "Workbench se copiarán a dicha partición. "
))

(set #which-disk-lang
(cat "¿En qué partición debería instalar los idiomas de la Versión 3?"
))

(set #which-disk-lang-help
(cat "\nEsta sección le permite elegir la partición del disco duro "
     "donde instalar los ficheros de idiomas de la Versión 3.1. Deben "
     "estar en el mismo lugar ue los ficheros del Workbench 3.1.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "Para utilizar los nuevos idiomas, debe arrancar su Amiga. Después, "
     "tiene que ejecutar el editor de preferencias \"Locale\" y elegir "
     "el idioma que prefiere utilizar.\n\n"
     "Retire el disco \"Amiga Install\" de la unidad y seleccione "
     "\"Seguir\" para reinicializar el Amiga."
))

(set #which-disk
(cat "¿En qué unidad o directorio quiere instalar el Workbench 3.1?"
))

(set #which-disk-help
(cat "\nEsta sección le permite elegir en qué unidad o directorio quiere "
     "instalar los ficheros del Workbench 3.1. Normalmente será el mismo "
     "lugar donde tiene actualmente su Workbench 1.3 o Workbench 2.0.\n\n"
     @askdir-help
))

(set #preparing-icons
(cat "\n\nPreparando iconos..."
))

(set #which-language
(cat "\n¿Qué idioma quiere instalar?"
))

(set #which-language-help
(cat "\nAmiga puede operar en diferentes idiomas. "
     "Se copiarán algunos ficheros a su disco duro "
     "por cada idioma utilizado.\n\n"
     "Para reducir el espacio usado en disco duro "
     "puede seleccionar sólo los ficheros del idioma "
     "que va a utilizar.\n\n"
     "Sólamente tiene que pulsar en el idioma que "
     "quiera instalar en su sistema.\n\n"
     @askoptions-help
))

(set #reboot
(cat "La instalación de Workbench 3.1 ha terminado.\n\n"
     "Para activarla, debe reinicializar su Amiga.\n\n"
     "Retire el disco \"Amiga Install\" de la unidad, "
     "y eliga \"Seguir\" para arrancar el Amiga."
))

))


;=============================================================================
; French strings

(if (= @language "français")
(
(set default_lang 16)

(set #bad-kick
(cat "Il vous faut utiliser le Kickstart 3.0 pour installer le Workbench 3.1"
))

(set #introduction
(cat "\n\nCe programme vous permet d'installer la version 3.1 du système d'exploitation "
     "AmigaDOS sur un disque dur. Il peut être utilisé pour mettre à jour une "
     "version antérieure, installer pour la première fois, ou pour contrôler quels "
     "langues sont installés sur un disque dur déja configuré avec le Workbench 3.1. "
))

(set #ask-function
(cat "\n\nChoississez \"Installer Version 3.1\" pour faire l'installation complète, ou "
     "\"Mettre à jour langues\" pour modifier les langues installées sur un disque "
     "dur déja configuré avec le Workbench 3.1."
))

(set #ask-function-1
(cat "Installer Version 3.1"
))

(set #ask-function-2
(cat "Mettre à jour langues"
))

(set #ask-function-help
(cat "\nVous devez choisir quelle opération effectuer.\n\n"
     "Choisir \"Installer Version 3.1\" vous permet d'effectuer l'installation "
     "complète de la version 3.1. Cette option doit être choisie pour mettre à jour "
     "depuis une version antérieure, ou pour installer le Workbench 3.1 sur un "
     " disque dur vierge.\n\n"
     "Les ordinateurs Amiga peuvent être utilisés avec plusieures langues. "
     "Quelques fichiers doivent être copiés sur votre disque dur pour "
     "chacune des langues. Choisir \"Mettre à jour Langues\" vous permet "
     "de selectionner quelles langues sont installés et utilisables sur "
     "le disque dur. Cette option ne doit être choisie que lorsque le "
     "workbench 3.1 a déja été installé.\n\n"
))

(set #confirm-target
(cat "\n\nCe programme va mettre votre disque dur à jour en installant le "
     "Workbench 3.1. Les fichiers du Workbench actuels seront remplaçés. "
     "Si nécessaire, certains fichiers seront conservés, par exemple le "
     "fichier Startup-Sequence.\n\n"
     "Désirez-vous que le Workbench 3.1 soit installé dans la partition \"%s\" ?"
))

(set #confirm-target-help
(cat "\nL'Installer a déterminé que le Workbench 3.1 devrait probablement être "
     "installé dans la partition \"%s\". Continuer l'installation "
     "remplacera tous les fichiers du Workbench actuel par de nouveaux "
     "fichiers.\n\n"
     "Certains fichiers seront d'abord copiés dans un endroit sûr, "
     "avant qu'ils ne soient remplacés, comme par exemple le fichier "
     "Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nLes langues disponibles avec le Workbench 3.1 vont "
     "être mis à jour. Votre Workbench 3.1 semble être installé "
     "dans la partition \"%s\".\n\n"
     "Est-ce correct ?"
))

(set #confirm-target-lang-help
(cat "L'Installer a déterminé que votre Workbench 3.1 est installé "
     "dans la partition \"%s\". Continuer l'installation va provoquer "
     "la copie des fichiers langages du Workbench à cet endroit."
))

(set #which-disk-lang
(cat "Dans quelles partitions les fichiers langages du Workbench 3.1 "
     "doivent-ils être installés ?"
))

(set #which-disk-lang-help
(cat "Cette partie vous permet de choisir sur quelle partition les fichiers "
     "langages du Workbench 3.1 doivent être installés. Ils devraient aller "
     "au même endroit que les fichiers du Workbench 3.1.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nLa mise à jour des langues du Workbench 3.1 est terminée.\n\n"
     "Pour utiliser ces langues, vous devez redémarrer votre ordinateur. "
     "Après le démarrage, vous devez utiliser le programme des "
     "préférences \"Locale\" pour choisir la langue que vous souhaitez "
     "utiliser.\n\n"
     "Retirez la disquette \"Amiga Install\" du lecteur,"
     "et choississez la cellule \"Continuer\" pour redémarrer"
     "votre ordinateur Amiga."
))

(set #which-disk
(cat "Dans quelle partition faut-il installer le Workbench 3.1 ?"
))

(set #which-disk-help
(cat "\nCette partie vous permet de choisir la partition dans laquelle les "
     "fichiers du Workbench 3.1 seront installés. Normalement, ils devraient "
     "être à la place des fichiers du Workbench 1.3, Workbench 2 ou "
     "Workbench 2.1.\n\n"
     @askdir-help
))

(set #preparing-icons
(cat "\n\nMise en place des icônes..."
))

(set #which-language
(cat "\nQuelles sont les langues à installer ?"
))

(set #which-language-help
(cat "\nL'Amiga peut fonctionner dans différentes langues. "
     "Plusieurs fichiers doivent être copiés sur le disque dur "
     "pour chaque langue utiliséee\n\n"
     "Pour réduire la quantité d'espace utilisé par ces fichiers, "
     "vous devez sélectionner seulement les langues qui vous seront "
     "utiles.\n\n"
     "Cochez les langues que vous souhaitez "
     "sur votre système\n\n"
     @askoptions-help
))

(set #reboot
(cat "\nL'installation du Workbench 3.1 est terminée.\n\n"
     "Pour en profiter, vous devez redémarrer votre ordinateur Amiga.\n\n"
     "Retirez la disquette \"Amiga Install\" du lecteur de disquette, "
     "et choississez la cellule \"Continuer\" pour redémmarrer votre "
     "ordinateur Amiga."
))

))


;=============================================================================
; German strings

(if (= @language "deutsch")
(
(set default_lang 2)

(set #bad-kick
(cat "Sie müssen Kickstart 3.0 zum Installieren von Workbench 3.1 benutzen."
))

(set #introduction
(cat "\n\nHiermit können Sie Version 3.1 des Amiga-Betriebssystems auf einer "
     "Festplatte installieren. Sie können damit eine ältere Version "
     "aktualisieren, von Grund auf neu installieren oder nachträglich "
     "wählen, welche Sprachen unter Version 3.1 auf der Festplatte zur "
     "Verfügung stehen sollen."
))

(set #ask-function
(cat "\n\nWählen Sie \"Version 3.1 installieren\", um eine komplette "
     "Installation durchzuführen, oder \"Sprachen aktualisieren\", um "
     "andere Sprachen auf einer Festplatte zur Verfügung zu stellen, "
     "auf der bereits Version 3.1 installiert ist."
))

(set #ask-function-1
(cat "Version 3.1 installieren"
))

(set #ask-function-2
(cat "Sprachen aktualisieren"
))

(set #ask-function-help
(cat "\nSie müssen auswählen, was installiert werden soll.\n\n"
     "Mit \"Version 3.1 installieren\" wird eine komplette Installation "
     "der Version 3.1 der Workbench-Software durchgeführt. Dies sollten "
     "Sie wählen, um ein System von einer älteren Version ausgehend zu "
     "aktualisieren oder die Version 3.1 frisch auf einer neuen Festplatte "
     "zu installieren.\n\n"
     "Der Amiga kann in vielen verschiedenen Sprachen betrieben werden. "
     "Für jede verwendete Sprache müssen mehrere Dateien auf Ihre "
     "Festplatte kopiert werden. Mit \"Sprachen aktualisieren\" können "
     "Sie auswählen, welche Sprachen auf der Festplatte installiert "
     "werden sollen, um dort zur Verfügung zu stehen. Dies dürfen Sie "
     "nur wählen, wenn die Version 3.1 schon installiert wurde.\n\n"
))

(set #confirm-target
(cat "\n\nDieses Programm installiert Version 3.1 auf Ihrer Festplatte. "
     "Bisherige Workbench-Versionen werden überschrieben. Wenn angebracht, "
     "werden bestimmte Dateien der früheren Version bewahrt, darunter "
     "die Startup-Sequence.\n\n"
     "Wollen Sie die Version 3.1 auf der Partition \"%s\" installiert "
     "bekommen?"
))

(set #confirm-target-help
(cat "\nDas Installationsprogramm hat herausgefunden, daß die Version 3.1 "
     "wahrscheinlich auf der Partition namens \"%s\" installiert "
     "werden soll. Bei der weiteren Installation "
     "werden die meisten Dateien in dieser Partition durch neuere Dateien "
     "ersetzt.\n\nEinige Dateien werden vorher an einen sicheren Platz "
     "kopiert, bevor ihre Nachfolger installiert werden, darunter die "
     "Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nDie unter Version 3.1 verfügbaren Sprachen werden jetzt auf Ihrer "
     "Festplatte aktualisiert. Ihre Workbench-Dateien der Version 3.1 sind "
     "offenbar auf der Partition \"%s\" installiert.\n\n"
     "Ist das korrekt?"
))

(set #confirm-target-lang-help
(cat "\nDer Installer hat herausgefunden, daß die aktuellen "
     "Workbench-Dateien offenbar in der Partition namens \"%s\" liegen. "
     "Wenn Sie die Installation einfach weiterlaufen lassen, werden die "
     "Sprachendateien dorthin kopiert."
))

(set #which-disk-lang
(cat "Auf welcher Festplattenpartition sollen die Sprachendateien für "
     "Version 3.1 installiert werden?"
))

(set #which-disk-lang-help
(cat "\nHier können Sie wählen, auf welcher Festplattenpartition die "
     "Sprachendateien für Version 3.1 installiert werden sollen. Das "
     "sollte dieselbe Partition wie die mit den Workbench-Dateien "
     "der Version 3.1 sein.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nDie Aktualisierung der Version 3.1 ist nun abgeschlossen.\n\n"
     "Um diese neuen Sprachen zu aktivieren, müssen Sie Ihren Amiga "
     "neu starten und danach den Voreinsteller-Editor \"Locale\" "
     "aus der Schublade \"Prefs\" aufrufen und dort die gewünschte "
     "Sprache auswählen.\n\n"
     "Entfernen Sie zunächst die Diskette \"Amiga Install\" aus dem "
     "Diskettenlaufwerk und wählen Sie dann das Symbol \"Weiter\", "
     "um Ihren Amiga neu zu starten."
))

(set #which-disk
(cat "Auf welcher Partition soll Version 3 installiert werden?"
))

(set #which-disk-help
(cat "\nHier können Sie wählen, auf welcher Festplattenpartition "
     "die Dateien von Version 3.1 installiert werden sollen. "
     "Sie sollten normalerweise an dieselbe Stelle installiert werden "
     "wie die bisherigen Dateien von Version 1.3, 2 oder 2.1.\n\n"
     @askdir-help
))

(set #preparing-icons
(cat "\n\nVorbereiten der Piktogramme ..."
))

(set #which-language
(cat "\nWelche Sprachen sollen installiert werden?"
))

(set #which-language-help
(cat "\nDer Amiga kann in vielen verschiedenen Sprachen betrieben werden. "
     "Für jede verwendete Sprache müssen mehrere Dateien auf Ihre "
     "Festplatte kopiert werden.\n\n"
     "Um den Platz in Grenzen zu halten, den die Sprachentreiberdateien "
     "einnehmen, können Sie wählen, nur die Treiber für bestimmte "
     "Sprachen zu installieren.\n\n"
     "Klicken Sie einfach die Felder derjenigen Sprachen an, die Sie in "
     "Ihrem System verfügbar haben wollen, so daß diese Sprachen mit "
     "einem Häkchen markiert sind.\n\n"
     @askoptions-help
))

(set #reboot
(cat "Die Installation der Version 3.1 ist nun abgeschlossen.\n\n"
     "Um die Version 3.1 zu aktivieren, müssen Sie Ihren Amiga neu "
     "starten. "
     "Nehmen Sie die Diskette \"Amiga Install\" aus dem Diskettenlaufwerk "
     "und klicken Sie dann auf \"Weiter\", um Ihren Amiga neu zu starten.\n\n"
     "Danach müssen Sie ggf. den Voreinsteller-Editor \"Locale\" "
     "aus der Schublade \"Prefs\" aufrufen und dort Ihr Land und die "
     "gewünschte Sprache auswählen. Prüfen Sie im Voreinsteller-Editor "
     "\"Input\", ob dort die gewünschte Tastaturbelegung "
     "ausgewählt ist."
))

))


;=============================================================================
; Dutch strings

(if (= @language "nederlands")
(
(set default_lang 64)

(set #bad-kick
(cat "U moet Kickstart 3.0 gebruiken om Workbench 3.1 te installeren"
))

(set #introduction
(cat "\n\nDit programma installeert Release 3.1 van het Amiga besturings "
     "systeem op uw harddisk. Het kan gebruikt worden om van een "
     "oudere versie naar een nieuwere versie van het besturings systeem "
     "over te gaan, om een volledig nieuwe installatie te doen, of om een "
     "andere taal te installeren op een harddisk, waarop Release 3.1 reeds "
     "geïnstalleerd is."
))

(set #ask-function
(cat "\n\nSelecteer \"Installeer Release 3.1\" om een complete installatie "
     "van Release 3.1 te doen, of selecteer \"Andere talen\" om de talen welke "
     "beschikbaar zijn op een harddisk, waarop Release 3.1 reeds is "
     "geïnstalleerd, te wijzigen."
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
     "welke talen op de harddisk geïnstalleerd en beschikbaar zijn. De "
     "functie kan alleen gekozen worden wanneer Release 3.1 reeds is "
     "geïnstalleerd.\n\n"
))

(set #confirm-target
(cat "\n\nRelease 3.1 wordt nu geïnstalleerd op uw harddisk. Elke "
     "voorgaande Release, met uitzondering van specifieke bestanden "
     "zoals de startup-Sequence, zal vervangen worden.\n\n"
     "Wilt u Release 3.1 geïnstalleerd hebben op de \"%s\" partitie?"
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
     "Workbench taal bestanden daarheen kopiëren."
))

(set #which-disk-lang
(cat "Op welke partitie moeten de Release 3.1 taal bestanden geïnstalleerd "
     "worden?"
))

(set #which-disk-lang-help
(cat "\nDit gedeelte stelt u in staan te kiezen op welke partitie de "
     "Release 3.1 taal bestanden geïnstalleerd moeten worden. Deze behoren "
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
(cat "Op welke partitie moet Workbench 3.1 geïnstalleerd worden?"
))

(set #which-disk-help
(cat "\nDit gedeelte stelt u in staat te kiezen op welke partitie de "
     "Release 3.1 bestanden geïnstalleerd moeten worden. Gewoonlijk "
     "dienen deze op dezelfde plaats te komen als de huidige Workbench "
     "1.3 of Workbench 2.0 bestanden.\n\n"
     @askdir-help
))

(set #preparing-icons
(cat "\n\nPrepareren iconen..."
))

(set #which-language
(cat "\nWelke taal moet geïnstalleerd worden?"
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
(cat "E' necessario il Kickstart 3.0 per installare il Workbench 3.1!"
))

(set #introduction
(cat "\n\nQuesto programma installa la Versione 3.1 del Sistema Operativo "
     "di Amiga su disco fisso. Può essere usato per aggiornare una versione "
     "precedente, per una nuova installazione o per controllare quali "
     "lingue sono disponibili sul disco fisso con la Versione 3.1 già "
     "installata."
))

(set #ask-function
(cat "\n\nSelezionare \"Installa Versione 3.1\" per fare una installazione "
     "completa della Versione 3.1, o selezionare \"Aggiorna lingue\" per "
     "modificare le lingue disponibili sul disco fisso con la Versione 3.1 "
     "già installata."
))

(set #ask-function-1
(cat "Installa Versione 3.1"
))

(set #ask-function-2
(cat "Aggiorna lingue"
))

(set #ask-function-help
(cat "\nDovete scegliere l'operazione che volete effettuare.\n\n"
     "Selezionando il pulsante \"Installa Versione 3.1\" verrà effettuata "
     "una installazione completa del software di sistema Versione 3.1. Questa "
     "opzione dovrebbe essere scelta quando si vuole aggiornare un "
     "sistema avente versioni precedenti, o quando si vuole installare "
     "la Versione 3.1 su un disco fisso nuovo.\n\n"
     "Amiga può operare con lingue differenti. Numerosi file devono essere "
     "copiati sul disco fisso per ogni lingua supportata. "
     "Selezionando il pulsante \"Aggiorna lingue\" potrete controllare "
     "quali lingue sono installate e quali sono disponibili per l'utilizzo "
     "col disco fisso. Questa opzione deve essere scelta solo se "
     "la Versione 3.1 è già stata installata.\n\n"
))

(set #confirm-target
(cat "\nLa Versione 3.1 verrà copiata sul disco fisso, qualsiasi versione "
     "precedente verrà sostituita. Se utilizzabili, alcuni file della "
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
     "Questo è corretto?"
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
(cat "\nL'aggiornamento delle lingue della Versione 3.1 è stato completato.\n\n"
     "Per usare queste nuove lingue è necessario riavviare Amiga. Dopo il "
     "riavvio dovrete utilizzare il programma di preferenze \"Locale\" per "
     "scegliere la lingua che volete utilizzare.\n\n"
     "Togliere il disco \"Amiga Install\" dalla unità a dischetti, e "
     "selezionare il pulsante \"Procedi\" per riavviare Amiga."
))

(set #which-disk
(cat "In quale partizione deve essere installata la Versione 3?"
))

(set #which-disk-help
(cat "\nQuesta sezione vi permette di scegliere in quale partizione devono "
     "essere installati i file del Workbench 3.1. Questa dovrebbe essere "
     "la stessa locazione in cui erano presenti i file del Workbench 1.3 "
     "o del Workbench 2.0.\n\n"
     @askdir-help
))

(set #preparing-icons
(cat "\n\nPreparazione icone..."
))

(set #which-language
(cat "\nQuali lingue devono essere installate? "
))

(set #which-language-help
(cat "\nAmiga può operate con lingue differenti. "
     "Per ogni lingua supportata saranno copiati "
     "numerosi file sul disco fisso.\n\n"
     "Per ridurre lo spazio occupato da questi "
     "file, potete installare solo quelli di una "
     "lingua specifica.\n\n"
     "Segnate solo i riquadri delle lingue che volete "
     "siano disponibili sul vostro sistema.\n\n"
     @askoptions-help
))

(set #reboot
(cat "\nL'installazione della Versione 3.1 è completa.\n\n"
     "Per abilitare la Versione 3.1, dovrete riavviare Amiga.\n\n"
     "Togliere il disco \"Amiga Install\" dalla unità a dischetti "
     "e selezionare il pulsante \"Procedi\" per riavviare Amiga."
))

))


;=============================================================================
; Danish strings

(if (= @language "dansk")
(
(set default_lang 1)

(set #bad-kick
(cat "De skal benytte Kickstart 3.0 ved installation af Workbench 3.1"
))

(set #introduction
(cat "\n\nMed dette program kan De installere Workbench 3.1 "
     "på en harddisk. Programmet kan benyttes til at opdatere en "
     "tidligere Workbench version, til grundinstallation, eller til at "
     "kontrollere hvilke sprog der er tilgængelige på en harddisk, hvor "
     "Workbench 3.1 allerede er installeret."
))

(set #ask-function
(cat "\n\nVælg \"Installere Workbench 3.1\" for at lave en komplet "
     "installation af Workbench 3.1, eller vælg \"Opdatering af sprog\" for at "
     "ændre hvilke sprog der er tilgængelige på en harddisk, hvor "
     "Workbench 3.1 allerede er installeret."
))

(set #ask-function-1
(cat "Installere Workbench 3.1"
))

(set #ask-function-2
(cat "Opdatering af sprog"
))

(set #ask-function-help
(cat "\nDe skal vælge hvilken operation De ønsker at udføre.\n\n"
     "Klik på \"Installere Workbench 3.1\" knappen for at udføre en komplet "
     "installation af Workbench 3.1 softwaren. Vælg denne mulighed "
     "når systemet skal opgraderes fra en tidligere version, eller når der "
     "installeres Workbench 3.1 på en ny harddisk.\n\n"
     "Deres Amiga computer kan kommunikere på mange forskellige sprog. "
     "Der skal kopieres adskillige filer til Deres harddisk "
     "for hvert sprog De ønsker at benytte.\n\n"
     "Klik på \"Opdatering af sprog\" knappen for at se hvilke sprog der er "
     "installeret og klar til brug på harddisken. "
     "Vælg kun denne mulighed når Workbench 3.1 allerede er installeret på "
     "harddisken.\n\n"
))

(set #confirm-target
(cat "Dette program vil opdatere Deres harddisk til Workbench 3.1.\n\n"
     "Deres nuværende Workbench filer vil blive overskrevet, "
     "dog vil Startup-Sequence og Shell-Startup filerne "
     "blive bibeholdt.\n\n"
     "Deres Workbench filer synes at være placeret i     \n\"%s\" "
     "skuffen.\n\n"
     "Ønsker De også Workbench 3.1 opdateringen placeret i denne skuffe?"
))

(set #confirm-target-help
(cat "\nInstallations programmet har fundet ud af at Deres nuværende "
     "Workbench findes i skuffen \"%s\".\n\nHvis De fortsætter med "
     "installationen vil de fleste af filerne i denne skuffe blive "
     "erstattet med nyere filer.\nNogle filer bliver først kopieret til "
     "et sikkert sted før deres erstatning installeres. Dette omfatter "
     "den nuværende Startup-Sequence "
     "og Shell-Startup filerne."
))

(set #confirm-target-lang
(cat "\n\nDe tilgængelige Workbench 3.1 sprogfiler på Deres harddisk vil nu "
     "blive opdateret. Deres Workbench 3.1 filer synes at være placeret "
     "i \"%s\" skuffen.\n\n"
     "Er dette korekt?"
))

(set #confirm-target-lang-help
(cat "\nInstallations programmet har fundet Deres nuværende Workbench i "
     "skuffen \"%s\". Hvis De fortsætter installationen vil Workbench sprog"
     "filerne blive kopieret dertil."
))

(set #which-disk-lang
(cat "I hvilken skuffe skal Workbench 3.1 sprogfilerne installeres?"
))

(set #which-disk-lang-help
(cat "\nDette afsnit lader Dem vælge i hvilken harddisk skuffe Workbench 3.1 "
     "sprogfilerne, skal installeres. Disse skal ligge i det samme område, "
     "som Workbench 3.1 filerne.\n\n"
))

(set #reboot-lang
(cat "\nOpdateringen af Workbench 3.1 sprog er nu færdig.\n\n"
     "For at benytte disse nye sprog, skal De slukke og tænde Deres Amiga igen. "
     "Efter genopstart, skal De kører \"Locale\" præference editoren og vælge "
     "det sprog De ønsker at bruge.\n\n"
     "Fjern \"Amiga Install\" disketten fra floppy drevet, "
     "og klik på \"Fortsætte\" knappen for at resette Deres Amiga."
))

(set #which-disk
(cat "På hvilken disk/i hvilken skuffe\nønsker De at installere Workbench 3.1 ?"
))

(set #which-disk-help
(cat "\nHer skal De vælge i hvilken skuffe eller på hvilken disk "
     "Workbench 3.1 filerne skal installeres.\n\nNormalt bør de installeres "
     "samme sted som de nuværende Workbench 1.3 eller Workbench 2.0 filer.\n\n"
     @askdir-help
))

(set #preparing-icons
(cat "\n\nKlargør ikoner..."
))

(set #which-language
(cat "\nHvilke(t) sprog ønskes installeret?"
))

(set #which-language-help
(cat "\nAmiga kan kommunikere på mange forskellige sprog. "
     "Der skal kopieres adskillige filer til Deres harddisk "
     "for hvert sprog De ønsker at benytte.\n\n"
     "For at reducere pladsen, der optages af sprog-filer, "
     "kan De vælge kun at kopiere sprog, som De ønsker at benytte.\n\n"
     "Klik på knapperne ud for hvert sprog De ønsker at have til rådighed "
     "på Deres system. Et 'flueben' indikerer et valgt sprog.\n\n"
     @askoptions-help
))

(set #reboot
(cat "\nInstallationen af Workbench 3.1 er nu færdig.\n\n"
     "For at aktiverer Workbench 3.1, skal De slukke/tænde Amiga "
     "computeren.\n\n"
     "Fjern \"Amiga Install\" disketten fra diskette drevet, og klik "
     "på \"Fortsætte\" knappen for at reboote Deres Amiga computer."
))

))


;=============================================================================
; Swedish strings

(if (= @language "svenska")
(
(set default_lang 512)

(set #bad-kick
(cat "Du måste använda Kickstart 3.0 för att kunna installera Workbench 3.1"
))

(set #introduction
(cat "\n\nDetta program låter dig installera Release 3.1 av Amiga Operativsystemet "
     "på en hårddisk. Det kan användas för att uppgradera en äldre hårddisk, "
     "installera från början, eller för att styra vilka språk som skall "
     "göras tillgängliga på en hårddisk som redan har Release 3.1 installerad."
))

(set #ask-function
(cat "\n\n\Välj \"Installera Release 3.1\" för att göra en komplett installation "
     "av Release 3.1, eller välj \"Uppdatera språk\" för att ändra vilka "
     "språk som skall vara tillgängliga på en hårddisk med ett redan "
     "installerat system."
))

(set #ask-function-1
(cat "Installera Release 3.1"
))

(set #ask-function-2
(cat "Uppdatera språk"
))

(set #ask-function-help
(cat "\nDu måste välja vilken funktion du vill utföra.\n\n"
     "Om du väljer \"Installera Release 3.1\" så kan du utföra en komplett "
     "installation av Release 3.1 Workbench. Du skall välja detta alternativ "
     "när du uppgraderar från en tidigare version av operativsystemet, "
     "eller när du skall installera Release 3.1 på en ny hårddisk.\n\n"
     "Din Amiga kan användas på flera olika språk. Flera filer måste kopieras "
     "till din hårddisk för varje språk du vill använda. Välj \"Uppdatera "
     "språk\" för att styra vilka språk du vill ha tillgängliga på din "
     "hårddisk. Detta alternativ skall bara väljas då du redan har "
     "installerat Release 3.\n\n"
))

(set #confirm-target
(cat "\n\nDetta program uppdaterar din hårddisk till Release 3.1. Om du "
     "har någon tidigare version av operativsystemet på din hårddisk "
     "kommer denna att skrivas över. Vissa viktiga filer (inklusive "
     "startup-Sequence) kommer att sparas.\n\n"
     "Vill du installera Release 3.1 i lådan \"%s\"?"
))

(set #confirm-target-help
(cat "\nInstalleraren har funnit att Release 3.1 bör installeras i lådan "
     "\"%s\". Om du fortsätter med installationen kommer de flesta av "
     "operativsystemets filer i denna låda att skrivas över.\n\nNågra "
     "filer kommer att kopieras till ett säkert ställe först, bl.a "
     "Startup-Sequence."
))

(set #confirm-target-lang
(cat "De tillgängliga Release 3-språken på din hårddisk kommer nu att "
     "uppdateras. Dina Release 3.1 Workbench-filer förefaller att finnas "
     "på partitionen \"%s\".\n\nÄr detta korrekt?"
))

(set #confirm-target-lang-help
(cat "\nInstallationsprogrammet har fastslagit att din nuvarande Workbench "
     "finns på partitionen \"%s\". Om du fortsätter med installationen "
     "kommer språkfilerna att kopieras dit."
))

(set #which-disk-lang
(cat "På vilken partition skall Release 3.1 språkfilerna placeras?"
))

(set #which-disk-lang-help
(cat "\nDetta avsnitt låter dig välja på vilken hårddiskspartition du vill "
     "placera språkfilerna för Release 3.1. Dessa skall ligga på samma "
     "partition som Release 3.1 Workbench-filerna.\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nUppdateringen av språken för Release 3.1 är nu komplett.\n\n"
     "För att använda de nya språken måste du starta om din Amiga. Efter "
     "omstart måste du använda Prefs-editorn \"Locale\" för att välja det "
     "eller de språk du vill använda.\n\n"
     "Ta ur disketten \"Amiga Install\" ur diskettstationen och välj "
     "\"Fortsätt\" för att starta om Amigan."
))

(set #which-disk
(cat "På vilken enhet eller i vilken låda skall Release 3.1 installeras?"
))

(set #which-disk-help
(cat "\nHär kan du välja vilken enhet (eller låda) filerna för Release 3.1 "
     "skall installeras på. Normalt (i nästan alla fall) skall detta vara "
     "samma ställe som där de tidigare filerna för Workbench 1.3, 2.0 "
     "eller 2.1 finns.\n\n"
     @askdir-help
))

(set #preparing-icons
(cat "\n\nStäller in symboler..."
))

(set #which-language
(cat "\nVilka språk skall installeras?"
))

(set #which-language-help
(cat "\nAmigan kan arbeta med flera olika språk. Ett flertal filer "
     "måste kopieras till hårddisken för varje språk du vill kunna "
     "använda.\n\n"
     "För att minimera hårddiskutrymmet för språkfiler kan du "
     "välja att bara installera filerna för vissa språk.\n\n"
     "Markera bara rutorna för de språk du önskar installera "
     "i din dator.\n\n"
     @askoptions-help
))

(set #reboot
(cat "\nInstallationen av Release 3.1 är nu komplett.\n\n"
     "För att använda Release 3.1 måste du starta om din Amiga.\n\n"
     "Ta ur disketten \"Amiga Install\" ur diskettenheten och välj "
     "\"Fortsätt\" för att starta om din Amiga."
))

))


;=============================================================================
; Norwegian strings

(if (= @language "norsk")
(
(set default_lang 128)

(set #bad-kick
(cat "Du må ha Kickstart 3.0 for å installere Workbench 3.1"
))

(set #introduction
(cat "\n\nDette programmet lar deg installere versjon 3.1 av Amigas "
     "operativsystem på harddisk. Det kan benyttes til å oppgradere en eldre "
     "utgave, til å installere fra grunnen av, eller til å endre hvilke språk "
     "som skal kunne benyttes på en harddisk som allerede inneholder versjon 3.1"
))

(set #ask-function
(cat "\n\nVelg \"Installer versjon 3.1\" for å gjennomføre en komplett "
     "installasjon av versjon 3.1, eller velg \"Oppdater språk\" for å endre "
     "hvilke språk som skal kunne benyttes på en maskin som allerede "
     "inneholder versjon 3.1"
))

(set #ask-function-1
(cat "Installer versjon 3.1"
))

(set #ask-function-2
(cat "Oppdater språk"
))

(set #ask-function-help
(cat "\nDu må velge hvilken operasjon du vil utføre.\n\n"
     "Ved å velge knappen merket \"Installer versjon 3.1\" kan du få utført en "
     "komplett installasjon av utgave 3.1 av Workbench. Dette alternativet skal "
     "velges når du vil oppgradere fra en tidligere utgave, eller når du skal "
     "installere versjon 3.1 på en ny harddisk.\n\n"
     "Amigaen kan operere på mange forskjellige språk. "
     "For hvert språk som skal kunne benyttes, må det "
     "kopieres inn mange filer på harddisken. Ved å velge "
     "\"Oppdater språk\" kan du bestemme hvilke språk som "
     "skal kunne benyttes på din maskin. Denne operasjonen kan bare velges "
     "hvis versjon 3.1 allerede er installert.\n\n"
))

(set #confirm-target
(cat "\nWb 3.1 vil nå bli installert på harddisken din. Filene fra en "
     "eldre utgave vil bli erstattet. Eventuelle viktige filer i den "
     "gamle utgaven vil bli beholdt, herunder din Startup-Sequence.\n\n"
     "Ønsker du å installere Workbench 3.1 på \"%s\"-partisjonen?"
))

(set #confirm-target-help
(cat "\nInstalleringsprogrammet har funnet ut at utgave 3.1 antakelig skal "
     "installeres på \"%s\"-partisjonen på harddisken. Hvis du fortsetter "
     "denne prosedyren, vil eventuelle Workbench-filer på denne partisjonen "
     "bli erstattet med nye.\n\n"
     "Noen filer vil først bli kopiert til et sikkert sted før de nye versjonene "
     "blir installert, herunder Startup-Sequence."
))

(set #confirm-target-lang
(cat "\n\nDe tilgjengelige språkene for Workbench vil nå bli oppdatert. "
     "Filene for versjon 3.1 av Workbench ser ut til å ligge på "
     "\"%s\"-partisjonen på harddisken.\n\n"
     "Er dette korrekt?"
))

(set #confirm-target-lang-help
(cat "\nInstallasjonsprogrammet har funnet ut at din kopi av Workbench "
     "ligger på \"%s\"-partisjonen på harddisken. Hvis du fortsetter med "
     "installasjonen, vil språkfilene for Workbench bli kopiert dit. "
))

(set #which-disk-lang
(cat "På hvilken partisjon skal språkfilene til utgave 3.1 bli installert?"
))

(set #which-disk-lang-help
(cat "\nHer kan du velge hvilken harddisk-partisjon språkfilene for "
     "versjon 3.1 skal installeres på. De bør ligge på samme partisjon som "
     "Workbench-filene for utgave 3.1\n\n"
     @askdir-help
))

(set #reboot-lang
(cat "\nOppdateringen av språkfilene for Workbench 3.1 er nå ferdig.\n\n"
     "For å benytte disse språkene, må du starte din Amiga på nytt. Etter "
     "oppstarten, må du kjøre \"Locale\"-programmet for å velge "
     "språk.\n\n"
     "Ta ut \"Amiga Install\"-disketten fra diskstasjonen, og velg "
     "\"Fortsett\"-knappen for å starte din Amiga om igjen."
))

(set #which-disk
(cat "På hvilken partisjon ønsker du å installere Workbench 3.1 ?"
))

(set #which-disk-help
(cat "Her skal du velge hvilken partisjon filene til versjon 3.1 skal "
     "installeres på. Normalt skal de plasseres på samme sted som de "
     "nåværende filene for Wb1.3, Wb2.0 eller Wb2.1 .\n\n"
     @askdir-help
))

(set #preparing-icons
(cat "\n\nKlargjør ikonene..."
))

(set #which-language
(cat "\nHvilke språk skal installeres?"
))

(set #which-language-help
(cat "Amiga kan benytte mange forskjellige språk. "
     "Mange filer må kopieres til harddisken for hvert "
     "språk som du installerer.\n\n"
     "For å redusere plassen som benyttes til språkfiler, "
     "kan du velge ut de språkene du vil ha tilgjengelige.\n\n"
     "Sett en hake ved de språkene du ønsker å ha "
     "tilgjengelige på din maskin.\n\n"
     @askoptions-help
))

(set #reboot
(cat "\nInstallasjonen av versjon 3.1 er nå ferdig.\n\n"
     "For å benytte versjon 3.1, må du starte din Amiga om igjen.\n\n"
     "Ta ut \"Amiga Install\"-disketten fra diskstasjonen, "
     "og velg \"Fortsett\"-knappen for å starte din Amiga på nytt."
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
				   "español"
				   "français"
				   "italiano"
				   "nederlands"
				   "norsk"
				   "português"
				   "svenska"
				   ""
			 )
	   )
    (
	(if (IN lang n)
	(

	    (makedir (tackon target (cat "Locale/Catalogs/" language)))
	    (makedir (tackon target (cat "Locale/Catalogs/" language "sys")))
	    (copyfiles (source (cat (tackon localePath "Catalogs") "/" language "/sys/amigacd.catalog"))
		       (dest (tackon target (cat "Locale/Catalogs/" language "/sys")))
	    )

	)
	(
	    (delete (tackon target (cat "Locale/Catalogs/" language "sys/amigacd.catalog")))
	))
	(set n (+ n 1))
    ))
))


;=============================================================================
; procedure to call on exit...

(procedure TERMINATE (makeassign "Install3.1")
)


;=============================================================================
;=====  The beginning....
;=============================================================================

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


(set installPath   "CD1200:")


(delopts "oknodelete" "force" "askuser")
(run (cat "Resident " (tackon installPath "C/IconPos PURE")) (safe))
(run (cat "Resident " (tackon installPath "C/Delete PURE")) (safe))
(run (cat "Resident " (tackon installPath "C/Reboot PURE")) (safe))

(run (cat (tackon installPath "C") "/GuessBootDev >ENV:BootDev CD1200:") (safe))
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
( OPEN

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


;=============================================================================
; which languages should be installed?

(user 2)
(set lang (askoptions (prompt #which-language)
		      (help #which-language-help)
		      (choices "Dansk" "Deutsch" "English" "Español" "Français" "Italiano" "Nederlands" "Norsk" "Português" "Svenska")
		      (default default_lang)
	  )
)
(user old_level)


;=============================================================================
; make some new drawers

(makedir (tackon target "Classes"))
(makedir (tackon target "Classes/Gadgets"))
(makedir (tackon target "Classes/Images"))
(makedir (tackon target "Libs"))
(makedir (tackon target "Locale"))
(makedir (tackon target "Locale/Catalogs"))
(makedir (tackon target "Prefs"))
(makedir (tackon target "Prefs/Env-Archive"))
(makedir (tackon target "Prefs/Env-Archive/Sys"))
(makedir (tackon target "Utilities"))

(complete 2)


;=============================================================================
; unstick any icons on the target

(working #preparing-icons)

(set listInfos (cat "list \"" target "\" PAT=#?.info ALL LFORMAT \"IconPos >NIL: *\"%p%n*\" -1 -1\" >T:InstallTemp"))
(run listInfos)
(execute "T:InstallTemp")
(delete "T:InstallTemp")

(complete 10)


;=============================================================================
; copy the files!

(copyfiles (source (tackon installPath "Classes/Gadgets"))
	   (dest (tackon target "Classes/Gadgets"))
	   (all)
)

(copyfiles (source (tackon installPath "Classes/Images"))
	   (dest (tackon target "Classes/Gadgets"))
	   (all)
)

(copyfiles (source (tackon installPath "Libs/nonvolatile.library"))
	   (dest (tackon target "Libs"))
)

(copyfiles (source (tackon installPath "Prefs"))
	   (dest (tackon target "Prefs"))
	   (pattern "CDPrefs#?")
)

(copyfiles (source (tackon installPath "Utilities"))
	   (dest (tackon target "Utilities"))
	   (pattern "CDAudio#?")
)

(DOLOCALE)

(complete 88)


;=============================================================================
; Clean up that mess young man!
;
(working #positioning-icons)

(run (cat "IconPos >NIL: \"" target "Prefs/CDPrefs\"  160 48\n"))
(run (cat "IconPos >NIL: \"" target "Utilities/CDAudio\" 91 11\n"))


(complete 100)


;=============================================================================
; "Say goodnight Gracy."

(user 2)
;(message #reboot)
;(run "Reboot")

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

(set lang (askoptions (prompt #which-language)
		      (help #which-language-help)
		      (choices "Dansk" "Deutsch" "English" "Español" "Français" "Italiano" "Nederlands" "Norsk" "Português" "Svenska")
		      (default default_lang)
	  )
)

(DOLOCALE)

(complete 100)


;=============================================================================
; "Say goodnight Gracy."
;
;(message #reboot-lang)
;(run "Reboot")

))
