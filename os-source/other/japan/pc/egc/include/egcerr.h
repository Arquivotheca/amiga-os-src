/************************************************************************/
/*                                                                      */
/*      EGCERR  : EGConvert ldml ERRor definition                       */
/*                                                                      */
/*              Designed    by  M.Yamashita     06/21/1988              */
/*              Coded       by  M.Yamashita     06/21/1988              */
/*                                                                      */
/*      (C) Copyright 1985-1991 ERGOSOFT Corp.                          */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#ifndef        __EGCERR__
#define        __EGCERR__

/* error number definition */
/*
**  egcinit()       LDML number 0x01
*/
#define     ERINI01    0x0101          /* init error   */
#define     ERINI02    0x0102          /* egc-sys = main-dict */
#define     ERINI03    0x0103          /* egc-sys < main-dict */
#define     ERINI04    0x0104          /* egc-sys > main-dict */
#define     ERINI05    0x0105          /* verno error */
#define     ERINI06    0x0106          /* mismatch GT type */
/*
**  egcfinish()     LDML number 0x02
*/
#define     ERFIN01    0x0201          /* finish error */
/*
**  egctxcvt()      LDML number 0x03
*/
#define     ERTXC01    0x0301          /* conversion error        */
#define     ERTXC02    0x0302          /* yomi string error       */
#define     ERTXC03    0x0303          /* yomi length error       */
#define     ERTXC10    0x0310          /* conversion length error */
#define     ERTXC11    0x0311          /* over clause error       */
/*
**  egcclcvt()      LDML number 0x04
*/
#define     ERCLC01    0x0401          /* conversion error        */
#define     ERCLC02    0x0402          /* yomi string error       */
#define     ERCLC03    0x0403          /* yomi length error       */
#define     ERCLC10    0x0410          /* conversion length error */
#define     ERCLC12    0x0412          /* over homonyms error     */
/*
**  egcrmcvt()      LDML number 0x05
*/
#define     ERRMC01    0x0501          /* conversion error        */
#define     ERRMC02    0x0502          /* romaji string error     */
#define     ERRMC03    0x0503          /* romaji length error     */
#define     ERRMC10    0x0510          /* conversion length error */
/*
**  egcjacvt()      LDML number 0x06
*/
#define     ERJAC01    0x0601          /* conversion error        */
#define     ERJAC02    0x0602          /* yomi string error       */
#define     ERJAC03    0x0603          /* yomi length error       */
#define     ERJAC10    0x0610          /* conversion length error */
/*
**  egcajcvt()      LDML number 0x07
*/
#define     ERAJC01    0x0701          /* conversion error        */
#define     ERAJC02    0x0702          /* yomi string error       */
#define     ERAJC03    0x0703          /* yomi length error       */
#define     ERAJC10    0x0710          /* conversion length error */
/*
**  egcsjcvt()      LDML number 0x08
*/
#define     ERSJC01    0x0801          /* conversion error        */
#define     ERSJC02    0x0802          /* s-jis string error      */
#define     ERSJC03    0x0803          /* s-jis length error      */
#define     ERSJC10    0x0810          /* conversion length error */
/*
**  egcjscvt()      LDML number 0x09
*/
#define     ERJSC01    0x0901          /* conversion error        */
#define     ERJSC02    0x0902          /* jis string error        */
#define     ERJSC03    0x0903          /* jis length error        */
#define     ERJSC10    0x0910          /* conversion length error */
/*
**  egclearn()      LDML number 0x0A
*/
#define     ERLER01    0x0A01          /* dict learn error */
#define     ERLER02    0x0A02          /* input error      */
#define     ERLER03    0x0A03          /* dictionary is not active */
/*
**  egcwdadd()      LDML number 0x0B
*/
#define     ERWDA01    0x0B01          /* user dict add error  */
#define     ERWDA02    0x0B02          /* yomi string error    */
#define     ERWDA03    0x0B03          /* yomi length error    */
#define     ERWDA04    0x0B04          /* kanji string error   */
#define     ERWDA05    0x0B05          /* kanji length error   */
#define     ERWDA06    0x0B06          /* hinshi code error    */
#define     ERWDA12    0x0B12          /* over homonyms error  */
#define     ERWDA20    0x0B20          /* plo full error       */
#define     ERWDA21    0x0B21          /* dso full error       */
#define     ERWDA22    0x0B22          /* user dict full error */
#define     ERWDA23    0x0B23          /* over near yomi error */
#define     ERWDA24    0x0B24          /* no extrayomi space   */
#define     ERWDA25    0x0B25          /* dictionary is not active */

/*
**  egcwddel()      LDML number 0x0C
*/
#define     ERWDD01    0x0C01          /* user dict del error   */
#define     ERWDD02    0x0C02          /* yomi string error     */
#define     ERWDD03    0x0C03          /* yomi length error     */
#define     ERWDD04    0x0C04          /* kanji string error    */
#define     ERWDD05    0x0C05          /* kanji length error    */
#define     ERWDD20    0x0C20          /* yomi not found error  */
#define     ERWDD21    0x0C21          /* kanji not found error */
#define     ERWDD22    0x0C22          /* dictionary is not active */
/*
**  egccllearn()    LDML number 0x0D
*/
#define     ERCLL01    0x0D01          /* clause learn error */
#define     ERCLL02    0x0D02          /* yomi string error  */
#define     ERCLL03    0x0D03          /* yomi length error  */
#define     ERCLL04    0x0D04          /* kanji string error */
#define     ERCLL05    0x0D05          /* kanji length error */
#define     ERCLL06    0x0C06          /* dictionary is not active */

