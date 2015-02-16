/*
=================================================================================     
*                        GL5202 DDR macro definition
* FileName: GL5202_ddr.h
* Author  : zhongxu  
* Version : v1.0
* Chip    : GL5202
* Date    : 2011-6-27
* Description:  GL5202 macro definition
* History :
*   v1.0    create this file    2011-6-27
=================================================================================  
*/

/*=================================================================================
*       DCU feature register Parameter
=================================================================================*/  
#define VDD_DET_EN              0x10000000      //vdd detect enable 
#define	Width_32		            0x01000000	    
#define	Width_16		            0x00000000
#define AUPD        		        0x00010000      //auto power down enable
//#define AUSR                    0x00020000      //auto selfrefresh enable
#define AUSR                    0x00030000      //auto selfrefresh enable
                                
#define ODT_INEN		            0x00100000	    //IC inner ODT enable
#define ODT_EXEN                0x00001000      //Device ODT enable
#define CMD_REORDER             0x00002000	    //CMD REORDER enable
#define ODTIN_VALUE_120         0x40000
#define ODTIN_VALUE_60          0x80000
#define ODTIN_VALUE_40          0xc0000
                                
#define PRIO0       		        0x00000000      //Priority configure  0
#define PRIO1       		        0x00000100      //Priority configure  1
#define PRIO2       		        0x00000200      //Priority configure  2 
#define PRIO3       		        0x00000300      //Priority configure  3
                                
#define CAP_256Mbit   	        0x00000000	        
#define CAP_512Mbit   	        0x00000010	  
#define CAP_1Gbit  		          0x00000020	  
#define CAP_2Gbit    		        0x00000030	  
#define CAP_4Gbit    		        0x00000040	  
#define CAP_8Gbit    		        0x00000050	  
#define CAP_16Gbit              0x00000060	  
                                                                
#define BANK4       		        0x00000000
#define BANK8       		        0x00000008
                                
#define PAGE_1KB 		            0x00000000      
#define PAGE_2KB  		          0x00000002      
#define PAGE_4KB  		          0x00000004      
                                
#define DDR2_SDRAM   		        0x00000000
#define DDR3_SDRAM		          0x00000001
#define DDR3L_SDRAM		          0x20000001


/*=================================================================================
*       DCU command register Parameter
=================================================================================*/

#define CMD_ZQC_IN   		        0x00100000	    //IC inner ZQ calibration
#define CMD_ZQCS_EX             0x00080000      //Device short ZQ calibration
#define CMD_ZQCL_EX             0x00040000      //Device long ZQ calibration
#define CMD_PREAMBLE_DETECT     0x00020000      //preamble detect command
#define CMD_TIMING_ADJUST       0x00008000	    //delay chain timing parameter adjust command
#define CMD_INIT    		        0x00001000	    //DDR initialization 
#define CMD_PDEN    		        0x00000200	    //power down enter
#define CMD_PDEX    		        0x00000100	    //power down exit
#define CMD_SREN    		        0x00000020	    //selfrefresh enter
#define CMD_SREX    		        0x00000010	    //selfrefresh exit
#define CMD_EMRS3    		        0x00000008	    //extend mode register 3 load command   
#define CMD_EMRS2    		        0x00000004	    //extend mode register 2 load command
#define CMD_EMRS1    		        0x00000002	    //extend mode register 1 load command
#define CMD_NMRS   		          0x00000001	    //mode register load command

/*=================================================================================
*       CORE CLOCK Parameter
=================================================================================*/

#define     CORECLK_48M      0x304    //  48  M
#define     CORECLK_60M      0x305    //  60  M
#define     CORECLK_72M      0x306    //  72  M
#define     CORECLK_84M      0x307    //  84  M
#define     CORECLK_96M      0x308    //  96  M
#define     CORECLK_108M     0x309    //  108 M
#define     CORECLK_120M     0x30A    //  120 M
#define     CORECLK_132M     0x30B    //  132 M
#define     CORECLK_144M     0x30C    //  144 M
#define     CORECLK_156M     0x30D    //  156 M
#define     CORECLK_168M     0x30E    //  168 M
#define     CORECLK_180M     0x30F    //  180 M
#define     CORECLK_192M     0x310    //  192 M
#define     CORECLK_204M     0x311    //  204 M
#define     CORECLK_216M     0x312    //  216 M
#define     CORECLK_228M     0x313    //  228 M
#define     CORECLK_240M     0x314    //  240 M
#define     CORECLK_252M     0x315    //  252 M
#define     CORECLK_264M     0x316    //  264 M
#define     CORECLK_276M     0x317    //  276 M
#define     CORECLK_288M     0x318    //  288 M
#define     CORECLK_300M     0x319    //  300 M
#define     CORECLK_312M     0x31A    //  312 M
#define     CORECLK_324M     0x31B    //  324 M
#define     CORECLK_336M     0x31C    //  336 M
#define     CORECLK_348M     0x31D    //  348 M
#define     CORECLK_360M     0x31E    //  360 M
#define     CORECLK_372M     0x31F    //  372 M
#define     CORECLK_384M     0x320    //  384 M
#define     CORECLK_396M     0x321    //  396 M
#define     CORECLK_408M     0x322    //  408 M
#define     CORECLK_420M     0x323    //  420 M
#define     CORECLK_432M     0x324    //  432 M
#define     CORECLK_444M     0x325    //  444 M
#define     CORECLK_456M     0x326    //  456 M
#define     CORECLK_468M     0x327    //  468 M
#define     CORECLK_480M     0x328    //  480 M
#define     CORECLK_492M     0x329    //  492 M
#define     CORECLK_504M     0x32A    //  504 M
#define     CORECLK_516M     0x32B    //  516 M
#define     CORECLK_528M     0x32C    //  528 M
#define     CORECLK_540M     0x32D    //  540 M
#define     CORECLK_552M     0x32E    //  552 M
#define     CORECLK_564M     0x32F    //  564 M
#define     CORECLK_576M     0x330    //  576 M
#define     CORECLK_588M     0x331    //  588 M
#define     CORECLK_600M     0x332    //  600 M
#define     CORECLK_612M     0x333    //  612 M
#define     CORECLK_624M     0x334    //  624 M
#define     CORECLK_636M     0x335    //  636 M
#define     CORECLK_648M     0x336    //  648 M
#define     CORECLK_660M     0x337    //  660 M
#define     CORECLK_672M     0x338    //  672 M
#define     CORECLK_684M     0x339    //  684 M
#define     CORECLK_696M     0x33A    //  696 M
#define     CORECLK_708M     0x33B    //  708 M
#define     CORECLK_720M     0x33C    //  720 M
#define     CORECLK_732M     0x33D    //  732 M
#define     CORECLK_744M     0x33E    //  744 M
#define     CORECLK_756M     0x33F    //  756 M
#define     CORECLK_768M     0x340    //  768 M
#define     CORECLK_780M     0x341    //  780 M
#define     CORECLK_792M     0x342    //  792 M
#define     CORECLK_804M     0x343    //  804 M
#define     CORECLK_816M     0x344    //  816 M
#define     CORECLK_828M     0x345    //  828 M
#define     CORECLK_840M     0x346    //  840 M
#define     CORECLK_852M     0x347    //  852 M
#define     CORECLK_864M     0x348    //  864 M
#define     CORECLK_876M     0x349    //  876 M
#define     CORECLK_888M     0x34A    //  888 M
#define     CORECLK_900M     0x34B    //  900 M
#define     CORECLK_912M     0x34C    //  912 M
#define     CORECLK_924M     0x34D    //  924 M
#define     CORECLK_936M     0x34E    //  936 M
#define     CORECLK_948M     0x34F    //  948 M
#define     CORECLK_960M     0x350    //  960 M
#define     CORECLK_972M     0x351    //  972 M
#define     CORECLK_984M     0x352    //  984 M
#define     CORECLK_996M     0x353    //  996 M
#define     CORECLK_1008M    0x354    //  1008M
#define     CORECLK_1020M    0x355    //  1020M
#define     CORECLK_1032M    0x356    //  1032M
#define     CORECLK_1044M    0x357    //  1044M
#define     CORECLK_1056M    0x358    //  1056M
#define     CORECLK_1068M    0x359    //  1068M
#define     CORECLK_1080M    0x35A    //  1080M
#define     CORECLK_1092M    0x35B    //  1092M
#define     CORECLK_1104M    0x35C    //  1104M
#define     CORECLK_1116M    0x35D    //  1116M
#define     CORECLK_1128M    0x35E    //  1128M
#define     CORECLK_1140M    0x35F    //  1140M
#define     CORECLK_1152M    0x360    //  1152M
#define     CORECLK_1164M    0x361    //  1164M
#define     CORECLK_1176M    0x362    //  1176M
#define     CORECLK_1188M    0x363    //  1188M
#define     CORECLK_1200M    0x364    //  1200M
#define     CORECLK_1212M    0x365    //  1212M
#define     CORECLK_1224M    0x366    //  1224M
#define     CORECLK_1236M    0x367    //  1236M
#define     CORECLK_1248M    0x368    //  1248M
#define     CORECLK_1260M    0x369    //  1260M
#define     CORECLK_1272M    0x36A    //  1272M
#define     CORECLK_1284M    0x36B    //  1284M
#define     CORECLK_1296M    0x36C    //  1296M
#define     CORECLK_1308M    0x36D    //  1308M
#define     CORECLK_1320M    0x36E    //  1320M
#define     CORECLK_1332M    0x36F    //  1332M
#define     CORECLK_1344M    0x370    //  1344M
#define     CORECLK_1356M    0x371    //  1356M
#define     CORECLK_1368M    0x372    //  1368M
#define     CORECLK_1380M    0x373    //  1380M
#define     CORECLK_1392M    0x374    //  1392M
#define     CORECLK_1404M    0x375    //  1404M
#define     CORECLK_1416M    0x376    //  1416M
#define     CORECLK_1428M    0x377    //  1428M
#define     CORECLK_1440M    0x378    //  1440M
#define     CORECLK_1452M    0x379    //  1452M
#define     CORECLK_1464M    0x37A    //  1464M
#define     CORECLK_1476M    0x37B    //  1476M
#define     CORECLK_1488M    0x37C    //  1488M
#define     CORECLK_1500M    0x37D    //  1500M
#define     CORECLK_1512M    0x37E    //  1512M
#define     CORECLK_1524M    0x37F    //  1524M