/*
**  egcfind()       LDML number 0x0E
*/
#define     ERFND01    0x0E01          /* dict find error   */
#define     ERFND02    0x0E02          /* yomi string error */
#define     ERFND03    0x0E03          /* yomi length error */
#define     ERFND07    0x0E07          /* find mode error   */
#define     ERFND08    0x0E08          /* dict mode error   */
/*
**  egcnext()       LDML number 0x0F
*/
#define     ERNXT01    0x0F01          /* next find error     */
#define     ERNXT24    0x0F24          /* bad data find error */
#define     ERNXT25    0x0F25          /* plo destroy error   */
#define     ERNXT99    0x0F99          /* illegal yomi length */

/*
**  egcread()       LDML number 0x10
*/
#define     ERRED01    0x1001          /* dict read error */

/*
**  egccreate()     LDML number 0x11
*/
#define     ERCRE01    0x1101          /* main dict create error */
#define     ERCRE02    0x1102          /* dict name error        */
#define     ERCRE03    0x1103          /* dict name length error */
/*
**  egcwrite()      LDML number 0x12
*/
#define     ERWRI01    0x1201          /* main dict write error */
#define     ERWRI02    0x1202          /* yomi string error     */
#define     ERWRI03    0x1203          /* yomi length error     */
#define     ERWRI04    0x1204          /* kanji string error    */
#define     ERWRI05    0x1205          /* kanji length error    */
#define     ERWRI06    0x1206          /* hinshi code error     */
#define     ERWRI12    0x1212          /* over homonyms error   */
#define     ERWRI13    0x1213          /* over tan-kanji error  */
#define     ERWRI14    0x1214          /* over segments error   */
#define     ERWRI22    0x1222          /* main dict full error  */
#define     ERWRI31    0x1231          /* egcflush error        */
#define     ERWRI32    0x1232          /* exist skip data error */
/*
**  egceof()        LDML number 0x13
*/
#define     EREOF01    0x1301          /* main dict finish error */
/*
**  egcndic()       LDML number 0x14
*/
#define     ERNDI01    0x1401          /* user dict create error */
#define     ERNDI02    0x1402          /* dict name error        */
#define     ERNDI03    0x1403          /* dict name length error */
#define     ERNDI04    0x1404          /* user name error        */
#define     ERNDI05    0x1405          /* user name length error */
#define     ERNDI09    0x1409          /* user dict size error   */
/*
**  egclearn2()     LDML number 0x15
*/
#define     ERLN201    0x1501          /* dict learn error        */
#define     ERLN202    0x1502          /* input error             */
#define     ERLN203    0x1503          /* expert-word input error */
#define     ERLN204    0x1504          /* dictionary is not active */
#define     ERLN205    0x1505          /* formula learning error  */
/*
**  egccodeinit()       LDML number 0x16
*/
#define     ERCIN01    0x1601          /* init error   */
/*
**  egcsecvt()          LDML number 0x17
*/
#define     ERSEC01    0x1701          /* conversion error        */
#define     ERSEC02    0x1702          /* shift jis string error  */
#define     ERSEC03    0x1703          /* shift jis length error  */
#define     ERSEC10    0x1710          /* conversion length error */
/*
**  egcescvt()          LDML number 0x18
*/
#define     ERESC01    0x1801          /* conversion error        */
#define     ERESC02    0x1802          /* EUC string error        */
#define     ERESC03    0x1803          /* EUC length error        */
#define     ERESC10    0x1810          /* conversion length error */
/*
**  egciecvt()          LDML number 0x19
*/
#define     ERIEC01    0x1901          /* conversion error        */
#define     ERIEC02    0x1902          /* IKIS string error       */
#define     ERIEC03    0x1903          /* IKIS length error        */
#define     ERIEC10    0x1910          /* conversion length error */
/*
**  egceicvt()          LDML number 0x1A
*/
#define     EREIC01    0x1A01          /* conversion error        */
#define     EREIC02    0x1A02          /* EUC string error        */
#define     EREIC03    0x1A03          /* EUC length error        */
#define     EREIC10    0x1A10          /* conversion length error */
/*
**  egcopenexpdic()     LDML number 0x1B
*/
#define     EROED01    0x1B01          /* expert_dict open error  */
#define     EROED02    0x1B02          /* opened dict already     */
/*
**  egccloseexpdic()    LDML number 0x1C
*/
#define     ERCED01    0x1C01          /* expert_dict close error */
/*
**  egcmexp()           LDML number 0x1D
*/
#define     ERMAR01    0x1D01          /* main open error         */
#define     ERMAR02    0x1D02          /* user open error         */
#define     ERMAR03    0x1D03          /* expert open error       */
#define     ERMAR11    0x1D11          /* main read error         */
#define     ERMAR12    0x1D12          /* user read error         */
#define     ERMAR23    0x1D23          /* expert write error      */
/*
**  egcrmcvt()          return value define
*/
#define EGCRM_NOT       0
#define EGCRM_ALL       1
#define EGCRM_PCE       2
#define EGCRM_PCN       3
/*
**  egcdicverchk()      LDML number 0x1E
*/
#define     ERVER00    0x1E00          /* something error */
#define     ERVER01    0x1E01          /* mismatch m&u version-NO. error */
#define     ERVER02    0x1E02          /* mismatch m&u GI-size error */
#define     ERVER03    0x1E03          /* egc-sys != main-dict */
/*
**  egcsysverchk()      LDML number 0x1F
*/
#define     ERSVC00    0x1F00          /* something error */
#define     ERSVC01    0x1F01          /* main-dict not open */
#define     ERSVC02    0x1F02          /* egc-sys = main-dict */
#define     ERSVC03    0x1F03          /* egc-sys < main-dict */
#define     ERSVC04    0x1F04          /* egc-sys > main-dict */
#define     ERSVC05    0x1F05          /* verno error */

#endif    /* __EGCERR__ */