/*=================================================================================
*       DDR PLL Parameter
=================================================================================*/

#define     DDRPLLEN       0x100

#define     DDRCLK_6M      0x00000001   //DDRPLL=12*1M   ;DDRCLK=DDRPLL/2= 6M     
#define     DDRCLK_12M     0x00000002   //DDRPLL=12*2M   ;DDRCLK=DDRPLL/2= 12M 
#define     DDRCLK_18M     0x00000003   //DDRPLL=12*3M   ;DDRCLK=DDRPLL/2= 18M    
#define     DDRCLK_24M     0x00000004   //DDRPLL=12*4M   ;DDRCLK=DDRPLL/2= 24M           
#define     DDRCLK_30M     0x00000005   //DDRPLL=12*5M   ;DDRCLK=DDRPLL/2= 30M           
#define     DDRCLK_36M     0x00000006   //DDRPLL=12*6M   ;DDRCLK=DDRPLL/2= 36M           
#define     DDRCLK_42M     0x00000007   //DDRPLL=12*7M   ;DDRCLK=DDRPLL/2= 42M           
#define     DDRCLK_48M     0x00000008   //DDRPLL=12*8M   ;DDRCLK=DDRPLL/2= 48M    
#define     DDRCLK_54M     0x00000009   //DDRPLL=12*9M   ;DDRCLK=DDRPLL/2= 54M 
#define     DDRCLK_60M     0x0000000A   //DDRPLL=12*10M   ;DDRCLK=DDRPLL/2= 60M 
#define     DDRCLK_66M     0x0000000B   //DDRPLL=12*11M   ;DDRCLK=DDRPLL/2= 66M 
#define     DDRCLK_72M     0x0000000C   //DDRPLL=12*12M  ;DDRCLK=DDRPLL/2= 72M 
#define     DDRCLK_78M     0x0000000D   //DDRPLL=12*13M  ;DDRCLK=DDRPLL/2= 78M 
#define     DDRCLK_84M     0x0000000E   //DDRPLL=12*14M  ;DDRCLK=DDRPLL/2= 84M 
#define     DDRCLK_90M     0x0000000F   //DDRPLL=12*15M  ;DDRCLK=DDRPLL/2= 90M 
#define     DDRCLK_96M     0x00000010   //DDRPLL=12*16M  ;DDRCLK=DDRPLL/2= 96M 
#define     DDRCLK_102M    0x00000011   //DDRPLL=12*17M  ;DDRCLK=DDRPLL/2= 102M
#define     DDRCLK_108M    0x00000012   //DDRPLL=12*18M  ;DDRCLK=DDRPLL/2= 108M
#define     DDRCLK_114M    0x00000013   //DDRPLL=12*19M  ;DDRCLK=DDRPLL/2= 114M
#define     DDRCLK_120M    0x00000014   //DDRPLL=12*20M  ;DDRCLK=DDRPLL/2= 120M
#define     DDRCLK_126M    0x00000015   //DDRPLL=12*21M  ;DDRCLK=DDRPLL/2= 126M
#define     DDRCLK_132M    0x00000016   //DDRPLL=12*22M  ;DDRCLK=DDRPLL/2= 132M
#define     DDRCLK_138M    0x00000017   //DDRPLL=12*23M  ;DDRCLK=DDRPLL/2= 138M
#define     DDRCLK_144M    0x00000018   //DDRPLL=12*24M  ;DDRCLK=DDRPLL/2= 144M
#define     DDRCLK_150M    0x00000019   //DDRPLL=12*25M  ;DDRCLK=DDRPLL/2= 150M
#define     DDRCLK_156M    0x0000001A   //DDRPLL=12*26M  ;DDRCLK=DDRPLL/2= 156M
#define     DDRCLK_162M    0x0000001B   //DDRPLL=12*27M  ;DDRCLK=DDRPLL/2= 162M
#define     DDRCLK_168M    0x0000001C   //DDRPLL=12*28M  ;DDRCLK=DDRPLL/2= 168M
#define     DDRCLK_174M    0x0000001D   //DDRPLL=12*29M  ;DDRCLK=DDRPLL/2= 174M
#define     DDRCLK_180M    0x0000001E   //DDRPLL=12*30M  ;DDRCLK=DDRPLL/2= 180M
#define     DDRCLK_186M    0x0000001F   //DDRPLL=12*31M  ;DDRCLK=DDRPLL/2= 186M
#define     DDRCLK_192M    0x00000020   //DDRPLL=12*32M  ;DDRCLK=DDRPLL/2= 192M
#define     DDRCLK_198M    0x00000021   //DDRPLL=12*33M  ;DDRCLK=DDRPLL/2= 198M
#define     DDRCLK_204M    0x00000022   //DDRPLL=12*34M  ;DDRCLK=DDRPLL/2= 204M
#define     DDRCLK_210M    0x00000023   //DDRPLL=12*35M  ;DDRCLK=DDRPLL/2= 210M
#define     DDRCLK_216M    0x00000024   //DDRPLL=12*36M  ;DDRCLK=DDRPLL/2= 216M                                                                              
#define     DDRCLK_222M    0x00000025   //DDRPLL=12*37M  ;DDRCLK=DDRPLL/2= 222M
#define     DDRCLK_228M    0x00000026   //DDRPLL=12*38M  ;DDRCLK=DDRPLL/2= 228M
#define     DDRCLK_234M    0x00000027   //DDRPLL=12*39M  ;DDRCLK=DDRPLL/2= 234M
#define     DDRCLK_240M    0x00000028   //DDRPLL=12*40M  ;DDRCLK=DDRPLL/2= 240M
#define     DDRCLK_246M    0x00000029   //DDRPLL=12*41M  ;DDRCLK=DDRPLL/2= 246M
#define     DDRCLK_252M    0x0000002A   //DDRPLL=12*42M  ;DDRCLK=DDRPLL/2= 252M
#define     DDRCLK_258M    0x0000002B   //DDRPLL=12*43M  ;DDRCLK=DDRPLL/2= 258M
#define     DDRCLK_264M    0x0000002C   //DDRPLL=12*44M  ;DDRCLK=DDRPLL/2= 264M
#define     DDRCLK_270M    0x0000002D   //DDRPLL=12*45M  ;DDRCLK=DDRPLL/2= 270M
#define     DDRCLK_276M    0x0000002E   //DDRPLL=12*46M  ;DDRCLK=DDRPLL/2= 276M
#define     DDRCLK_282M    0x0000002F   //DDRPLL=12*47M  ;DDRCLK=DDRPLL/2= 282M
#define     DDRCLK_288M    0x00000030   //DDRPLL=12*48M  ;DDRCLK=DDRPLL/2= 288M
#define     DDRCLK_294M    0x00000031   //DDRPLL=12*49M  ;DDRCLK=DDRPLL/2= 294M
#define     DDRCLK_300M    0x00000032   //DDRPLL=12*50M  ;DDRCLK=DDRPLL/2= 300M
#define     DDRCLK_306M    0x00000033   //DDRPLL=12*51M  ;DDRCLK=DDRPLL/2= 306M
#define     DDRCLK_312M    0x00000034   //DDRPLL=12*52M  ;DDRCLK=DDRPLL/2= 312M
#define     DDRCLK_318M    0x00000035   //DDRPLL=12*53M   ;DDRCLK=DDRPLL/2= 318M   
#define     DDRCLK_324M    0x00000036   //DDRPLL=12*54M   ;DDRCLK=DDRPLL/2= 324M
#define     DDRCLK_330M    0x00000037   //DDRPLL=12*55M   ;DDRCLK=DDRPLL/2= 330M   
#define     DDRCLK_336M    0x00000038   //DDRPLL=12*56M   ;DDRCLK=DDRPLL/2= 336M          
#define     DDRCLK_342M    0x00000039   //DDRPLL=12*57M   ;DDRCLK=DDRPLL/2= 342M          
#define     DDRCLK_348M    0x0000003A   //DDRPLL=12*58M   ;DDRCLK=DDRPLL/2= 348M          
#define     DDRCLK_354M    0x0000003B   //DDRPLL=12*59M   ;DDRCLK=DDRPLL/2= 354M          
#define     DDRCLK_360M    0x0000003C   //DDRPLL=12*60M   ;DDRCLK=DDRPLL/2= 360M   
#define     DDRCLK_366M    0x0000003D   //DDRPLL=12*61M   ;DDRCLK=DDRPLL/2= 366M
#define     DDRCLK_372M    0x0000003E   //DDRPLL=12*62M   ;DDRCLK=DDRPLL/2= 372M
#define     DDRCLK_378M    0x0000003F   //DDRPLL=12*63M   ;DDRCLK=DDRPLL/2= 378M
#define     DDRCLK_384M    0x00000040   //DDRPLL=12*64M  ;DDRCLK=DDRPLL/2= 384M
#define     DDRCLK_390M    0x00000041   //DDRPLL=12*65M  ;DDRCLK=DDRPLL/2= 390M
#define     DDRCLK_396M    0x00000042   //DDRPLL=12*66M  ;DDRCLK=DDRPLL/2= 396M
#define     DDRCLK_402M    0x00000043   //DDRPLL=12*67M  ;DDRCLK=DDRPLL/2= 402M                                                                              
#define     DDRCLK_408M    0x00000044   //DDRPLL=12*68M  ;DDRCLK=DDRPLL/2= 408M
#define     DDRCLK_414M    0x00000045   //DDRPLL=12*69M  ;DDRCLK=DDRPLL/2= 414M
#define     DDRCLK_420M    0x00000046   //DDRPLL=12*70M  ;DDRCLK=DDRPLL/2= 420M
#define     DDRCLK_426M    0x00000047   //DDRPLL=12*71M  ;DDRCLK=DDRPLL/2= 426M
#define     DDRCLK_432M    0x00000048   //DDRPLL=12*72M  ;DDRCLK=DDRPLL/2= 432M
#define     DDRCLK_438M    0x00000049   //DDRPLL=12*73M  ;DDRCLK=DDRPLL/2= 438M
#define     DDRCLK_444M    0x0000004A   //DDRPLL=12*74M  ;DDRCLK=DDRPLL/2= 444M
#define     DDRCLK_450M    0x0000004B   //DDRPLL=12*75M  ;DDRCLK=DDRPLL/2= 450M
#define     DDRCLK_456M    0x0000004C   //DDRPLL=12*76M  ;DDRCLK=DDRPLL/2= 456M
#define     DDRCLK_462M    0x0000004D   //DDRPLL=12*77M  ;DDRCLK=DDRPLL/2= 462M
#define     DDRCLK_468M    0x0000004E   //DDRPLL=12*78M  ;DDRCLK=DDRPLL/2= 468M
#define     DDRCLK_474M    0x0000004F   //DDRPLL=12*79M  ;DDRCLK=DDRPLL/2= 474M
#define     DDRCLK_480M    0x00000050   //DDRPLL=12*80M  ;DDRCLK=DDRPLL/2= 480M
#define     DDRCLK_486M    0x00000051   //DDRPLL=12*81M  ;DDRCLK=DDRPLL/2= 486M
#define     DDRCLK_492M    0x00000052   //DDRPLL=12*82M  ;DDRCLK=DDRPLL/2= 492M
#define     DDRCLK_498M    0x00000053   //DDRPLL=12*83M  ;DDRCLK=DDRPLL/2= 498M
#define     DDRCLK_504M    0x00000054   //DDRPLL=12*84M  ;DDRCLK=DDRPLL/2= 504M
#define     DDRCLK_510M    0x00000055   //DDRPLL=12*85M  ;DDRCLK=DDRPLL/2= 510M
#define     DDRCLK_516M    0x00000056   //DDRPLL=12*86M  ;DDRCLK=DDRPLL/2= 516M
#define     DDRCLK_522M    0x00000057   //DDRPLL=12*87M  ;DDRCLK=DDRPLL/2= 522M                                                                             
#define     DDRCLK_528M    0x00000058   //DDRPLL=12*88M  ;DDRCLK=DDRPLL/2= 528M
#define     DDRCLK_534M    0x00000059   //DDRPLL=12*89M  ;DDRCLK=DDRPLL/2= 534M
#define     DDRCLK_540M    0x0000005A   //DDRPLL=12*90M  ;DDRCLK=DDRPLL/2= 540M
#define     DDRCLK_546M    0x0000005B   //DDRPLL=12*91M  ;DDRCLK=DDRPLL/2= 546M
#define     DDRCLK_552M    0x0000005C   //DDRPLL=12*92M  ;DDRCLK=DDRPLL/2= 552M
#define     DDRCLK_558M    0x0000005D   //DDRPLL=12*93M  ;DDRCLK=DDRPLL/2= 558M
#define     DDRCLK_564M    0x0000005E   //DDRPLL=12*94M  ;DDRCLK=DDRPLL/2= 564M
#define     DDRCLK_570M    0x0000005F   //DDRPLL=12*95M  ;DDRCLK=DDRPLL/2= 570M
#define     DDRCLK_576M    0x00000060   //DDRPLL=12*96M  ;DDRCLK=DDRPLL/2= 576M
#define     DDRCLK_582M    0x00000061   //DDRPLL=12*97M  ;DDRCLK=DDRPLL/2= 582M
#define     DDRCLK_588M    0x00000062   //DDRPLL=12*98M  ;DDRCLK=DDRPLL/2= 588M
#define     DDRCLK_594M    0x00000063   //DDRPLL=12*99M  ;DDRCLK=DDRPLL/2= 594M
#define     DDRCLK_600M    0x00000064   //DDRPLL=12*100M ;DDRCLK=DDRPLL/2= 600M

/*=================================================================================
*       DMA定义
=================================================================================*/
#define	    DMA_DTrg_DDR      	       0x00120000
#define	    DMA_DTrg_SRAM      	       0x00040000
                                    
#define	    DMA_STrg_DDR      	       0x00000012
#define	    DMA_STrg_SRAM      	       0x00000004
#define     DMA_RELO                   0x80000000

#define Write_reg(reg,value)            (*(volatile unsigned int *)(reg) = (value))
#define Read_reg(reg)                   (*(volatile unsigned int *)(reg))
#define Write_byte(addr,value)            (*(volatile char *)(addr) = (value))

#ifndef __ASSEMBLY__

extern int od_first_ok_dly, od_last_ok_dly;
extern int id_first_ok_dly, id_last_ok_dly;  
extern void dcu_cmd_apply(unsigned int dcu_cmd);
extern void _GL5202_BDMA0_TRANSFER(unsigned int src,unsigned int dest,unsigned int cnt,unsigned int mode);
extern int _GL5202_data_compare_byte(unsigned int src,unsigned int dest,unsigned int len);
extern int _GL5202_DDR3_init(unsigned int ddr_type,unsigned int freq,unsigned int single_dram_cap,unsigned int ddr_drv0,unsigned int ddr_drv1,unsigned int boot_mode,unsigned int freq_init,unsigned char ddrmisc);
extern int _GL5202_DelayChain_Scan(unsigned int srcAddr,unsigned int dstAddr,unsigned int dataLen,unsigned int freq);
extern int _GL5202_DDR_CAP_DETECT(void);
extern int _test_data_init(unsigned int start_addr,unsigned int data_len);
extern int _ddr_delaychain_check(unsigned int srcAddr,unsigned int dstAddr,unsigned int dataLen,unsigned int StartScanDly,unsigned int od_id,unsigned int freq);
extern void _delay_func(volatile unsigned int num);
extern int _ddr_freq_change (unsigned int freq,unsigned int single_dram_cap,unsigned int driver0,unsigned int driver1,unsigned char ddrmisc);

#endif

